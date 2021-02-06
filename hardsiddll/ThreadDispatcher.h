#ifndef SIDBLASTER_THREADDISPATCHER_H_INCLUDED
#define SIDBLASTER_THREADDISPATCHER_H_INCLUDED

#include "SIDBlasterInterface.h"
#include "CommandDispatcher.h"
#include "DriverDefs.h"

#include <thread>
#include <mutex>

namespace SIDBlaster {

class ThreadCommandReceiver;

class ThreadDispatcher : public CommandDispatcher {
public:
                          ThreadDispatcher();
  virtual void            Initialize() SIDB_OVERRIDE;
  virtual void            Uninitialize() SIDB_OVERRIDE;
  virtual int             SendCommand(CommandParams const& cmd) SIDB_OVERRIDE;
  virtual bool            IsAsync() SIDB_OVERRIDE;
  virtual int             DeviceCount() SIDB_OVERRIDE;

  void                    EnsureInitialized();
  void                    setWriteBufferSize(int bufferSize);

  ThreadCommandReceiver*  m_Receiver;
  std::thread             m_SIDWriteThread;
  std::mutex              m_SIDWriteThreadMutex;
  bool                    m_AbortSIDWriteThread;
  bool                    m_IsInitialized;
  int                     m_bufferSize;
};

}

#endif
