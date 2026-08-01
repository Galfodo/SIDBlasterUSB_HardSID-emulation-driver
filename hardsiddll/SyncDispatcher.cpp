
#include "SyncDispatcher.h"
#include "SIDBlasterInterface.h"
#include "DebugSIDBlasterInterface.h"
#include "SIDBlasterInterfaceImpl.h"

#include <assert.h>

namespace SIDBlaster {

static int s_DeviceCount;

SyncDispatcher::SyncDispatcher() : m_bufferSize(SIDBlasterInterface::DEFAULT_WRITE_BUFFER_SIZE) {
}

int SyncDispatcher::SendCommand(CommandParams const& cmd) {
  if (cmd.m_Device < 0 || cmd.m_Device >= (int)m_SIDBlasters.size()) {
    return 0;
  }
  SIDBlasterInterface* sidblaster = m_SIDBlasters.at(cmd.m_Device);
  switch (cmd.m_Command)
  {
  case CommandParams::Write:
    sidblaster->Write(cmd.m_Reg, cmd.m_Data);
    break;
  case CommandParams::Read:
    {
      byte data = 0;
      byte rxlen = sidblaster->Read((byte)cmd.m_Reg, data);
      return rxlen | ((int)data << 8);
    }
  case CommandParams::Reset:
    sidblaster->Reset();
    break;
  case CommandParams::Sync:
    sidblaster->Sync();
    break;
  case CommandParams::Flush:
    sidblaster->Flush();
    break;
  case CommandParams::SoftFlush:
    sidblaster->SoftFlush();
    break;
  case CommandParams::Mute:
    sidblaster->Mute((byte)cmd.m_Reg);
    break;
  case CommandParams::MuteAll:
    sidblaster->MuteAll();
    break;
  // Delay/Lock/Unlock/Filter/MuteLine/OpenDevice/CloseDevice: same no-ops as
  // the async CommandReceiver path (devices are opened upfront in Initialize(),
  // Lock/Unlock and Filter aren't implemented at this layer - see handover notes).
  default:
    break;
  }
  return 0;
}

bool SyncDispatcher::IsAsync() {
  return false;
}

void SyncDispatcher::Initialize() {
#ifdef ENABLE_ATTACH_DEBUGGER
  MessageBox(0, "Attach debugger now!", "hardsid.dll", MB_OK);
#endif
  int deviceID = 0;
  for (int i = 0; i < SIDBlasterEnumerator::Instance()->DeviceCount(); ++i) {
    m_SIDBlasters.push_back(SIDBlasterEnumerator::Instance()->CreateInterface(this, deviceID++));
  }
  for (int i = 0; i < (int)m_SIDBlasters.size(); ++i) {
    m_SIDBlasters[i]->SetWriteBufferSize(m_bufferSize);
    m_SIDBlasters[i]->Open();
  }
}

void SyncDispatcher::SetWriteBufferSize(int bufferSize) {
  m_bufferSize = bufferSize;
  for (int i = 0; i < (int)m_SIDBlasters.size(); ++i) {
    m_SIDBlasters[i]->SetWriteBufferSize(bufferSize);
  }
}

void SyncDispatcher::Uninitialize() {
  for (int i = 0; i < (int)m_SIDBlasters.size(); ++i) {
    m_SIDBlasters[i]->Close();
    SIDBlasterEnumerator::Instance()->ReleaseInterface(m_SIDBlasters[i]);
  }
  m_SIDBlasters.clear();
}

int SyncDispatcher::DeviceCount() {
  return (int)m_SIDBlasters.size();
}

void SyncDispatcher::Log(const char* format, ...) {
}

}
