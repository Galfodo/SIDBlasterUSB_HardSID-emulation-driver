
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
    {
      // Sleep until a command is queued instead of busy-polling CommandsPending() -
      // the old busy-spin loop pinned a full CPU core for the lifetime of the DLL.
      // wait_for (with a short timeout as a safety net) rather than a bare wait()
      // avoids a lost-wakeup hang if a notify races ahead of us reaching this wait.
      std::unique_lock<std::mutex>
        lockGuard(receiver->m_CommandPendingMutex);
      receiver->m_CommandPendingEvent.wait_for(lockGuard, std::chrono::milliseconds(2), [&]() {
        return receiver->CommandsPending() || *do_abort;
      });
    }
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

ThreadDispatcher::ThreadDispatcher() : m_Receiver(NULL), m_IsInitialized(false), m_AbortSIDWriteThread(false), m_bufferSize(SIDBlasterInterface::DEFAULT_WRITE_BUFFER_SIZE) {
}

void ThreadDispatcher::SetWriteBufferSize(int bufferSize) {
	m_bufferSize = bufferSize;
}

int
ThreadDispatcher::SendCommand(CommandParams const& cmd) {
  EnsureInitialized();
  int retval = 0;
  {
    while (!m_SIDWriteThread.joinable()) {
      std::this_thread::yield();
    }
    if (cmd.m_Command == CommandParams::Flush) {
      m_Receiver->Flush();
      m_Receiver->m_CommandPendingEvent.notify_all();
      while (m_Receiver->m_CommandRead != m_Receiver->m_CommandWrite) {
        std::this_thread::yield();
      }
      return 0;
    }
    assert(!m_Receiver->IsReadResultReady());
    if (!m_Receiver->TryPutCommand(cmd)) {
      m_Receiver->m_CommandPendingEvent.notify_all();
      return -1;
    }
    switch (cmd.m_Command)
    {
    case CommandParams::Write:
      // Always notify, regardless of m_IsBuffered. That flag only means
      // "batch the USB transfer" (see CommandReceiver::ExecuteCommand's
      // buffer_writes/BufferWrite) - it says nothing about whether the
      // dispatcher thread should be woken. Skipping notify here was safe
      // back when the dispatcher busy-spun and picked up buffered commands
      // within a fraction of a millisecond regardless; now that it sleeps
      // (see ThreadDispatcher.cpp's wait_for), skipping notify meant every
      // batch of buffered writes - which is what HardSID_Try_Write always
      // sends, see hardsiddll.cpp - waited for the 2ms fallback poll
      // instead of dispatching immediately. At SID-register-write timing
      // precision, a consistent 2ms-per-batch stall is audible stuttering.
      m_Receiver->m_CommandPendingEvent.notify_all();
      break;
    case CommandParams::Read:
      // BLOCKING! Should be re-engineered to allow an async interface
      m_Receiver->m_CommandPendingEvent.notify_all();
      // Block until read result available
      while (!m_Receiver->IsReadResultReady()) {
        std::this_thread::yield();
      }
      retval = m_Receiver->ReadResult();
      assert(!m_Receiver->IsReadResultReady());
      break;
    default:
      m_Receiver->m_CommandPendingEvent.notify_all();
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
  m_IsInitialized = false;
}

void ThreadDispatcher::EnsureInitialized() {
  if (!m_IsInitialized) {
#ifdef ENABLE_ATTACH_DEBUGGER
    MessageBox(0, "Attach debugger now!", "hardsid.dll", MB_OK);
#endif
    assert(m_Receiver == NULL);
    m_Receiver = new ThreadCommandReceiver();
    s_DeviceCount = -1;
    m_AbortSIDWriteThread = false;
    m_Receiver->Initialize(m_bufferSize);
    m_SIDWriteThread = std::thread(ThreadFunction, m_Receiver, &m_AbortSIDWriteThread);
    if (m_SIDWriteThread.joinable()) {
      while (s_DeviceCount < 0) {
        std::this_thread::yield();
      }
    } else {
      s_DeviceCount = 0;
    }
    m_IsInitialized = true;
  }
}

void ThreadDispatcher::Uninitialize() {
  if (m_IsInitialized) {
    m_AbortSIDWriteThread = true;
    m_SIDWriteThread.join();
    m_Receiver->Uninitialize();
    delete m_Receiver;
    m_Receiver = NULL;
    m_IsInitialized = false;
  }
}

bool ThreadDispatcher::IsAsync() {
  return true;
}

int ThreadDispatcher::DeviceCount() {
  EnsureInitialized();
  return s_DeviceCount;
}

}
