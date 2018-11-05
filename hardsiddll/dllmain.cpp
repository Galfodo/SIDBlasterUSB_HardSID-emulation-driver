// dllmain.cpp : Defines the entry point for the DLL application.

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include "ThreadDispatcher.h"
#include "SyncDispatcher.h"
#include "DriverDefs.h"

#include <mmsystem.h>

SIDBlaster::CommandDispatcher
  *g_CommandDispatcher;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
           )
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    timeBeginPeriod(1);
#ifdef USE_ASYNC_INTERFACE
    g_CommandDispatcher = new SIDBlaster::ThreadDispatcher();
#else
    g_CommandDispatcher = new SIDBlaster::SyncDispatcher();
#endif
    g_CommandDispatcher->Initialize();
    break;

  case DLL_PROCESS_DETACH:
    g_CommandDispatcher->Uninitialize();
    timeEndPeriod(1);
    break;

  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}

