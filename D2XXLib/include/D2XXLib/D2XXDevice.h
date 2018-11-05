#ifndef D2XXDEVICE_H_INCLUDED
#define D2XXDEVICE_H_INCLUDED

/* Adapted from https://github.com/ssidko/DMT/blob/master/D2xx.h */

#include "ftd2xx.h"
#include "Utilities.h"
#include <vector>

namespace D2XXLib {

class D2XXDevice : public IO
{
private:
  FT_HANDLE handle;
  FT_STATUS ft_status;
  FT_DEVICE_LIST_INFO_NODE info;

  BOOL OpenEx(PVOID arg, DWORD flag);
  BOOL OpenByIndex(DWORD index);
  BOOL OpenBySerialNumber(const char *serial_number);
  BOOL OpenByDescription(const char *description);
  BOOL OpenByLocation(DWORD location);

public:
  D2XXDevice(const FT_DEVICE_LIST_INFO_NODE *device_info);
  ~D2XXDevice();
  virtual BOOL Open();
  virtual void Close(void);
  virtual DWORD Read(void *buffer, DWORD count);
  virtual DWORD Write(void *buffer, DWORD count);
  BOOL IsOpen();
  BOOL Initialise(void);
  BOOL SetTimeouts(DWORD read_timeout, DWORD write_timeout);
  BOOL SetBaudRate(DWORD baud_rate);
  BOOL SetDataCharacteristics(BYTE word_length, BYTE stop_bits, BYTE parity);
  BOOL GetQueueStatus(DWORD *rx_bytes, DWORD *tx_bytes, DWORD *event_status);
#ifdef USE_FTCHIPID
  BOOL GetUniqueChipID(DWORD *chip_id);
#endif
  DWORD FT_Status(void);
  DWORD GetType(void);
  DWORD GetVIDPID(void);
  DWORD GetLocationID(void);
  const char *GetSerialNumber(void);
  const char *GetDescription(void);
  void DisplayInfo(void);
  DWORD Send(std::vector<unsigned char> const& data);
  DWORD Send(unsigned char const* data, size_t size);
  DWORD Recv(std::vector<unsigned char>& data);
};

}

#endif
