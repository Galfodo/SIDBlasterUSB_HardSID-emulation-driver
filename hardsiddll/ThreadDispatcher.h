#ifndef SIDBLASTER_THREADDISPATCHER_H_INCLUDED
#define SIDBLASTER_THREADDISPATCHER_H_INCLUDED

#include "SIDBlasterInterface.h"
#include "CommandDispatcher.h"
#include "ThreadObject.h"
#include "DriverDefs.h"

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

  ThreadCommandReceiver*  m_Receiver;
  ThreadObject            m_SIDWriteThread;
};

}

#endif
