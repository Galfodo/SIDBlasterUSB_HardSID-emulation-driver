
#define _CRT_SECURE_NO_DEPRECATE

/**
 * Uses a worker thread for scheduling writes to the SIDBlaster hardware.
 * Written by stein.pedersen@hue.no
 */

#include "ThreadDispatcher.h"
#include "ThreadReceiver.h"
#include "SIDBlasterInterface.h"
#include "DebugSIDBlasterInterface.h"
#include "SIDBlasterInterfaceImpl.h"

#include <cassert>
#include <vector>

namespace SIDBlaster {

static int s_DeviceCount;

static void
ThreadFunction(ThreadCommandReceiver* receiver, bool* do_abort) {
  s_DeviceCount = receiver->DeviceCount();
  for (int i = 0; i < s_DeviceCount; ++i) {
    CommandParams opencommand(i, CommandParams::OpenDevice);
    receiver->ExecuteCommand(opencommand);
  }
  while (!*do_abort) {
#ifdef USE_COMMAND_PENDING_EVENT
    {
      std::unique_lock<std::mutex>
        lockGuard(receiver->m_CommandPendingMutex);
      receiver->m_CommandPendingEvent.wait(lockGuard); 
    }
#endif
    while (receiver->CommandsPending()) {
      CommandParams command(0, CommandParams::NOP);
      if (receiver->TryGetCommand(command)) {
        receiver->ExecuteCommand(command);
      }
    }
#ifdef WAIT_FOR_COMMAND_COMPLETION
    receiver->m_CommandsCompletedEvent.notify_all();
#endif
  }
#ifdef ENABLE_LOG_FILE
  {
    FILE* file = fopen("sidblasterusb.log", "wb");
    if (file && receiver->DeviceCount()) {
      fprintf(file, "Final writebuffer size: %d\n", receiver->Device(0));
      fclose(file);
    }
  }
#endif
  CommandParams closecommand(0, CommandParams::CloseDevice);
  receiver->ExecuteCommand(closecommand);
}

ThreadDispatcher::ThreadDispatcher() : m_Receiver(NULL) {
}

int
ThreadDispatcher::SendCommand(CommandParams const& cmd) {
  int retval = 0;
  {
    while (!m_SIDWriteThread.joinable()) {
      std::this_thread::yield();
    }
    if (cmd.m_Command == CommandParams::Flush) {
      m_Receiver->Flush();
#ifdef USE_COMMAND_PENDING_EVENT
      m_Receiver->m_CommandPendingEvent.notify_all();
#endif
      while (m_Receiver->m_CommandRead != m_Receiver->m_CommandWrite) {
        std::this_thread::yield();
      }
      return 0;
    }
    assert(!m_Receiver->IsReadResultReady());
    if (!m_Receiver->TryPutCommand(cmd)) {
#ifdef USE_COMMAND_PENDING_EVENT
      m_Receiver->m_CommandPendingEvent.notify_all();
#endif
      return -1;
    }
    switch (cmd.m_Command)
    {
    case CommandParams::Write:
#ifdef USE_COMMAND_PENDING_EVENT
      if (!cmd.m_IsBuffered) {
        m_Receiver->m_CommandPendingEvent.notify_all();
      }
#endif
      break;
    case CommandParams::Read:
      // BLOCKING! Should be re-engineered to allow an async interface
#ifdef USE_COMMAND_PENDING_EVENT
      m_Receiver->m_CommandPendingEvent.notify_all();
#endif
      // Block until read result available
      while (!m_Receiver->IsReadResultReady()) {
        std::this_thread::yield();
      }
      retval = m_Receiver->ReadResult();
      assert(!m_Receiver->IsReadResultReady());
      break;
    default:
#ifdef USE_COMMAND_PENDING_EVENT
      m_Receiver->m_CommandPendingEvent.notify_all();
#endif
      break;
    }
  }
#ifdef WAIT_FOR_COMMAND_COMPLETION
  {
    std::unique_lock<std::mutex>
      lockGuard(m_Receiver->m_CommandsCompletedMutex);

    m_Receiver->m_CommandsCompletedEvent.wait_for(lockGuard, std::chrono::milliseconds(1000)); // Set timeout in case worker thread is interrupted
  }
#endif
  return retval;
}

void ThreadDispatcher::Initialize() {
#ifdef ENABLE_ATTACH_DEBUGGER
  MessageBox(0, "Attach debugger now!", "hardsid.dll", MB_OK);
#endif
  assert(m_Receiver == NULL);
  m_Receiver = new ThreadCommandReceiver();
  s_DeviceCount = 0;
  m_AbortSIDWriteThread = false;
  m_Receiver->Initialize();
  m_SIDWriteThread = std::thread(ThreadFunction, m_Receiver, &m_AbortSIDWriteThread);
}

void ThreadDispatcher::Uninitialize() {
  m_AbortSIDWriteThread = true;
  m_SIDWriteThread.join();
  m_Receiver->Uninitialize();
  delete m_Receiver;
  m_Receiver = NULL;
}

bool ThreadDispatcher::IsAsync() {
  return true;
}

int ThreadDispatcher::DeviceCount() {
  while (!m_SIDWriteThread.joinable()) {
    std::this_thread::yield();
  }
  return s_DeviceCount;
}

}
