/**
 * Simple Debug SIDBlasterInterface. Prints all incoming commands.
 * Written by stein.pedersen@hue.no
 */

#define _CRT_SECURE_NO_DEPRECATE
#include "DebugSIDBlasterInterface.h"
#include "CommandReceiver.h"

// Number of debug interfaces
#define DEBUG_DEVICE_COUNT 0

#ifndef WIN32
#include <cstdio>
#endif

namespace SIDBlaster {

DebugSIDBlasterInterface::DebugSIDBlasterInterface(ILogger* logger, int deviceID) : SIDBlasterInterface(logger, deviceID) {
}

DebugSIDBlasterInterface::~DebugSIDBlasterInterface() {
}

void DebugSIDBlasterInterface::Open() {
  m_Logger->Log("DebugSIDBlasterInterface::Open(%d)\n");
}

void DebugSIDBlasterInterface::Close() {
  m_Logger->Log("DebugSIDBlasterInterface::Close()\n");
}

void DebugSIDBlasterInterface::Reset() {
  m_Logger->Log("DebugSIDBlasterInterface::Reset()\n");
}

byte DebugSIDBlasterInterface::Read(byte reg, byte& data) {
  m_Logger->Log("DebugSIDBlasterInterface::Read(register = %02x)\n", (int)reg);
  data = 0xff;
  return 0;
}

void DebugSIDBlasterInterface::Mute(byte ch) {
  m_Logger->Log("DebugSIDBlasterInterface::Mute()\n");
}

void DebugSIDBlasterInterface::MuteAll() {
  m_Logger->Log("DebugSIDBlasterInterface::MuteAll()\n");
}

void
DebugSIDBlasterInterface::Delay() {
  m_Logger->Log("DebugSIDBlasterInterface::Delay()\n");
}


void DebugSIDBlasterInterface::Sync() {
  m_Logger->Log("DebugSIDBlasterInterface::Sync()\n");
}

void DebugSIDBlasterInterface::Write(byte reg, byte data) {
  m_Logger->Log("DebugSIDBlasterInterface::Write(register = %02x, data = %02x)\n", (int)reg, (int)data);
}

void DebugSIDBlasterInterface::BufferWrite(byte reg, byte data) {
  m_Logger->Log("DebugSIDBlasterInterface::BufferWrite(register = %02x, data = %02x)\n", (int)reg, (int)data);
}

void DebugSIDBlasterInterface::Flush() {
  m_Logger->Log("DebugSIDBlasterInterface::Flush()\n");
}

void DebugSIDBlasterInterface::SoftFlush() {
  m_Logger->Log("DebugSIDBlasterInterface::SoftFlush()\n");
}

int DebugSIDBlasterEnumerator::DeviceCount() {
  return DEBUG_DEVICE_COUNT;
}

SIDBlasterInterface* 
DebugSIDBlasterEnumerator::CreateInterface(ILogger* logger, int deviceID) {
  return new DebugSIDBlasterInterface(logger, deviceID);
}

void  
DebugSIDBlasterEnumerator::ReleaseInterface(SIDBlasterInterface* sidblaster) {
  delete sidblaster;
}

DebugSIDBlasterEnumerator* 
DebugSIDBlasterEnumerator::Instance() {
  static DebugSIDBlasterEnumerator
    s_Instance;

  return &s_Instance;
}

}
