#ifndef _UTILITIES
#define _UTILITIES

#include <list>
#include <assert.h>

namespace D2XXLib
{
#ifdef WIN32
  typedef unsigned long DWORD;
#endif
  typedef void* PVOID;

  class Subject;
  class ManagerNotifier;
  class ManagerObserver;

  enum DeviceEventType {
    kUnknown,
    kPlugged,
    kUnplugged
  };

  class DMException {};

  class IO 
  {
  public:
    virtual ~IO() {}
    virtual bool Open() = 0;
    virtual void Close() = 0;
    virtual DWORD Read(void *buff, DWORD count) = 0;
    virtual DWORD Write(void *buff, DWORD count) = 0;
  };

  class Observer
  {
  protected:
    Observer() {}
  public:
    virtual ~Observer() {}
    virtual void Update(Subject *subject) = 0;
  };

  class Subject
  {
  protected:
    Subject() {}
  public:
    std::list<Observer *> observers;

    virtual ~Subject()
    {
      observers.clear();  
    }

    virtual void Attach(Observer *observer)
    {
      observers.push_back(observer);
    }

    virtual void Detach(Observer *observer)
    {
      std::list<Observer*>::iterator it = observers.begin();
      while (it != observers.end()) {
        if (*it == observer) {
          observers.erase(it);
          break;
        }
      }  
    }

    virtual void Notify()
    {
      std::list<Observer*>::iterator it = observers.begin();
      while (it != observers.end()) {
        (*it)->Update(this);
        ++it;
      }
    }
  };

  class ManagerObserver : public Observer
  {
  public:
    ManagerObserver() {}
    virtual ~ManagerObserver() {}
    virtual void Update(Subject *_pSubject) {}
    virtual void Update(ManagerNotifier *manager, void *device, DWORD event_code) = 0;
  };

  class ManagerNotifier : public Subject
  {
  public:
    ManagerNotifier() {}
    virtual ~ManagerNotifier() {}
    virtual void Notify(void *device, DWORD event_code)
    {
      assert(device);
      std::list<Observer *>::iterator it = observers.begin();
      while (it != observers.end()) {
        ManagerObserver *obs = dynamic_cast<ManagerObserver *>(*it);
        if (obs) obs->Update(this, device, event_code);
        ++it;
      }
    }
  };

}

#endif // _UTILITIES
