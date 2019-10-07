
#define _CRT_SECURE_NO_DEPRECATE

/** 
 * Interprets SIDBlaster CommandParams and calls the appropriate interface 
 * Written by stein.pedersen@hue.no
 */

#include "CommandReceiver.h"
#include "SIDBlasterInterfaceImpl.h"

#include <cassert>
#include <chrono>

namespace SIDBlaster {

CommandReceiver::CommandReceiver() : m_AdaptiveWriteBuffer(false) {
  for (int i = 0; i < TIMING_SAMPLES; ++i) {
    m_TimingSamples[i] = 0;
  }
  m_TimingSampleIndex = 0;
}

CommandReceiver::~CommandReceiver() {
}

void
CommandReceiver::Initialize() {
  QueryPerformanceFrequency((LARGE_INTEGER*)&m_Frq);
  QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTick);
  m_NextTick = m_StartTick;
  m_CycleScale = m_Frq / PAL_CLOCK;
  m_InvertedCycleScale = PAL_CLOCK / m_Frq;
  m_AccumulatedCycles = 0;
  m_IsReadResultReady = false;
  m_ReadResult = 0;
  for (int i = 0; i < SIDBlasterEnumerator::Instance()->DeviceCount(); ++i) {
    m_Devices.push_back(SIDBlasterEnumerator::Instance()->CreateInterface(this, i));
  }
}

void
CommandReceiver::Uninitialize() {
  for (int i = 0; i < (int)m_Devices.size(); ++i) {
    SIDBlasterEnumerator::Instance()->ReleaseInterface(m_Devices[i]);
  }
  m_Devices.clear();
}

void CommandReceiver::ResetTimer() {
  QueryPerformanceCounter((LARGE_INTEGER*)&m_NextTick);
  m_StartTick = m_NextTick;
  m_AccumulatedCycles = 0;
}

int CommandReceiver::WaitForCycle(int cycle) {
  int64 tick = 0;
  QueryPerformanceCounter((LARGE_INTEGER*)&tick);
  int64 waitForTick = m_NextTick + (int64)(cycle * m_CycleScale);
  int64 waitDelta = waitForTick - tick;
  int msec = (int)(waitDelta * 1000 / m_Frq) - 1;
  if (msec > 0) {
    for (int i = 0; i < (int)m_Devices.size(); ++i) {
      m_Devices[i]->SoftFlush();
    }
    {
      std::unique_lock<std::mutex> 
        lockguard(m_WaitMutex);

      m_WaitEvent.wait_for(lockguard, std::chrono::milliseconds(msec));
    }
  }
  while (tick < waitForTick) {
    QueryPerformanceCounter((LARGE_INTEGER*)&tick);
  }
  m_CurrentTick = tick;
  m_NextTick = waitForTick;
  m_AccumulatedCycles += cycle; // For debugging only
  return (int)(waitDelta * m_InvertedCycleScale);
}

void CommandReceiver::Log(const char* format, ...) {
  va_list arg;
  char buffer[1024];
  va_start(arg, format);
  vsprintf(buffer, format, arg);
  va_end(arg);
#ifdef WIN32
  OutputDebugStringA(buffer);
#else
  fprintf(stdout, "%s", buffer);
#endif
}

void CommandReceiver::LogTimestamp() {
  Log("CYC: %08I64X ACC: %08I64X\n", CycleFromTick(m_CurrentTick), m_AccumulatedCycles);
}

int64 CommandReceiver::CycleFromTick(int64 tick) {
  return (int64)((tick - m_StartTick) * m_InvertedCycleScale);
}

int
CommandReceiver::ExecuteCommand(CommandParams const& command) {
  if (m_Devices.empty()) {
    return 0;
  }
  bool buffer_writes = m_Devices[0]->WriteBufferSize() > 0 || m_AdaptiveWriteBuffer;
  int retval = 0;
  int waited_cycles = 0;
  if (command.m_Cycle > 0) {
    waited_cycles = WaitForCycle(command.m_Cycle);
  }
  m_TimingSamples[m_TimingSampleIndex] = waited_cycles;
  m_TimingSampleIndex = (m_TimingSampleIndex + 1) % TIMING_SAMPLES;
  //Log("Waited cycles: %d, buffer size: %d\n", waited_cycles, m_SIDBlaster->WriteBufferSize());
  if (waited_cycles < 0) {
    buffer_writes = true;
  }
  int sum = 0;
  if (m_AdaptiveWriteBuffer) {
    for (int i = 0; i < TIMING_SAMPLES; ++i) {
      sum += m_TimingSamples[i];
    }
    int avg = sum / TIMING_SAMPLES;
    if (avg < -10000) {
      for (int i = 0; i < (int)m_Devices.size(); ++i) {
        m_Devices[i]->SetWriteBufferSize(m_Devices[i]->WriteBufferSize() + 1);
      }
    }
  }
  //LogTimestamp();
  if (command.m_Device >= 0 && command.m_Device < (int)m_Devices.size()) {
    SIDBlasterInterface* sidblaster = m_Devices.at(command.m_Device);
    switch(command.m_Command) {
    case CommandParams::OpenDevice:
      sidblaster->Open();
      break;
    case CommandParams::CloseDevice:
      sidblaster->Close();
      break;
    case CommandParams::Delay:
      // Do nothing...
      break;
    case CommandParams::Write:
      if (buffer_writes) {
        sidblaster->BufferWrite(command.m_Reg, command.m_Data);
      } else {
        sidblaster->Write(command.m_Reg, command.m_Data);
      }
      break;
    case CommandParams::Read:
      {
        byte data = 0;
        retval = sidblaster->Read(command.m_Reg, data) | ((int)data << 8);
        if (data != 0) {
          int debug = 0;
        }
        m_ReadResult = retval;
        m_IsReadResultReady = true;
      }
      break;
    case CommandParams::Mute:
      sidblaster->Mute(command.m_Reg);
      break;
    case CommandParams::MuteAll:
      sidblaster->MuteAll();
      break;
    case CommandParams::Sync:
      sidblaster->Sync();
      ResetTimer();
      break;
    case CommandParams::SoftFlush: // What's the difference?
      sidblaster->SoftFlush();
      break;
    case CommandParams::Flush:
      sidblaster->Flush();
      ResetTimer();
      break;
    case CommandParams::Reset:
      sidblaster->Reset();
      ResetTimer();
      break;
    //case CommandParams::MuteLine: // What does this do?
    //  break;
    case CommandParams::Filter: // What does this do?
      break;
    case CommandParams::Lock: // Should ensure no other applications can obtain a sidblaster device
      break;
    case CommandParams::Unlock:
      break;
    default:
      ; 
    }
  }
  return retval;
}

}
