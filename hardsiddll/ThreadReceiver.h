#ifndef SIDBLASTER_THREADRECEIVER_H_INCLUDED
#define SIDBLASTER_THREADRECEIVER_H_INCLUDED

#include "SIDBlasterInterface.h"
#include "CommandDispatcher.h"
#include "CommandReceiver.h"
#include "DriverDefs.h"

#include <mutex>
#include <condition_variable>
#include <thread>

namespace SIDBlaster {

class ThreadCommandReceiver : public CommandReceiver {
public:
  enum {
    BUFFERSIZE = 64
  };

  ThreadCommandReceiver();

#ifdef WAIT_FOR_COMMAND_COMPLETION
  std::condition_variable
    m_CommandsCompletedEvent;

  std::mutex
    m_CommandsCompletedMutex;
#endif

  std::condition_variable
    m_CommandPendingEvent;

  std::mutex
    m_CommandPendingMutex;

  CommandParams
    m_CommandBuffer[BUFFERSIZE];

  int
    m_CommandWrite,
    m_CommandRead;

  bool
    m_Flush;

  inline void Flush() {
    m_Flush = true;
  }

  inline bool CommandsPending() {
    return m_CommandRead != m_CommandWrite || m_Flush;
  }

  inline bool CommandBufferFull() {
    if (((m_CommandWrite + 1) % BUFFERSIZE) == m_CommandRead) {
      return true;
    }
    return false;
  }

  inline bool TryPutCommand(CommandParams const& params) {
    if (!CommandBufferFull()) {
      int writepos = m_CommandWrite;
      m_CommandBuffer[writepos++] = params;
      m_CommandWrite = (writepos % BUFFERSIZE);
      return true;
    } else {
      return false;
    }
  }

  inline bool TryGetCommand(CommandParams& params) {
    if (m_Flush) {
      m_Flush = false;
      params.m_Command = CommandParams::Flush;
      params.m_Device = 0;
      m_CommandWrite = m_CommandRead;
      return true;
    }
    if (CommandsPending()) {
      int readpos = m_CommandRead;
      params = m_CommandBuffer[readpos++];
      m_CommandRead = (readpos % BUFFERSIZE);
      return true;
    } else {
      return false;
    }
  }

};

}

#endif

