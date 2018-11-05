
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

static int
ThreadFunction(ThreadObject* thread) {
  assert(thread);
  assert(thread->Arg());
  ThreadCommandReceiver* receiver = (ThreadCommandReceiver*)thread->Arg();
  s_DeviceCount = receiver->DeviceCount();
  for (int i = 0; i < s_DeviceCount; ++i) {
    CommandParams opencommand(i, CommandParams::OpenDevice);
    receiver->ExecuteCommand(opencommand);
  }
  thread->Start();
  while (!thread->ShouldAbort()) {
#ifdef USE_COMMAND_PENDING_EVENT
    receiver->m_CommandPendingEvent.Wait(); 
#endif
    while (receiver->CommandsPending()) {
      CommandParams command(0, CommandParams::NOP);
      if (receiver->TryGetCommand(command)) {
        receiver->ExecuteCommand(command);
      }
    }
#ifdef USE_COMMAND_PENDING_EVENT
    receiver->m_CommandPendingEvent.Reset();
#endif
#ifdef WAIT_FOR_COMMAND_COMPLETION
    receiver->m_CommandsCompletedEvent.Signal();
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
  return thread->Exit(0);
}

ThreadDispatcher::ThreadDispatcher() : m_Receiver(NULL) {
}

int
ThreadDispatcher::SendCommand(CommandParams const& cmd) {
  int retval = 0;
  {
    while (!m_SIDWriteThread.IsStarted()) {
      ThreadObject::Yield();
    }
    if (cmd.m_Command == CommandParams::Flush) {
      m_Receiver->Flush();
#ifdef USE_COMMAND_PENDING_EVENT
      m_Receiver->m_CommandPendingEvent.Signal();
#endif
      while (m_Receiver->m_CommandRead != m_Receiver->m_CommandWrite) {
        ThreadObject::Yield();
      }
      return 0;
    }
    assert(!m_Receiver->IsReadResultReady());
    if (!m_Receiver->TryPutCommand(cmd)) {
#ifdef USE_COMMAND_PENDING_EVENT
      m_Receiver->m_CommandPendingEvent.Signal();
#endif
      return -1;
    }
    switch (cmd.m_Command)
    {
    case CommandParams::Write:
#ifdef USE_COMMAND_PENDING_EVENT
      if (!cmd.m_IsBuffered) {
        m_Receiver->m_CommandPendingEvent.Signal();
      }
#endif
      break;
    case CommandParams::Read:
      // BLOCKING! Should be re-engineered to allow an async interface
#ifdef USE_COMMAND_PENDING_EVENT
      m_Receiver->m_CommandPendingEvent.Signal();
#endif
      // Block until read result available
      while (!m_Receiver->IsReadResultReady()) {
        ThreadObject::Yield();
      }
      retval = m_Receiver->ReadResult();
      assert(!m_Receiver->IsReadResultReady());
      break;
    default:
#ifdef USE_COMMAND_PENDING_EVENT
      m_Receiver->m_CommandPendingEvent.Signal();
#endif
      break;
    }
  }
#ifdef WAIT_FOR_COMMAND_COMPLETION
  m_Receiver->m_CommandsCompletedEvent.Wait(1000); // Set timeout in case worker thread is interrupted
  m_Receiver->m_CommandsCompletedEvent.Reset();
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
  m_Receiver->Initialize();
  m_Receiver->m_Mutex.Allocate();
  m_Receiver->m_CommandPendingEvent.Allocate("CommandPending");
#ifdef WAIT_FOR_COMMAND_COMPLETION
  m_Receiver->m_CommandsCompletedEvent.Allocate("CommandsCompleted");
#endif
  m_SIDWriteThread.StartThread((ThreadObject::ThreadFunction)ThreadFunction, m_Receiver);
}

void ThreadDispatcher::Uninitialize() {
  m_SIDWriteThread.Abort();
  while (!m_SIDWriteThread.IsExited()) {
    ThreadObject::Yield();
  }
  m_Receiver->Uninitialize();
#ifdef WAIT_FOR_COMMAND_COMPLETION
  m_Receiver->m_CommandsCompletedEvent.DeAllocate();
#endif
  m_Receiver->m_CommandPendingEvent.DeAllocate();
  m_Receiver->m_Mutex.DeAllocate();
  delete m_Receiver;
  m_Receiver = NULL;
}

bool ThreadDispatcher::IsAsync() {
  return true;
}

int ThreadDispatcher::DeviceCount() {
  while (!m_SIDWriteThread.IsStarted()) {
    ThreadObject::Yield();
  }
  return s_DeviceCount;
}

}
