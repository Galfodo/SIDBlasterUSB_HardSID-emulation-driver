#define _CRT_SECURE_NO_DEPRECATE

// Use WaitForSingleObject for long waits
#define USE_MSEC_WAIT 1 

// Default size of device write buffer
#define WRITEBUFFER_DEFAULT_SIZE 16

#define NOMINMAX

#include "SIDBlasterInterface.h"
#include <stdio.h>
#include <stdarg.h>
#include <algorithm>
#include <assert.h>

namespace SIDBlaster {

SIDBlasterInterface::SIDBlasterInterface(ILogger* logger, int deviceID) {
  assert(logger);
  m_WriteBufferSize = WRITEBUFFER_DEFAULT_SIZE;
  m_Logger = logger;
  m_DeviceID = deviceID;
}

SIDBlasterInterface::~SIDBlasterInterface() {
}

void SIDBlasterInterface::SetWriteBufferSize(int size) {
  m_WriteBufferSize = std::max(0, std::min(size, (int)MAX_WRITE_BUFFER_SIZE));
}

}
