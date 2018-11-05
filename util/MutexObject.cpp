/**
 * Mutex abstraction. 
 * Written by stein.pedersen@hue.no
 */

#include "MutexObject.h"
#include <cassert>

namespace SIDBlaster {

MutexObject::MutexObject() : m_Allocated(false) {
}

MutexObject::~MutexObject() {
  if (m_Allocated) {
    DeAllocate();
  }
}

void
MutexObject::Allocate() {
#ifdef WIN32
  InitializeCriticalSection(&m_NativeMutex);
#else
#error "not implemented"
#endif
  m_Allocated = true;
}

void
MutexObject::DeAllocate() {
  assert(m_Allocated);
#ifdef WIN32
  DeleteCriticalSection(&m_NativeMutex);
#else
#error "not implemented"
#endif
  m_Allocated = false;
}

void
MutexObject::Lock() {
  assert(m_Allocated);
#ifdef WIN32
  EnterCriticalSection(&m_NativeMutex);
#else
#error "not implemented"
#endif
}

void
MutexObject::Unlock() {
#ifdef WIN32
  LeaveCriticalSection(&m_NativeMutex);
#else
#error "not implemented"
#endif
}

MutexLock::MutexLock(MutexObject& mutex) : m_MutexObject(mutex) {
  m_MutexObject.Lock();
}

MutexLock::~MutexLock() {
  m_MutexObject.Unlock();
}

}
