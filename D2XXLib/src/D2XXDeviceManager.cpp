
/* Adapted from https://github.com/ssidko/DMT/blob/master/D2xx.h */

#include "D2XXLib/D2XXDevice.h"
#include "D2XXLib/D2XXDeviceManager.h"
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <algorithm>

namespace D2XXLib
{
  bool D2XXManager::IsValidDeviceInfo(FT_DEVICE_LIST_INFO_NODE *dev_info)
  {
    assert (dev_info);

    //Check if the FTDI is a real Sidblaster
    if (strcmp(dev_info->Description, "SIDBlaster/USB") == 0) {
      return true;
    }
    else {
      return false;
    }
  }
  
  // Sort device list by seriaL number
  bool sortBySerial(D2XXDevice * lhs, D2XXDevice * rhs)
  {	
	  return (strcmp(lhs->GetSerialNumber(), rhs->GetSerialNumber()) < 0);
  }
  
  DWORD D2XXManager::CreateDeviceList(D2XXDevicesList *list)
  {
    DWORD device_count = 0;
    D2XXDevice *device = NULL;
    FT_DEVICE_LIST_INFO_NODE *ft_info = NULL;
    FT_DEVICE_LIST_INFO_NODE *ft_info_list = NULL;
		
    CleanList(list);
    if (FT_SUCCESS(ft_status = FT_CreateDeviceInfoList(&device_count))) {
      if (device_count) {
        ft_info_list = new FT_DEVICE_LIST_INFO_NODE[device_count*sizeof(FT_DEVICE_LIST_INFO_NODE)];
        memset(ft_info_list, 0x00, device_count*sizeof(FT_DEVICE_LIST_INFO_NODE));
        if (FT_SUCCESS(ft_status = FT_GetDeviceInfoList(ft_info_list, &device_count))) {
          ft_info = ft_info_list;
          for (DWORD i = 0; i < device_count; i++, ft_info++) {
            if (IsValidDeviceInfo(ft_info)) {
              device = new D2XXDevice(ft_info);
              list->push_back(device);
            }
		  }
		  std::sort(list->begin(),list->end(),sortBySerial);
        }
        delete[] ft_info_list;
      }
    }
    return (DWORD)list->size();
  }

  void CleanList(D2XXDevicesList *list)
  {
    D2XXDevicesList::iterator it;
    for (it = list->begin(); it != list->end(); it++)
      delete *it;
    list->clear();
  }

  D2XXManager::D2XXManager() : dev_count(0), ft_status(FT_OK)
  {
  }

  D2XXManager::~D2XXManager()
  {
    CleanList(&dev_list);
  }

  DWORD D2XXManager::Count(void)
  {
    return dev_count;
  }

  DWORD D2XXManager::Rescan(void)
  {
    bool find = false;
    D2XXDevicesList::iterator it;
    D2XXDevicesList::iterator tmp_it;
    D2XXDevicesList tmp_list;
    DWORD tmp_count = CreateDeviceList(&tmp_list);
    for (tmp_it = tmp_list.begin(); tmp_it != tmp_list.end(); tmp_it++) {
      for (it = dev_list.begin(); it != dev_list.end(); it++) {
        if (!strcmp((*tmp_it)->GetSerialNumber(), (*it)->GetSerialNumber())) {
          find = true;
          break;  
        }          
      }
      if (!find) {
        dev_list.push_back(*tmp_it);
        Notify(*tmp_it, kPlugged);
      }
      find = false;
    }
    for (it = dev_list.begin(); it != dev_list.end(); it++) {
      for (tmp_it = tmp_list.begin(); tmp_it != tmp_list.end(); tmp_it++) {
        if (!strcmp((*it)->GetSerialNumber(), (*tmp_it)->GetSerialNumber())) {
          find = true;
          break;
        }
      }
      if (!find) {
        Notify(*it, kUnplugged);
        delete *it;
        it = dev_list.erase(it);
        if (it == dev_list.end()) break;
      }
      find = false;
    }
    return dev_count = (DWORD)dev_list.size();
  }

  D2XXDevice *D2XXManager::GetDevice(DWORD index)
  {
    return dev_list.at(index);
  }

  DWORD D2XXManager::FT_Status(void)
  {
    return ft_status;
  }

  void D2XXManager::Update(Subject *subject)
  {
    Rescan();
  }

  void 
  D2XXManager::CleanList(D2XXDevicesList *list) {
    assert(list);
    for (size_t i = 0; i < list->size(); ++i) {
      delete list->at(i);
    }
    list->clear();
  }

  void D2XXManager::DisplayDevicesInfo(void)
  {
    printf("===================================\n");
    printf("Devices: %d\n", dev_count);
    printf("===================================\n");
    for (DWORD i = 0; i < dev_count; i++) {
      dev_list.at(i)->DisplayInfo();
      printf("===================================\n");
    }
  }

}
