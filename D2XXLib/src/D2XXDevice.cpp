
/* Adapted from https://github.com/ssidko/DMT/blob/master/D2xx.h */

#include "D2XXLib/D2XXDevice.h"
#ifdef USE_FTCHIPID
#include "FTChipID.h"
#endif

#include <stdio.h>
#include <vector>
#include <assert.h>

#if defined linux || __APPLE__
  #include <cstring>
  #include <stdint.h>
  #define strcpy_s strcpy
#endif

#define FT_OPEN_BY_INDEX  8

#define FT_READ_TIMEOUT   1000
#define FT_WRITE_TIMEOUT  1000
#define FT_BAUD_RATE      500000 //FT_BAUD_115200
#define FT_WORD_LENGTH    FT_BITS_8
#define FT_STOP_BITS      FT_STOP_BITS_1
#define FT_PARITY         FT_PARITY_NONE

//#define DMT_ENABLE_TRACE

#ifdef DMT_ENABLE_TRACE
#define DMT_TRACE(fmt, val) printf(fmt, val)
#else
#define DMT_TRACE(fmt, val)
#endif

namespace D2XXLib
{

class D2XXManager;

D2XXManager *GetD2XXManager(void);

bool 
D2XXDevice::OpenEx(PVOID arg, DWORD flag) {
  Close();
  if (flag & FT_OPEN_BY_INDEX) 
    ft_status = FT_Open((int)(intptr_t)arg, &handle);
  else 
    ft_status = FT_OpenEx(arg, flag, &handle);
  if (FT_SUCCESS(ft_status)) 
    info.Flags |= FT_FLAGS_OPENED;
  return FT_SUCCESS(ft_status);
}

bool D2XXDevice::OpenByIndex(DWORD index) {
  return OpenEx((PVOID)(intptr_t)index, FT_OPEN_BY_INDEX);
}

bool D2XXDevice::OpenBySerialNumber(const char *serial_number) {
  return OpenEx((PVOID)serial_number, FT_OPEN_BY_SERIAL_NUMBER);
}

bool D2XXDevice::OpenByDescription(const char *description) {
  return OpenEx((PVOID)description, FT_OPEN_BY_DESCRIPTION);
}

bool D2XXDevice::OpenByLocation(DWORD location) {
  return OpenEx((PVOID)(intptr_t)location, FT_OPEN_BY_LOCATION);
}

D2XXDevice::D2XXDevice(const FT_DEVICE_LIST_INFO_NODE *device_info) : handle(NULL), ft_status(FT_DEVICE_NOT_OPENED) {
  assert(device_info);
  memcpy(&info, device_info, sizeof(FT_DEVICE_LIST_INFO_NODE));
}

D2XXDevice::~D2XXDevice() {
  Close();
}

bool D2XXDevice::Open() {
  return OpenEx(info.SerialNumber, FT_OPEN_BY_SERIAL_NUMBER);
}

void D2XXDevice::Close(void) {
  if (handle) {
    if (FT_SUCCESS(ft_status = FT_Close(handle)))
      info.Flags &= ~FT_FLAGS_OPENED;
    handle = NULL;
    ft_status = FT_DEVICE_NOT_OPENED;
  }  
}

DWORD D2XXDevice::Read(void *buffer, DWORD count) {
  DWORD rw = 0;
  DMT_TRACE("\tReading %d bytes: ", count);
  if (FT_SUCCESS(ft_status = FT_Read(handle, buffer, count, &rw))) {
    DMT_TRACE(" (%d)\n", rw);
    return rw;
  }
  else {
    printf(" ERROR\n");
    return 0;
  } 
}

DWORD D2XXDevice::Write(void *buffer, DWORD count) {
  DWORD rw = 0;
  DMT_TRACE("\tWriting %d bytes: ", count);
  if (FT_SUCCESS(ft_status = FT_Write(handle, buffer, count, &rw))) {
    DMT_TRACE(" (%d)\n", rw);        
    return rw;
  }
  else {
    printf(" ERROR\n");
    return 0;
  }
}

bool 
D2XXDevice::IsOpen() {
  return (handle != NULL);
}

bool 
D2XXDevice::Initialize(void) {
  bool ret = true;
  ret &= SetTimeouts(FT_READ_TIMEOUT, FT_WRITE_TIMEOUT);
  ret &= SetBaudRate(FT_BAUD_RATE);
  ret &= SetDataCharacteristics(FT_WORD_LENGTH, FT_STOP_BITS_1, FT_PARITY_NONE);
  return ret;
}

bool 
D2XXDevice::SetTimeouts(DWORD read_timeout, DWORD write_timeout) {
  return (FT_SUCCESS(ft_status = FT_SetTimeouts(handle, read_timeout, write_timeout)));
}

bool D2XXDevice::SetBaudRate(DWORD baud_rate) {
  return (FT_SUCCESS(ft_status = FT_SetBaudRate(handle, baud_rate)));
}

bool D2XXDevice::SetDataCharacteristics(BYTE word_length, BYTE stop_bits, BYTE parity) {
  return (FT_SUCCESS(ft_status = FT_SetDataCharacteristics(handle, word_length, stop_bits, parity)));
}

bool D2XXDevice::GetQueueStatus(DWORD *rx_bytes, DWORD *tx_bytes, DWORD *event_status) {
  return FT_SUCCESS(ft_status = FT_GetStatus(handle, rx_bytes, tx_bytes, event_status));
}

#ifdef USE_FTCHIPID

bool D2XXDevice::GetUniqueChipID(DWORD *chip_id) {
  return FT_SUCCESS(ft_status = FTID_GetChipIDFromHandle(handle, chip_id));
}

#endif

DWORD D2XXDevice::FT_Status(void) {
  return ft_status;
}

DWORD D2XXDevice::GetType(void) {
  return info.Type;
}

DWORD D2XXDevice::GetVIDPID(void) {
  return info.ID;
}

DWORD D2XXDevice::GetLocationID(void) {
  return info.LocId;
}

const char *D2XXDevice::GetSerialNumber(void) {
  return info.SerialNumber;
}

const char *D2XXDevice::GetDescription(void) {
  return info.Description;
}

SID_TYPE D2XXDevice::GetSIDType(void) {
	if (strlen(info.Description) == 14) {
		return SID_TYPE_NONE;
	}
	else {
		if (strlen(info.Description) == 19) {
			if (strcmp(info.Description + 15, "6581") == 0) return SID_TYPE_6581;
			if (strcmp(info.Description + 15, "8580") == 0) return SID_TYPE_8580;
		}
		return SID_TYPE_NONE;
	}
}

int D2XXDevice::SetSIDType(DWORD index, SID_TYPE sidtype) {
	
	FT_PROGRAM_DATA mypdata;
	char ManufacturerBuf[32];
	char ManufacturerIdBuf[16];
	char DescriptionBuf[64];
	char SerialNumberBuf[16];
	
	mypdata.Signature1 = 0x00000000;
	mypdata.Signature2 = 0xffffffff;
	mypdata.Version = 0x00000002;

	mypdata.Manufacturer = ManufacturerBuf;
	mypdata.ManufacturerId = ManufacturerIdBuf;
	mypdata.Description = DescriptionBuf;
	mypdata.SerialNumber = SerialNumberBuf;

	if (!IsOpen()) {
		OpenByIndex(index);
	}
	ft_status = FT_EE_Read(handle,&mypdata);
	switch (sidtype)
	{
	case SID_TYPE_NONE:
		strcpy_s(DescriptionBuf, "SIDBlaster/USB");
		break;
	case SID_TYPE_6581:
		strcpy_s(DescriptionBuf, "SIDBlaster/USB/6581");
		break;
	case SID_TYPE_8580:
		strcpy_s(DescriptionBuf, "SIDBlaster/USB/8580");
		break;
	default:
		return 1;
	}

	ft_status = FT_EE_Program(handle, &mypdata);
	
	return ft_status;
}

int D2XXDevice::SetSerialNo(DWORD index, const char *serialNo) {

	FT_PROGRAM_DATA mypdata;
	char ManufacturerBuf[32];
	char ManufacturerIdBuf[16];
	char DescriptionBuf[64];
	char SerialNumberBuf[16];
        memset(SerialNumberBuf, '\0', 16);

	mypdata.Signature1 = 0x00000000;
	mypdata.Signature2 = 0xffffffff;
	mypdata.Version = 0x00000002;

	mypdata.Manufacturer = ManufacturerBuf;
	mypdata.ManufacturerId = ManufacturerIdBuf;
	mypdata.Description = DescriptionBuf;
	mypdata.SerialNumber = SerialNumberBuf;
	
	if (!IsOpen()) {
		OpenByIndex(index);
	}
	ft_status = FT_EE_Read(handle, &mypdata);

#ifdef WIN32
        strncpy_s(SerialNumberBuf, 16, serialNo, 8);
#elif defined linux || __APPLE__
        strncpy(SerialNumberBuf, serialNo, 8);
#endif

	ft_status = FT_EE_Program(handle, &mypdata);
	
	return ft_status;
}

void D2XXDevice::DisplayInfo(void) {
  const char *dev_type_str[] = {"232BM", "232AM", "100AX", "UNKNOWN", "2232C", "232R", "2232H", "4232H", "232H"};

  printf("%18s%s\n",      "FT Device type: ", dev_type_str[info.Type]);
  printf("%18s%s\n",      "Serial number: ",  info.SerialNumber);
  printf("%18s%s\n",      "Description: ",    info.Description);
  printf("%18s0x%08X\n",  "VID&PID: ",        info.ID);
  printf("%18s%d\n",      "Is opened: ",      (info.Flags & FT_FLAGS_OPENED));
  printf("%18s0x%p\n",    "Handle: ",         (void *)info.ftHandle);
  printf("%18s0x%08X\n",  "Location ID: ",    info.LocId);
}

DWORD D2XXDevice::Send(std::vector<unsigned char> const& data) {
  if (data.size()) {
    return Write((void*)&data[0], (DWORD)data.size());
  }
  return 0;
}

DWORD D2XXDevice::Send(unsigned char const* data, size_t size) {
  if (size) {
    return Write((void*)data, (DWORD)size);
  }
  return 0;
}

DWORD D2XXDevice::Recv(std::vector<unsigned char>& data) {
  DWORD
    rx_bytes = 0,
    tx_bytes = 0,
    event_status = 0;

  if (GetQueueStatus(&rx_bytes, &tx_bytes, &event_status) && rx_bytes) {
    data.resize(rx_bytes);
    return Read((void*)&data.at(0), rx_bytes);
  }
  return 0;
}

}
