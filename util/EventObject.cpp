/**
 * Event abstraction. 
 * Written by stein.pedersen@hue.no
 */

#include "EventObject.h"
#include <cstring>
#include <cstdlib>

#ifdef WIN32
#define strdup _strdup
#endif

namespace SIDBlaster {

EventObject::EventObject() : m_Name(NULL) {
}

EventObject::~EventObject() {
}

void 
EventObject::Allocate(const char* name) {
  m_Name = strdup(name);
#ifdef WIN32
  m_hEvent = CreateEvent(NULL, true, false, m_Name);
#else
#error "not implemented"
#endif
}

void 
EventObject::DeAllocate() {
#ifdef WIN32
  CloseHandle(m_hEvent);
  m_hEvent = 0;
#else
#error "not implemented"
#endif
  free((void*)m_Name);
  m_Name = NULL;
}

void
EventObject::Signal() {
#ifdef WIN32
  SetEvent(m_hEvent);
#else
#error "not implemented"
#endif
}

void
EventObject::Wait(int milliseconds) {
#ifdef WIN32
  if (milliseconds < 0)
    milliseconds = INFINITE;
  WaitForSingleObject(m_hEvent, milliseconds);
#else
#error "not implemented"
#endif
}

void
EventObject::Reset() {
#ifdef WIN32
  ResetEvent(m_hEvent);
#else
#error "not implemented"
#endif
}

}
