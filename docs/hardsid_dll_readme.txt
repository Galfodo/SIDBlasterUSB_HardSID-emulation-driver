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

// DLLs version 0x0301 and above
BYTE WINAPI HardSID_Try_Write  (BYTE deviceID, int cycles, BYTE SID_reg, BYTE data);
// Non-blocking HardSID_Write - returns HSID_USB_WSTATE_BUSY if the
// internal command buffer is full instead of blocking, OK once queued.
// Callers must retry with a bounded timeout, not spin forever - see
// docs/integrators_guide.md's "HardSID_Try_Write and the BUSY retry
// loop" section for the full pattern and why yield()-vs-sleep() matters.

// DLLs above version 0x206
// Support whether the channel change was a request from the user or the program
// (auto or manual respectively). External mixers can use this to prioritise requests
void WINAPI HardSID_Mute2      (BYTE deviceID, BYTE channel, BOOL mute, BOOL manual);

// DLLs above version 0x207
// Enable support for non hardsid hardware (e.g. Catweasel MK3/4)
void WINAPI HardSID_OtherHardware (void);

// NOTE: HardSID_Group, HardSID_Mute2, HardSID_OtherHardware, SetDebug,
// GetDLLVersion, MuteHardSID and MuteHardSIDAll above are part of the
// general historical HardSID DLL spec this file was originally
// documenting, but are NOT exported by this driver (not present in
// hardsid.def) - don't rely on them being there.

// SIDBlaster-specific additions actually exported by this driver
// (see hardsid.def for the full, authoritative export list; hardsid.hpp
// for exact signatures). Not part of the original HardSID spec above.

// 0x0202
void WINAPI HardSID_GetSerial  (char* output, int bufferSize, BYTE deviceID);
// Reads the device's 8-character serial number.

// 0x0203
void WINAPI HardSID_SoftFlush  (BYTE deviceID);
// Like HardSID_Flush, but sent as its own dispatcher command
// (CommandParams::SoftFlush) rather than HardSID_Flush's Flush command -
// see ThreadDispatcher::SendCommand's special-cased handling of Flush
// (it drains the command queue synchronously) vs. how SoftFlush is just
// queued and dispatched normally.

void WINAPI HardSID_SetWriteBufferSize (BYTE bufferSize);
// The function-call equivalent of the SIDBLASTERUSB_WRITEBUFFER_SIZE
// environment variable above - sets the same USB write-batching buffer
// size at runtime instead of via env var. Whichever runs last wins if
// you use both; there's no conflict, just pick one.

int WINAPI HardSID_SetSIDType  (BYTE deviceID, int sidtype);
// sidtype: 0 = none, 1 = 6581, 2 = 8580. Returns 0 on success.
// WARNING: writes to the device's FTDI EEPROM (via its description
// string), not a runtime setting - the host process must exit and the
// SIDBlaster must be unplugged/replugged (or the OS driver reloaded)
// before the new type is recognized. Not something to call during
// normal playback; see SIDBlasterTool for the intended interactive use.

int WINAPI HardSID_GetSIDType  (BYTE deviceID);
// Reads back the SID type last written with HardSID_SetSIDType.

int WINAPI HardSID_SetSerial   (BYTE deviceID, const char *serialNo);
// Writes an 8-character (A-Z, 0-9) serial number to the device's FTDI
// EEPROM. Same reconnect-required caveat as HardSID_SetSIDType above.

boolean WINAPI HardSID_ExternalTiming (BYTE deviceID);
// Returns whether the dispatcher runs asynchronously on its own thread
// (ThreadDispatcher::IsAsync(), always true for this driver's normal
// build) vs. synchronously on the caller's thread (SyncDispatcher).

void WINAPI HardSID_Uninitialize (void);
// POSIX only - a no-op on Windows (DllMain's DLL_PROCESS_DETACH already
// tears the dispatcher down there). On Linux/macOS there is no such
// hook, so host software MUST call this itself before process exit or
// before dlclose()-ing the library, or the background dispatcher thread
// can be left running/torn out from under - see docs/integrators_guide.md.

// DLLs from version 0x0204 (this fork's bug-fix cleanup pass, see the
// fork's README section) - no new exports, and no change to whether
// host software needs "keep-alive" writes while idle. That pattern
// exists for a different reason than the busy-spin fix (HardSID_Write's
// cycles argument is a small fixed value per call, so playback timing
// accuracy comes from call frequency, not driver dispatch latency) -
// keep it regardless of driver version. Investigated directly; do not
// remove based on this version number.
