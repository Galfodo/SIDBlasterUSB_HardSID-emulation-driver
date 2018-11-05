
#include "SyncDispatcher.h"
#include "SIDBlasterInterface.h"
#include "DebugSIDBlasterInterface.h"
#include "SIDBlasterInterfaceImpl.h"

#include <assert.h>

namespace SIDBlaster {

static int s_DeviceCount;

SyncDispatcher::SyncDispatcher() {
}

int SyncDispatcher::SendCommand(CommandParams const& cmd) {
  if (cmd.m_Device >= 0 && cmd.m_Device < (int)m_SIDBlasters.size()) {
    SIDBlasterInterface* sidblaster = m_SIDBlasters.at(cmd.m_Device);
    switch (cmd.m_Command)
    {
    case CommandParams::Write:
      sidblaster->Write(cmd.m_Reg, cmd.m_Data);
      break;
    default:
      break;
    }
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
    m_SIDBlasters[i]->Open();
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
