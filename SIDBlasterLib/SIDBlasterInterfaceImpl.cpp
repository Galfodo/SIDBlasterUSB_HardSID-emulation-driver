
/** 
 * Communicate with SIDBlaster via d2xx driver. 
 * Adapted from HardSID_Host.bas by stein.pedersen@hue.no
 **/

#include "SIDBlasterInterfaceImpl.h"
#include "DebugSIDBlasterInterface.h"

#include <assert.h>

#include "D2XXLib/D2XXDevice.h"
#include "D2XXLib/D2XXDeviceManager.h"

using namespace D2XXLib;

namespace SIDBlaster {
  
SIDBlasterInterfaceImpl::SIDBlasterInterfaceImpl(ILogger* logger, int deviceID, D2XXLib::D2XXDevice * device) : 
  SIDBlasterInterface(logger, deviceID),
  m_isOpen(false),
  m_SID(device),
  m_WritePos(0)
{
  char
    buffer[256];

  assert(sizeof(LARGE_INTEGER) == sizeof(int64));
  memset(buffer, 0, sizeof(buffer));
  if (GetEnvironmentVariableA("SIDBLASTERUSB_WRITEBUFFER_SIZE", buffer, sizeof(buffer)))
  {
    buffer[sizeof(buffer)-1] = '\0';
    if (*buffer) {
      int write_buffer_size = atoi(buffer);
      SetWriteBufferSize(write_buffer_size);
    }
  }
}

SIDBlasterInterfaceImpl::~SIDBlasterInterfaceImpl() {
  if (m_isOpen) {
    Close();
  }
}

void SIDBlasterInterfaceImpl::Open() {
  if (m_SID && !m_isOpen) {
    if (!m_SID->Open() || !m_SID->Initialize()) {
      m_SID = NULL;
      fprintf(stderr, "Failed to initialize SIDBlaster.\n");
      return;
    }
    m_MuteCh.resize(4);
    m_MuteAll.resize(12);

    m_MuteAll[0] = 0xe0;
    m_MuteAll[2] = 0xe1;
    m_MuteAll[4] = 0xe7;
    m_MuteAll[6] = 0xe8;
    m_MuteAll[8] = 0xee;
    m_MuteAll[10] = 0xef;
    m_isOpen = true;
    Flush();
  }
}

void
SIDBlasterInterfaceImpl::Close() {
  if (m_SID && m_isOpen) {
    Reset();
    m_SID = NULL;
    m_isOpen = false;
  }
}

void
SIDBlasterInterfaceImpl::Sync() {
  SoftFlush();
}

byte
SIDBlasterInterfaceImpl::Read(byte reg, byte& data) {
  SoftFlush();
  data = 0;
  if (m_SID) {
    reg |= 0xa0;
    m_SID->Send(&reg, 1);
    std::vector<byte> rxqueue;
    do {
    } while (m_SID->Recv(rxqueue) == 0);
    if (!rxqueue.empty()) {
      data = rxqueue.back();
    }
    return (byte)rxqueue.size();
  }
  return 0;
}

void
SIDBlasterInterfaceImpl::Mute(byte ch) {
  SoftFlush();
  m_MuteCh[0] = (ch * 7 + 0) | 0xe0;
  m_MuteCh[2] = (ch * 7 + 1) | 0xe0;
  if (m_SID) {
    m_SID->Send(m_MuteCh);
  }
}

void
SIDBlasterInterfaceImpl::MuteAll() {
  SoftFlush();
  if (m_SID) {
    m_SID->Send(m_MuteAll);
  }
}

void
SIDBlasterInterfaceImpl::Reset() {
  Flush();
  Sync();
  MuteAll();
}

void 
SIDBlasterInterfaceImpl::Write(byte reg, byte data) {
  if (m_WritePos) {
    SoftFlush();
  }
  byte buf[2];
  buf[0] = reg | 0xe0;
  buf[1] = data;
  if (m_SID) {
    m_SID->Send(buf, 2);
  }
}

void 
SIDBlasterInterfaceImpl::BufferWrite(byte reg, byte data) {
  if (m_WritePos >= WriteBufferSize()) {
    SoftFlush();
  }
  m_WriteBuffer[m_WritePos++] = reg | 0xe0;
  m_WriteBuffer[m_WritePos++] = data;
}

void
SIDBlasterInterfaceImpl::FlushInternal() {
  m_WritePos = 0;
}

void
SIDBlasterInterfaceImpl::Delay() {
}

void
SIDBlasterInterfaceImpl::Flush() {
  FlushInternal();
}

void
SIDBlasterInterfaceImpl::SoftFlush() {
  if (m_WritePos)
  {
    if (m_SID) {
      m_SID->Send(m_WriteBuffer, m_WritePos);
    }
    m_WritePos = 0;
  }
}

SIDBlasterEnumerator::SIDBlasterEnumerator() {
  m_Manager = NULL;
}

SIDBlasterEnumerator::~SIDBlasterEnumerator() {
  delete m_Manager;
}

int   
SIDBlasterEnumerator::DeviceCount() {
  if (m_Manager == NULL) {
    m_Manager = new D2XXManager();
    m_Manager->Rescan();
    for (int i = 0; i < (int)m_Manager->Count(); ++i) {
      m_Devices.push_back(m_Manager->GetDevice(i));
      m_DevicesAllocated.push_back(false);
    }
  }
  return (int)m_Devices.size() + DebugSIDBlasterEnumerator::Instance()->DeviceCount();
}

SIDBlasterInterface* 
SIDBlasterEnumerator::CreateInterface(ILogger* logger, int deviceID) {
  if (deviceID < (int)m_Devices.size()) {
    assert(!m_DevicesAllocated[deviceID]);
    D2XXDevice* sid = m_Devices[deviceID];
    m_DevicesAllocated[deviceID] = true;
    return new SIDBlasterInterfaceImpl(logger, deviceID, sid);
  } else {
    return DebugSIDBlasterEnumerator::Instance()->CreateInterface(logger, deviceID);
  }
  return NULL;
}

void  
SIDBlasterEnumerator::ReleaseInterface(SIDBlasterInterface* sidblaster) {
  if (sidblaster->DeviceID() < (int)m_Devices.size()) {
    m_DevicesAllocated[sidblaster->DeviceID()] = false;
  }
  delete sidblaster;
}

SIDBlasterEnumerator* 
SIDBlasterEnumerator::Instance() {
  static SIDBlasterEnumerator
    s_Instance;

  return &s_Instance;
}

}
