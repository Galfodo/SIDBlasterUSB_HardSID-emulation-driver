#ifndef SIDBLASTER_COMMANDRECEIVER_H_INCLUDED
#define SIDBLASTER_COMMANDRECEIVER_H_INCLUDED

#include "CommandDispatcher.h"
#include "ILogger.h"

#include <vector>
#include <chrono>

namespace SIDBlaster {

class CommandReceiver : public ILogger {
public:
  typedef std::chrono::high_resolution_clock::time_point timestamp_t;
  typedef std::chrono::nanoseconds  duration_t;
  typedef std::nano                 ratio_t;

                        CommandReceiver();
                        ~CommandReceiver();
  int                   ExecuteCommand(CommandParams const& command);
  void                  Initialize(int bufferSize);
  void                  Uninitialize();
  void                  ResetTimer();
  int                   WaitForCycle(int cycle);
  virtual void          Log(const char* format, ...) override;
  void                  LogTimestamp();
  inline int            DeviceCount() { 
                          return (int)m_Devices.size(); 
                        }
  inline SIDBlasterInterface*
                        Device(int deviceID) { 
                          if (deviceID >= 0 && deviceID < (int)m_Devices.size()) {
                            return m_Devices.at(deviceID);
                          } else {
                            return NULL;
                          }
                        }

  inline bool           IsReadResultReady() {
                          return m_IsReadResultReady;
                        }
  inline int            ReadResult() {
                          m_IsReadResultReady = false;
                          return m_ReadResult;
                        }
//protected:
  std::vector<SIDBlasterInterface*>
                        m_Devices;
  enum
  {
    TIMING_SAMPLES = 8
  };
  int64                 CycleFromTimestamp(timestamp_t tick);
  timestamp_t           m_StartTime;
  timestamp_t           m_NextTime;
  timestamp_t           m_CurrentTime;
  double                m_CPUcycleDuration;
  double                m_InvCPUcycleDurationNanoSeconds;

  int64                 m_AccumulatedCycles;

  bool                  m_AdaptiveWriteBuffer;

  int                   m_TimingSamples[TIMING_SAMPLES];
  int                   m_TimingSampleIndex;

  bool                  m_IsReadResultReady;
  int                   m_ReadResult;
};

}

#endif
