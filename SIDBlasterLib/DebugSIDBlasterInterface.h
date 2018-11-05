#ifndef SIDBLASTER_DEBUGSIDBLASTERINTERFACE_H_INCLUDED
#define SIDBLASTER_DEBUGSIDBLASTERINTERFACE_H_INCLUDED

#include "SIDBlasterInterface.h"

namespace SIDBlaster {

class DebugSIDBlasterInterface : public SIDBlasterInterface
{
public:
  virtual       ~DebugSIDBlasterInterface() SIDB_OVERRIDE;
  virtual void  Open() SIDB_OVERRIDE;
  virtual void  Close() SIDB_OVERRIDE;
  virtual void  Reset() SIDB_OVERRIDE;
  virtual byte  Read(byte reg, byte& data) SIDB_OVERRIDE;
  virtual void  Mute(byte ch) SIDB_OVERRIDE;
  virtual void  MuteAll() SIDB_OVERRIDE;
  virtual void  Sync() SIDB_OVERRIDE;
  virtual void  Write(byte reg, byte data) SIDB_OVERRIDE;
  virtual void  BufferWrite(byte reg, byte data) SIDB_OVERRIDE;
  virtual void  Delay() SIDB_OVERRIDE;
  virtual void  Flush() SIDB_OVERRIDE;
  virtual void  SoftFlush() SIDB_OVERRIDE;
private:
                DebugSIDBlasterInterface(ILogger* logger, int deviceID);

  friend class DebugSIDBlasterEnumerator;
};

class DebugSIDBlasterEnumerator : public SIDBlasterEnumeratorInterface {
  virtual int   DeviceCount() SIDB_OVERRIDE;
  virtual SIDBlasterInterface* 
                CreateInterface(ILogger* logger, int deviceID) SIDB_OVERRIDE;
  virtual void  ReleaseInterface(SIDBlasterInterface* sidblaster) SIDB_OVERRIDE;
  static DebugSIDBlasterEnumerator* 
                Instance();
  friend class SIDBlasterEnumerator;
};

}

#endif
