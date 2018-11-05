#ifndef SIDBLASTER_EVENTOBJECT_H_INCLUDED
#define SIDBLASTER_EVENTOBJECT_H_INCLUDED

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace SIDBlaster {

class EventObject
{
public:
  EventObject();
  ~EventObject();
  void Allocate(const char* name);
  void DeAllocate();
  void Signal();
  void Wait(int milliseconds = -1);
  void Reset();
private:
  const char* m_Name;
#ifdef WIN32
  HANDLE m_hEvent;
#endif
  EventObject& operator= (EventObject const&);
};

}

#endif
