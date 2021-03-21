#ifndef D2XXDEVICE_H_INCLUDED
#define D2XXDEVICE_H_INCLUDED

/* Adapted from https://github.com/ssidko/DMT/blob/master/D2xx.h */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif
#include "../ftd2xx.h"
#include "Utilities.h"
#include <vector>

enum SID_TYPE {
	SID_TYPE_NONE = 0, SID_TYPE_6581, SID_TYPE_8580
};

namespace D2XXLib {

class D2XXDevice : public IO
{
private:
  FT_HANDLE handle;
  FT_STATUS ft_status;
  FT_DEVICE_LIST_INFO_NODE info;

  bool OpenEx(PVOID arg, DWORD flag);
  bool OpenByIndex(DWORD index);
  bool OpenBySerialNumber(const char *serial_number);
  bool OpenByDescription(const char *description);
  bool OpenByLocation(DWORD location);

public:
  D2XXDevice(const FT_DEVICE_LIST_INFO_NODE *device_info);
  ~D2XXDevice();
  virtual bool Open();
  virtual void Close(void);
  virtual DWORD Read(void *buffer, DWORD count);
  virtual DWORD Write(void *buffer, DWORD count);
  bool IsOpen();
  bool Initialize(void);
  bool SetTimeouts(DWORD read_timeout, DWORD write_timeout);
  bool SetBaudRate(DWORD baud_rate);
  bool SetDataCharacteristics(BYTE word_length, BYTE stop_bits, BYTE parity);
  bool GetQueueStatus(DWORD *rx_bytes, DWORD *tx_bytes, DWORD *event_status);
#ifdef USE_FTCHIPID
  bool GetUniqueChipID(DWORD *chip_id);
#endif
  DWORD FT_Status(void);
  DWORD GetType(void);
  DWORD GetVIDPID(void);
  DWORD GetLocationID(void);
  const char *GetSerialNumber(void);
  const char *GetDescription(void);
  SID_TYPE GetSIDType(void);
  int SetSIDType(DWORD index, SID_TYPE sidtype);
  void DisplayInfo(void);
  DWORD Send(std::vector<unsigned char> const& data);
  DWORD Send(unsigned char const* data, size_t size);
  DWORD Recv(std::vector<unsigned char>& data);
};

}

#endif
