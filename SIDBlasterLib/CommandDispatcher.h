#ifndef SIDBLASTER_SIDBLASTERAPI_H_INCLUDED
#define SIDBLASTER_SIDBLASTERAPI_H_INCLUDED

#include "SIDBlasterInterface.h"

namespace SIDBlaster {

struct CommandParams
{
  enum Command { // Keeps the order of command ids the same as the original windows host
    MinCommand = 0, 
    Reset = MinCommand,
    Delay,
    Write,
    Read,
    Sync,
    Flush,
    Mute,
    MuteAll,
    SoftFlush,
    Lock,
    MuteLine,
    Filter = MuteLine,
    Unlock,
    OpenDevice,
    CloseDevice,
    NOP,
    MaxCommand
  } m_Command;
  int   m_Device;
  int   m_Reg;
  int   m_Data;
  int   m_Cycle;
  int   m_IsBuffered;

  CommandParams(int device = 0, Command cmd = NOP, int reg = 0, int data = 0, int cycle = -1, bool is_buffered = false) :
    m_Device(device),
    m_Command(cmd),
    m_Reg(reg),
    m_Data(data),
    m_Cycle(cycle),
    m_IsBuffered(is_buffered ? 1 : 0)
  {
  }
};

class CommandDispatcher {
public:
  virtual int   SendCommand(CommandParams const& cmd) = 0;
  virtual bool  IsAsync() = 0;
  virtual void  Initialize() = 0;
  virtual void  Uninitialize() = 0;
  virtual int   DeviceCount() = 0;
  virtual void  SetWriteBufferSize(int bufferSize) = 0;
};

}

#endif
