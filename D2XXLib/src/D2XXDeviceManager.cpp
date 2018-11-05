
/* Adapted from https://github.com/ssidko/DMT/blob/master/D2xx.h */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#error "unsupported platform"
#endif

#include "D2XXLib/D2XXDevice.h"
#include "D2XXLib/D2XXDeviceManager.h"
#include <stdio.h>
#include <vector>
#include <assert.h>

#define D2XX_MANAGER_RESCAN_TIMEOUT          (DWORD)15000
#define D2XX_MANAGER_RESCAN_SLEEP_TIMEOUT      (DWORD)500

namespace D2XXLib
{
  BOOL D2XXManager::IsValidDeiviceInfo(FT_DEVICE_LIST_INFO_NODE *dev_info)
  {
    assert (dev_info);
    if (strlen(dev_info->SerialNumber)) {
                  return TRUE;
      //if (dev_info->Flags & FT_FLAGS_OPENED)
      //  return TRUE;
      //else if (dev_info->LocId)
      //  return TRUE;
    }
    return FALSE;
  }

  DWORD D2XXManager::CreateDeviceList(D2XXDevicesList *list)
  {
    BOOL all_dev_valid = TRUE;
    DWORD deice_count = 0;
    D2XXDevice *device = NULL;
    FT_DEVICE_LIST_INFO_NODE *ft_info = NULL;
    FT_DEVICE_LIST_INFO_NODE *ft_info_list = NULL;
    DWORD tick_count = ::GetTickCount();

    do {
      CleanList(list);
      all_dev_valid = TRUE;
      if (FT_SUCCESS(ft_status = FT_CreateDeviceInfoList(&deice_count))) {
        if (deice_count) {
          ft_info_list = new FT_DEVICE_LIST_INFO_NODE[deice_count*sizeof(FT_DEVICE_LIST_INFO_NODE)];
          memset(ft_info_list, 0x00, deice_count*sizeof(FT_DEVICE_LIST_INFO_NODE));
          if (FT_SUCCESS(ft_status = FT_GetDeviceInfoList(ft_info_list, &deice_count))) {
            ft_info = ft_info_list;
            for (DWORD i = 0; i < deice_count; i++, ft_info++) {
              if (IsValidDeiviceInfo(ft_info)) {
                device = new D2XXDevice(ft_info);
                list->push_back(device);
              }
              else all_dev_valid = FALSE;
            }
          }
          delete[] ft_info_list;
        }
      }
      if (!all_dev_valid) ::Sleep(D2XX_MANAGER_RESCAN_SLEEP_TIMEOUT);
    } while (!(all_dev_valid || ((::GetTickCount() - tick_count) > D2XX_MANAGER_RESCAN_TIMEOUT)));
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
    BOOL find = FALSE;
    D2XXDevicesList::iterator it;
    D2XXDevicesList::iterator tmp_it;
    D2XXDevicesList tmp_list;
    DWORD tmp_count = CreateDeviceList(&tmp_list);
    for (tmp_it = tmp_list.begin(); tmp_it != tmp_list.end(); tmp_it++) {
      for (it = dev_list.begin(); it != dev_list.end(); it++) {
        if (!strcmp((*tmp_it)->GetSerialNumber(), (*it)->GetSerialNumber())) {
          find = TRUE;
          break;  
        }          
      }
      if (!find) {
        dev_list.push_back(*tmp_it);
        Notify(*tmp_it, kPlugged);
      }
      find = FALSE;
    }
    for (it = dev_list.begin(); it != dev_list.end(); it++) {
      for (tmp_it = tmp_list.begin(); tmp_it != tmp_list.end(); tmp_it++) {
        if (!strcmp((*it)->GetSerialNumber(), (*tmp_it)->GetSerialNumber())) {
          find = TRUE;
          break;
        }
      }
      if (!find) {
        Notify(*it, kUnplugged);
        delete *it;
        it = dev_list.erase(it);
        if (it == dev_list.end()) break;
      }
      find = FALSE;
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
