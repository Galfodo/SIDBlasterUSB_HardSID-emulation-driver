#ifndef SIDBLASTER_SIDBLASTERINTERFACE_H_INCLUDED
#define SIDBLASTER_SIDBLASTERINTERFACE_H_INCLUDED

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "ILogger.h"

#include <vector>

#ifdef _MSC_VER
#define SIDB_OVERRIDE override
#else
#define SIDB_OVERRIDE
#endif

namespace SIDBlaster {

typedef unsigned char byte;
#ifdef _MSC_VER
typedef __int64 int64;
#else
#error "need int64 typedef"
#endif

#define PAL_CLOCK 985248.611
#define NTSC_CLOCK 1022727.143

class SIDBlasterInterface
{
public:
  enum
  {
    MAX_WRITE_BUFFER_SIZE = 256
  };

  virtual       ~SIDBlasterInterface();
  virtual void  Open() = 0;
  virtual void  Close() = 0;
  virtual void  Reset() = 0;
  virtual byte  Read(byte reg, byte& data) = 0;
  virtual void  Mute(byte ch) = 0;
  virtual void  MuteAll() = 0;
  virtual void  Sync() = 0;
  virtual void  Delay() = 0;
  virtual void  Write(byte reg, byte data) = 0;
  virtual void  BufferWrite(byte reg, byte data) = 0;
  virtual void  Flush() = 0;
  virtual void  SoftFlush() = 0;
  inline int    DeviceID() const {
                  return m_DeviceID;
                }
  inline int    WriteBufferSize() const {
                  return m_WriteBufferSize;
                }
  void          SetWriteBufferSize(int size);
protected:
                SIDBlasterInterface(ILogger* logger, int deviceID);
  int           m_WriteBufferSize;
  ILogger*      
                m_Logger;
  int           m_DeviceID;
};

class SIDBlasterEnumeratorInterface {
public:
  virtual int   DeviceCount() = 0;
  virtual SIDBlasterInterface* 
                CreateInterface(ILogger* logger, int deviceID) = 0;
  virtual void  ReleaseInterface(SIDBlasterInterface* sidblaster) = 0;
protected:
  virtual       ~SIDBlasterEnumeratorInterface() {}
};

}

#endif
