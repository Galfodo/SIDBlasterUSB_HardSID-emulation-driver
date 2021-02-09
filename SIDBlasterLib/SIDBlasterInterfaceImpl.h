#ifndef SIDBLASTER_SIDBLASTERINTERFACEIMPL_H_INCLUDED
#define SIDBLASTER_SIDBLASTERINTERFACEIMPL_H_INCLUDED

#include "SIDBlasterInterface.h"

#include <vector>

namespace D2XXLib {
  class D2XXDevice;
  class D2XXManager;
}

namespace SIDBlaster {

class CommandReceiver;

class SIDBlasterInterfaceImpl : public SIDBlasterInterface
{
public:
  virtual               ~SIDBlasterInterfaceImpl() SIDB_OVERRIDE;
  virtual void          Open() SIDB_OVERRIDE;
  virtual void          Close() SIDB_OVERRIDE;
  virtual void          Reset() SIDB_OVERRIDE;
  virtual byte          Read(byte reg, byte& data) SIDB_OVERRIDE;
  virtual void          Mute(byte ch) SIDB_OVERRIDE;
  virtual void          MuteAll() SIDB_OVERRIDE;
  virtual void          Sync() SIDB_OVERRIDE;
  virtual void          Write(byte reg, byte data) SIDB_OVERRIDE;
  virtual void          BufferWrite(byte reg, byte data) SIDB_OVERRIDE;
  virtual void          Delay() SIDB_OVERRIDE;
  virtual void          Flush() SIDB_OVERRIDE;
  virtual void          SoftFlush() SIDB_OVERRIDE;
private:
                        SIDBlasterInterfaceImpl(ILogger* logger, int deviceID, D2XXLib::D2XXDevice * device);
  void                  FlushInternal();

  // Packets:
  std::vector<byte>     m_MuteCh;
  std::vector<byte>     m_MuteAll;

  D2XXLib::D2XXDevice * m_SID;

  int                   m_DeviceID;
  bool                  m_isOpen;

  byte                  m_WriteBuffer[MAX_WRITE_BUFFER_SIZE * 2];
  int                   m_WritePos;

  friend class SIDBlasterEnumerator;
};

class SIDBlasterEnumerator : public SIDBlasterEnumeratorInterface {
public:
  virtual int           DeviceCount() SIDB_OVERRIDE;
  virtual SIDBlasterInterface* 
                        CreateInterface(ILogger* logger, int deviceID) SIDB_OVERRIDE;
  virtual void          ReleaseInterface(SIDBlasterInterface* sidblaster) SIDB_OVERRIDE;
  static SIDBlasterEnumerator* 
                        Instance();
private:
                        SIDBlasterEnumerator();
                        ~SIDBlasterEnumerator();
  
  std::vector<D2XXLib::D2XXDevice*>
                        m_Devices;
  std::vector<bool>     m_DevicesAllocated;
};

}

#endif
