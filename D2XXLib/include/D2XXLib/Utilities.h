#ifndef _UTILITIES
#define _UTILITIES

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#else
typedef unsigned int DWORD;
typedef int BOOL;
#endif

#include <list>
#include <windows.h>
#include <assert.h>

namespace D2XXLib
{
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
    virtual BOOL Open() = 0;
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

namespace test
{
  class Notifier;

  class Observer
  {
  public:
    virtual ~Observer() {}
    virtual void Update(Notifier *notifier, DWORD event_code = 0, void *param = NULL) = 0;
  };

  class Notifier
  {
  private:
    std::list<Observer *> observers;

  public:
    virtual ~Notifier()
    {
      observers.clear();
    }

    void Attach(Observer *observer)
    {
      assert(observer);
      observers.push_back(observer);
    }

    void Detach(Observer *observer)
    {
      assert(observer);
      std::list<Observer *>::iterator it = observers.begin();
      while (it != observers.end()) {
        if (*it == observer) {
          observers.erase(it);
          break;
        }
      }
    }

    void Notify(DWORD event_code = 0, void *param = NULL)
    {
      std::list<Observer*>::iterator it = observers.begin();
      while (it != observers.end()) {
        (*it)->Update(this, event_code, param);
        ++it;
      }      
    }
  };
}

#endif // _UTILITIES
