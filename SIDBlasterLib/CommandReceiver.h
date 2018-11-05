#ifndef SIDBLASTER_COMMANDRECEIVER_H_INCLUDED
#define SIDBLASTER_COMMANDRECEIVER_H_INCLUDED

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include "CommandDispatcher.h"
#include "ILogger.h"
#include "EventObject.h"

#include <vector>

namespace SIDBlaster {

class CommandReceiver : public ILogger {
public:
                        CommandReceiver();
                        ~CommandReceiver();
  int                   ExecuteCommand(CommandParams const& command);
  void                  Initialize();
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
  int64                 CycleFromTick(int64 tick);
  int64                 m_Frq; // Performance Frequency
  int64                 m_StartTick;
  int64                 m_NextTick;
  int64                 m_CurrentTick;
  double                m_CycleScale; // CPU_CYCLE * CYCLESCALE = TICKS (QPC ticks)
  double                m_InvertedCycleScale;

  int64                 m_AccumulatedCycles;

  bool                  m_AdaptiveWriteBuffer;

  EventObject           m_WaitEvent;

  int                   m_TimingSamples[TIMING_SAMPLES];
  int                   m_TimingSampleIndex;

  bool                  m_IsReadResultReady;
  int                   m_ReadResult;
};

}

#endif
