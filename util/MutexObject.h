#ifndef SIDBLASTER_MUTEXOBJECT_H_INCLUDED
#define SIDBLASTER_MUTEXOBJECT_H_INCLUDED

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace SIDBlaster {

class MutexObject
{
public:
  MutexObject();
  ~MutexObject();
  void Allocate();
  void DeAllocate();
private:
  void Lock();
  void Unlock();
  friend class MutexLock;
#ifdef WIN32
  typedef CRITICAL_SECTION NativeMutex_t;
#else
#error "need typedef NativeMutex_t"
#endif
  bool m_Allocated;
  NativeMutex_t m_NativeMutex;
  MutexObject& operator= (MutexObject const&); 
};

class MutexLock
{
public:
  MutexLock(MutexObject& mutex);
  ~MutexLock();
private:
  MutexLock& operator= (MutexLock const&);
  MutexObject& m_MutexObject;
};

}

#endif
