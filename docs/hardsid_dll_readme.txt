This driver is not official and any problems resulting from its use
should be directed to sidplay2@yahoo.com and not Hard Software.
All Windows platforms are supported with this single driver.

To install the driver either copy it to the same directory as the
application wanting to use it, or place it in a system folder
(e.g. c:\windows\system) after having previously installed the
appropriate official hardsid drivers.  You must run
HardSIDConfig.exe before this driver will work.


Support for the following features are provided:

Official library interface
Mute extensions
Cycle exact extensions
SID Groups extensions (not fully documented)
Multiple cards (at any of the valid addresses)
Mixed card types (HardSID/Quattro/ISA/PCI)
HardSIDConfig
HardSIDSoundConfig
Real/Faked Reads


The following interfaces are supported:

// Cycle exact interface
void WINAPI HardSID_Delay      (BYTE deviceID, WORD cycles);
BYTE WINAPI HardSID_Devices    (void);
void WINAPI HardSID_Filter     (BYTE deviceID, BOOL filter);
// Empties the devices FIFO without playing it (if supported)
void WINAPI HardSID_Flush      (BYTE deviceID);
void WINAPI HardSID_Mute       (BYTE deviceID, BYTE channel, BOOL mute);
void WINAPI HardSID_MuteAll    (BYTE deviceID, BOOL mute);
void WINAPI HardSID_Reset      (BYTE deviceID);
BYTE WINAPI HardSID_Read       (BYTE deviceID, WORD cycles, BYTE SID_reg);
// Empties the devices FIFO by playing it (if supported)
void WINAPI HardSID_Sync       (BYTE deviceID);
void WINAPI HardSID_Write      (BYTE deviceID, WORD cycles, BYTE SID_reg, BYTE data);
// Internal version number for this DLL e.g. 0x0202 (2.2)
WORD WINAPI HardSID_Version    (void);

// Official HardSID interface (depreciated)
BYTE WINAPI GetHardSIDCount    (void);
void WINAPI InitHardSID_Mapper (void);
void WINAPI MuteHardSID_Line   (BOOL mute);
BYTE WINAPI ReadFromHardSID    (BYTE deviceID, BYTE SID_reg);
void WINAPI SetDebug           (BOOL enabled);
void WINAPI WriteToHardSID     (BYTE deviceID, BYTE SID_reg, BYTE data);

// Mute Extensions (depreciated)
WORD WINAPI GetDLLVersion      (void);
void WINAPI MuteHardSID        (BYTE deviceID, BYTE channel, BOOL mute);
void WINAPI MuteHardSIDAll     (BYTE deviceID, BOOL mute);

// DLLs above version 0x0203
void WINAPI HardSID_Reset2     (BYTE deviceID, BYTE volume); // Click reduction
BOOL WINAPI HardSID_Lock       (BYTE deviceID); // Lock SID to application
void WINAPI HardSID_Unlock     (BYTE deviceID);
// Add SID to group when enable is true.  SID can only be added or moved
// to an existing group.  If deviceID = groupID then a new group is created
// with the SID device becoming group master.  Only writes to the master are
// played on the other grouped SIDs.
BOOL WINAPI HardSID_Group      (BYTE deviceID, BOOL enable, BYTE groupID);

// DLLs above version 0x206
// Support whether the channel change was a request from the user or the program
// (auto or manual respectively). External mixers can use this to prioritise requests
void WINAPI HardSID_Mute2      (BYTE deviceID, BYTE channel, BOOL mute, BOOL manual);

// DLLs above version 0x207
// Enable support for non hardsid hardware (e.g. Catweasel MK3/4)
void WINAPI HardSID_OtherHardware (void);
