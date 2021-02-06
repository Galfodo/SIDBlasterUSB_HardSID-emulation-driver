/**
 * Implements hardsid.dll api calls
 * Written by stein.pedersen@hue.no
 */
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <thread>
#include "CommandDispatcher.h"
#include "..\D2XXLib\include\D2XXLib\D2XXDevice.h"
#include "..\D2XXLib\include\D2XXLib\D2XXDeviceManager.h"

using namespace SIDBlaster;

#ifdef _MSC_VER
#define DLLEXPORT __stdcall // We're using a .def file to create undecorated names with __stdcall calling convention
#else
#define DLLEXPORT __declspec(dllexport) __stdcall
#endif

#define HARDSID_VERSION      0x0203

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned char boolean;

D2XXLib::D2XXManager* x_Manager = D2XXLib::D2XXManager::getInstance();

// Command despatcher is defined in a different file, depending on mode of operation: standalone dll or winhost
extern SIDBlaster::CommandDispatcher
  *g_CommandDispatcher;

extern "C" {

	Uint8 DLLEXPORT HardSID_Read(Uint8 DeviceID, int Cycles, Uint8 SID_reg);

	Uint16 DLLEXPORT HardSID_Version(void) {
		return HARDSID_VERSION;
	}

	Uint8 DLLEXPORT HardSID_Devices(void) {
		return g_CommandDispatcher->DeviceCount();
	}

	void DLLEXPORT HardSID_Delay(Uint8 DeviceID, Uint16 Cycles) {
		CommandParams cmd(DeviceID, CommandParams::Delay, 0, 0, Cycles, false);
		g_CommandDispatcher->SendCommand(cmd);
	}

	void DLLEXPORT HardSID_Write(Uint8 DeviceID, int Cycles, Uint8 SID_reg, Uint8 Data) {
		CommandParams cmd(DeviceID, CommandParams::Write, SID_reg, Data, Cycles, false);
		while (g_CommandDispatcher->SendCommand(cmd) != 0) {
			std::this_thread::yield();
		}
		//// TEST CODE, REMOVE!
		//if (Cycles % 20000 < 10) {
		//  Uint8 data = HardSID_Read(DeviceID, 0, 0x1b);
		//  if (1 || data) {
		//    printf("%02X\n", (int)data);
		//  }
		//}
	}

	// Blocking ...
	Uint8 DLLEXPORT HardSID_Read(Uint8 DeviceID, int Cycles, Uint8 SID_reg) {
		CommandParams cmd(DeviceID, CommandParams::Read, SID_reg, 0, Cycles);
		int result = g_CommandDispatcher->SendCommand(cmd);
		if (result) {
			return (result >> 8);
		}
		else {
			return 0;
		}
	}

	void DLLEXPORT HardSID_Flush(Uint8 DeviceID) {
		CommandParams cmd(DeviceID, CommandParams::Flush);
		g_CommandDispatcher->SendCommand(cmd);
	}

	void DLLEXPORT HardSID_SoftFlush(Uint8 DeviceID) {
		CommandParams cmd(DeviceID, CommandParams::SoftFlush);
		g_CommandDispatcher->SendCommand(cmd);
	}

	boolean DLLEXPORT HardSID_Lock(Uint8 DeviceID) {
		CommandParams cmd(DeviceID, CommandParams::Lock);
		g_CommandDispatcher->SendCommand(cmd);
		return true;
	}

	void DLLEXPORT HardSID_Filter(Uint8 DeviceID, boolean Filter) {
		CommandParams cmd(DeviceID, CommandParams::Filter, 0, Filter);
		g_CommandDispatcher->SendCommand(cmd);
	}

	void DLLEXPORT HardSID_Reset(Uint8 DeviceID) {
		CommandParams cmd(DeviceID, CommandParams::Reset);
		g_CommandDispatcher->SendCommand(cmd);
	}

	void DLLEXPORT HardSID_Sync(Uint8 DeviceID) {
		CommandParams cmd(DeviceID, CommandParams::Sync);
		g_CommandDispatcher->SendCommand(cmd);
	}

	void DLLEXPORT HardSID_Mute(Uint8 DeviceID, Uint8 Channel, boolean Mute) {
		if (Mute) {
			CommandParams cmd(DeviceID, CommandParams::MuteAll, 0, Channel);
			g_CommandDispatcher->SendCommand(cmd);
		}
	}

	void DLLEXPORT HardSID_MuteAll(Uint8 DeviceID, boolean Mute) {
		if (Mute) {
			CommandParams cmd(DeviceID, CommandParams::MuteAll);
			g_CommandDispatcher->SendCommand(cmd);
		}
	}

	void DLLEXPORT InitHardSID_Mapper(void) {
	}

	Uint8 DLLEXPORT GetHardSIDCount(void) {
		return HardSID_Devices();
	}

	void DLLEXPORT WriteToHardSID(Uint8 DeviceID, Uint8 SID_reg, Uint8 Data) {
		HardSID_Write(DeviceID, 0, SID_reg, Data);
	}

	Uint8 DLLEXPORT ReadFromHardSID(Uint8 DeviceID, Uint8 SID_reg) {
		CommandParams cmd(DeviceID, CommandParams::Read, SID_reg);
		return g_CommandDispatcher->SendCommand(cmd);
	}

	void DLLEXPORT MuteHardSID_Line(int Mute) {
	}

	// DLLs above version 0x0203
	void DLLEXPORT HardSID_Reset2(Uint8 DeviceID, Uint8 Volume) {
	}

	// Unlock the current device to be no more used by the client
	void DLLEXPORT HardSID_Unlock(Uint8 DeviceID) {
		CommandParams cmd(DeviceID, CommandParams::Unlock);
		g_CommandDispatcher->SendCommand(cmd);
	}

	// DLLs version 0x0301 and above

	/*
	Documentation of HardSID_Try_Write:

	If the hardware buffer is full then HardSID_Try_Write should return HSID_USB_WSTATE_BUSY. Acid64 then keeps calling this method in a loop (with a certain delay of course) until HardSID_Try_Write returns any other state. You should return HSID_USB_WSTATE_OK when the buffer is send successfully to the hardware.

	The following states it can return:

	typedef enum {HSID_USB_WSTATE_OK = 1, HSID_USB_WSTATE_BUSY,
				  HSID_USB_WSTATE_ERROR, HSID_USB_WSTATE_END}

	Also make sure in your implementation to kill the hardware buffer when HardSID_Flush is called.
	*/

	enum {
		HSID_USB_WSTATE_OK = 1, HSID_USB_WSTATE_BUSY,
		HSID_USB_WSTATE_ERROR, HSID_USB_WSTATE_END
	};

	BYTE DLLEXPORT HardSID_Try_Write(Uint8 DeviceID, int Cycles, Uint8 SID_reg, Uint8 Data) {
		CommandParams cmd(DeviceID, CommandParams::Write, SID_reg, Data, Cycles, true);
		if (g_CommandDispatcher->SendCommand(cmd) == 0) {
			return HSID_USB_WSTATE_OK;
		}
		else {
			return HSID_USB_WSTATE_BUSY;
		}
	}

	BOOL DLLEXPORT HardSID_ExternalTiming(Uint8 DeviceID) {
		return g_CommandDispatcher->IsAsync();
	}

	// ***********************************************0x202****************************************************
	void DLLEXPORT HardSID_GetSerial(Uint8 DeviceID, char* output) {
		strncpy_s(output, 9, (char*)x_Manager->GetSerialNo(DeviceID), 8);
	}
	
	// ***********************************************0x203****************************************************
	
	void DLLEXPORT HardSID_SetWriteBufferSize(Uint8 bufferSize) {
		g_CommandDispatcher->setWriteBufferSize(bufferSize);
	}
	
	// sidtype:
	// 0 none
	// 1 6581
	// 2 8580
	// this function is "death end", host program must terminate after call and the sidblaster must reconnect
	int DLLEXPORT HardSID_SetSIDInfo(Uint8 DeviceID, int sidtype) { //returns success
		return x_Manager->SetSIDInfo(DeviceID, sidtype);
	}

	int DLLEXPORT HardSID_GetSIDInfo(Uint8 DeviceID) { //returns sidtype
		return x_Manager->GetSIDInfo(DeviceID);
	}


}