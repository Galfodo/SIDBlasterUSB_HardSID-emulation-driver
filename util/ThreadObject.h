#ifndef SIDBLASTER_THREADOBJECT_H_INCLUDED
#define SIDBLASTER_THREADOBJECT_H_INCLUDED

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef Yield
#endif

namespace SIDBlaster {

class ThreadObject
{
public:
  typedef int (*ThreadFunction)(ThreadObject*);

                ThreadObject();
                ~ThreadObject();

  // Called from parent thread
  void          StartThread(ThreadFunction func, void* arg);
  bool          IsStarted();
  void          WaitComplete();
  void          Abort();
  bool          IsExited();
  int           ExitCode();

  // Called from ThreadFunction
  void          Start();
  void*         Arg();
  bool          ShouldAbort();
  int           Exit(int exitcode);

  // Generic thread yield and sleep
  static void   Yield();
  static void   Sleep(int milliseconds);

private:
  ThreadObject  operator= (ThreadObject const&);
  void*         m_Arg;
  int           m_ExitCode;
  bool          m_IsExited;
  bool          m_Abort;
  bool          m_IsStarted;

#ifdef WIN32
  DWORD         m_ThreadId;
  HANDLE        m_ThreadHandle;
#endif
};

}

#endif
