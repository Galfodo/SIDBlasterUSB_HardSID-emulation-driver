#ifndef SIDBLASTER_ILOGGER_H_INCLUDED
#define SIDBLASTER_ILOGGER_H_INCLUDED

namespace SIDBlaster {

struct ILogger
{
  virtual ~ILogger() {}
  virtual void Log(const char*, ...) = 0;
};

}
#endif
