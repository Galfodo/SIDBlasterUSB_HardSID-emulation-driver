
#define _CRT_SECURE_NO_DEPRECATE

/** 
 * Interprets SIDBlaster CommandParams and calls the appropriate interface 
 * Written by stein.pedersen@hue.no
 */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include "CommandReceiver.h"
#include "SIDBlasterInterfaceImpl.h"

#include <cassert>
#include <chrono>
#include <thread>

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
CommandReceiver::Initialize(int bufferSize) {
  m_IsReadResultReady               = false;
  m_ReadResult                      = 0;
  m_CPUcycleDuration                = ratio_t::den / PAL_CLOCK;
  m_InvCPUcycleDurationNanoSeconds  = 1.0 / (1000000000 / PAL_CLOCK);
  ResetTimer();
  int tdc = SIDBlasterEnumerator::Instance()->DeviceCount();
  for (int i = 0; i < tdc; ++i) {
    m_Devices.push_back(SIDBlasterEnumerator::Instance()->CreateInterface(this, i));
	m_Devices[i]->SetWriteBufferSize(bufferSize);
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
  m_StartTime = m_NextTime = std::chrono::high_resolution_clock::now();
  m_AccumulatedCycles = 0;
}

int CommandReceiver::WaitForCycle(int cycle) {
  timestamp_t now = std::chrono::high_resolution_clock::now();
  double dur_ = cycle * m_CPUcycleDuration;
  duration_t dur = (duration_t)(int64_t)dur_;
  auto target_time = m_NextTime + dur;
  auto target_delta = target_time - now;
  auto wait_msec = std::chrono::duration_cast<std::chrono::milliseconds>(target_delta);
  auto wait_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(target_delta);
  if (wait_msec.count() > 0) {
    for (int i = 0; i < (int)m_Devices.size(); ++i) {
      m_Devices[i]->SoftFlush();
    }
    std::this_thread::sleep_for(wait_msec);
  }
  while (now < target_time) {
    now = std::chrono::high_resolution_clock::now();
  }
  m_CurrentTime       = now;
  m_NextTime          = target_time;
  int waited_cycles   = (int)(wait_nsec.count() * m_InvCPUcycleDurationNanoSeconds);
  return waited_cycles;
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
  Log("CYC: %08I64X ACC: %08I64X\n", CycleFromTimestamp(m_CurrentTime), m_AccumulatedCycles);
}

int64 CommandReceiver::CycleFromTimestamp(timestamp_t timestamp) {
  auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp - m_StartTime);
  return (int64)(nsec.count() * m_InvCPUcycleDurationNanoSeconds);
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
