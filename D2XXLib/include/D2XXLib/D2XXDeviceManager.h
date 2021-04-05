#ifndef D2XXDEVICEMANAGER_H_INCLUDED
#define D2XXDEVICEMANAGER_H_INCLUDED

/* Adapted from https://github.com/ssidko/DMT/blob/master/D2xx.h */

#include "Utilities.h"
#include <vector>

namespace D2XXLib 
{
  typedef std::vector<D2XXDevice *> D2XXDevicesList;

  class D2XXManager : public ManagerNotifier, public Observer
  {
  private:
	static D2XXManager* instance;
	D2XXManager();
	D2XXManager(const D2XXManager&) {}
	~D2XXManager();
    DWORD dev_count;
    FT_STATUS ft_status;
    D2XXDevicesList dev_list;

    bool IsValidDeviceInfo(FT_DEVICE_LIST_INFO_NODE *dev_info);
    DWORD CreateDeviceList(D2XXDevicesList *list);
    void CleanList(D2XXDevicesList *list);
  public:
	static D2XXManager* GetInstance();
	static void Destroy();
    DWORD Count(void);
    DWORD Rescan(void);
    D2XXDevice *GetDevice(DWORD index);
    DWORD FT_Status(void);
    void DisplayDevicesInfo(void);
    virtual void Update(Subject *subject);
	const char* GetSerialNo(DWORD index);
	SID_TYPE GetSIDType(DWORD index);
	int SetSIDType(DWORD index, SID_TYPE sidtype);
	int SetSerialNo(DWORD index, const char *serialNo);
  };

}

#endif
