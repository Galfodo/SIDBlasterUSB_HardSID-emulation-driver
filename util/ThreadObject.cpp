/**
 * Simple thread abstraction.
 * Written by stein.pedersen@hue.no
 */

#include "ThreadObject.h"

namespace SIDBlaster {

ThreadObject::ThreadObject() : m_IsExited(false), m_Abort(false), m_IsStarted(false), m_ExitCode(-1), m_Arg(NULL) {
}

ThreadObject::~ThreadObject() {
}

void
ThreadObject::StartThread(ThreadObject::ThreadFunction func, void* arg) {
  m_Arg = arg;
#ifdef WIN32
  m_ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, this, 0, &m_ThreadId);
#else
#error "not implemented"
#endif
}

void
ThreadObject::WaitComplete() {
#ifdef WIN32
  #if 0
    // Neither of these methods work when called while unloading dll
    #if 0
      WaitForSingleObject(m_ThreadHandle, INFINITE);
    #else
      while (true) {
        DWORD exitcode = 0;
        GetExitCodeThread(m_ThreadHandle, &exitcode);
        if (exitcode != STILL_ACTIVE) {
          return;
        }
        Sleep(0);
      }
  #endif
#else
  while (!IsExited())
  {
    ThreadObject::Yield();
  }
#endif

#else
#error "not implemented"
#endif
}

void ThreadObject::Yield() {
#ifdef WIN32
  ::Sleep(0);
#else
#error "not implemented"
#endif
}

void ThreadObject::Sleep(int milliseconds) {
#ifdef WIN32
  ::Sleep(milliseconds);
#else
#error "not implemented"
#endif
}

void ThreadObject::Abort() {
  m_Abort = true;
}

bool ThreadObject::ShouldAbort() {
  return m_Abort;
}

int ThreadObject::Exit(int exitcode) {
  m_ExitCode = exitcode;
  m_IsExited = true;
  return exitcode;
}

bool ThreadObject::IsExited() {
  if (m_IsExited) {
    return true;
  }
#ifdef WIN32
  else {
    DWORD exitcode = 0;
    GetExitCodeThread(m_ThreadHandle, &exitcode);
    if (exitcode != STILL_ACTIVE) {
      return true;
    }
    return false;
  }
#else
  return false;
#endif
}

void* ThreadObject::Arg() {
  return m_Arg;
}

bool ThreadObject::IsStarted() {
  return m_IsStarted;
}

void ThreadObject::Start() {
  m_IsStarted = true;
}

}
