#ifndef SIDBLASTER_SYNCCOMMANDDISPATCHER_H_INCLUDED
#define SIDBLASTER_SYNCCOMMANDDISPATCHER_H_INCLUDED

#include "SIDBlasterInterface.h"
#include "CommandDispatcher.h"
#include "DriverDefs.h"
#include "ILogger.h"
#include <vector>

namespace SIDBlaster {

class SyncDispatcher : public CommandDispatcher, ILogger {
public:
                        SyncDispatcher();
  virtual int           SendCommand(CommandParams const& cmd) SIDB_OVERRIDE;
  virtual bool          IsAsync() SIDB_OVERRIDE;
  virtual void          Initialize() SIDB_OVERRIDE;
  virtual void          Uninitialize() SIDB_OVERRIDE;
  virtual int           DeviceCount() SIDB_OVERRIDE;
  virtual void          Log(const char* format, ...) SIDB_OVERRIDE;
private:
  std::vector<SIDBlasterInterface*>
                        m_SIDBlasters;
};

}

#endif
