This fork (Andreas Schumm / crazy-midi.de)
-------------------------------------------

Bug-fix-only cleanup pass over the original driver below - no new
features, no protocol changes:

* `ThreadReceiver`/`ThreadDispatcher`: replaced a CPU-burning busy-spin
  wait with a proper condition-variable wait (`wait_for` + predicate, not
  a bare `wait()` - avoids a lost-wakeup hang if a notify races ahead of
  the consumer reaching the wait); made the shared command/read state
  atomic.
* `ThreadDispatcher::SendCommand`: fixed a second, related bug this
  surfaced once the dispatcher actually slept instead of spinning -
  `notify_all()` was being skipped for any command marked
  `m_IsBuffered` (which is every write `HardSID_Try_Write` sends, see
  `hardsiddll.cpp`). With the busy-spin gone, that meant every batch of
  buffered writes wasn't dispatched until the 2ms fallback poll - at
  SID-register timing precision, a consistent 2ms-per-batch stall is
  audible stuttering. Now notifies unconditionally; `m_IsBuffered` still
  controls USB-transfer batching (`CommandReceiver::ExecuteCommand`'s
  `BufferWrite` vs `Write`), just not whether the dispatcher gets woken.
* `SyncDispatcher`: added the missing `SetWriteBufferSize` override and
  a full command switch.
* `CommandDispatcher`: fixed the `Filter` enum aliasing with `MuteLine`
  (moved `Filter` to the end of the enum).
* `CommandReceiver`: removed dead adaptive-buffer code, made the
  read-result state atomic.
* `SIDBlasterInterfaceImpl`: added a 1000ms timeout to the USB read spin
  loop (previously could hang forever on a wedged device).

`HardSID_Version()` bumped `0x0203` -> `0x0204` to mark these fixes.
Note this does NOT mean client-side "keep-alive write" patterns (e.g.
`SIDWriteThread`'s do-nothing writes when idle, see the AIASS/ASID
Player repos) can be dropped - investigated that directly, and they're
load-bearing for a different reason: `HardSID_Write`'s `cycles` argument
is usually called with a small fixed value per call, so real-time
playback accuracy comes from calling the driver often enough, not from
the driver's own dispatch latency. That's unrelated to this fix and
still needed regardless of driver version.

Tested against real hardware with AIASS and SIDBlaster-ASID-Player -
confirmed clean playback, no stuttering, no audible artifacts (better
than the pre-fix baseline, if anything - immediate `notify_all()` wakes
the dispatcher through a real kernel wait/signal path, versus however
often a free-running spin loop happened to check next). Likely the last
release of this fork - active development has moved to SIDBlaster NG, a
from-scratch firmware + driver stack built on the exSID protocol. This
driver stays as-is for existing SIDBlaster-USB hardware that isn't
reflashed to NG.

**Linux/macOS CMake, built and verified on real hardware.** All the
fixes above are plain C++11 standard library (`std::mutex`/
`condition_variable`/`chrono`/`atomic`), nothing Windows-specific, so
they carry over as-is - this was originally a CMake-only pass, since
verified by actually building it: `install(TARGETS hardsid LIBRARY
DESTINATION /usr/local/lib)` (matches where AIASS/ASID Player's
Linux/Mac paths already expect to find it - see their `Sid.cpp`) and
`CMAKE_OSX_ARCHITECTURES "x86_64;arm64"` for a universal macOS binary
(CMake raises the arm64 slice's effective minimum OS version
automatically - `CMAKE_OSX_DEPLOYMENT_TARGET` above only constrains the
x86_64 slice). `FindFTD2XX.cmake` and the D2XXLib linux/`__APPLE__`
branches were already in place from Ken's original port - untouched.
Built cleanly on Ubuntu 24.04 and macOS 12.7 (`libhardsid.so` /
universal `libhardsid.dylib`, x86_64+arm64), and played real SID tunes
on real SIDBlaster-USB hardware on both, passed through the same
physical device one VM at a time - confirmed by ear equal to Windows,
no quality regression. The arm64 slice of the macOS build compiles
cleanly but hasn't been runtime-tested on actual Apple Silicon yet
(only from an Intel VM so far).

**Downloads: see the [Releases](https://github.com/gh0stless/SIDBlasterUSB_HardSID-emulation-driver/releases) page.**
GitHub Releases ship plain binaries carrying the fixes above - no
installer, just the library plus what it needs:

* **Windows** - `hardsid-x64.dll` / `hardsid-x86.dll`. Drop the matching
  one next to your player's .exe, or into `System32` (64-bit hosts) /
  `SysWOW64` (32-bit hosts, e.g. SIDPlay2, older ACID64/VICE builds) for
  system-wide use. The old driver program that came with the device
  (`SIDBlaster.exe`) must still be disabled for this to work.
* **Linux** - `libhardsid.so` + bundled FTDI D2XX runtime. Copy to
  `/usr/local/lib`, `ldconfig`, and add a udev rule detaching the
  kernel's `ftdi_sio`/`usbserial` VCP driver on attach - without that
  rule the device shows up as a serial port instead, and D2XX/libhardsid
  can't open it. That's the D2XX-vs-VCP conflict that makes
  SIDBlaster-USB "a bit tricky" on Linux.
* **macOS** - universal (Intel + Apple Silicon) `libhardsid.dylib` +
  bundled FTDI D2XX runtime. Copy to `/usr/local/lib`.

Prefer a proper installer that handles all of the above for you (System32/
SysWOW64 placement on Windows, the udev rule on Linux, the D2XX symlink
on macOS)? Get one from **[crazy-midi.de](https://crazy-midi.de)**.

**Prerequisite: the FTDI D2XX driver itself.** `hardsid.dll`/`libhardsid`
talks to the SIDBlaster-USB through FTDI's D2XX library, which is a
separate install from everything above and the actual "tricky" part on
Linux/macOS (documented in more detail in the
[AIASS-Uno-VST manual](https://github.com/gh0stless/AIASS-Uno-VST)):

* **Windows** - usually arrives via Windows Update (Settings / Windows
  Update / Optional Updates) the first time the device is plugged in.
  That handshake needs an active internet connection and fails under a
  "Metered Connection" - temporarily disable that if the device isn't
  being recognized. Older Windows versions may need the driver from
  [ftdichip.com/drivers/d2xx-drivers](https://ftdichip.com/drivers/d2xx-drivers/).
* **Linux** - download and install from
  [ftdichip.com/drivers/d2xx-drivers](https://ftdichip.com/drivers/d2xx-drivers/)
  per FTDI's
  [AN_220 installation guide](https://www.ftdichip.com/Support/Documents/AppNotes/AN_220_FTDI_Drivers_Installation_Guide_for_Linux.pdf).
  If the device still isn't usable afterwards, that's the D2XX-vs-VCP
  conflict mentioned above - the udev rule fixes it.
* **macOS** - download and install from the same
  [ftdichip.com/drivers/d2xx-drivers](https://ftdichip.com/drivers/d2xx-drivers/)
  page per FTDI's
  [AN_134 installation guide](https://ftdichip.com/wp-content/uploads/2020/08/AN_134_FTDI_Drivers_Installation_Guide_for_MAC_OSX-1.pdf),
  and also install D2XXHelper from the same page. Gatekeeper may then
  refuse to run unsigned/non-notarized binaries built against it - if
  so, either `sudo xattr -rd com.apple.quarantine /path/to/your.app`
  (removes the quarantine flag from just that app) or, as a blunter
  last resort, `sudo spctl --master-disable` (turns off Gatekeeper's
  developer-verification check entirely).

Writing host software against this DLL? See
[`docs/integrators_guide.md`](docs/integrators_guide.md) - covers the
POSIX `HardSID_Uninitialize()` requirement, the `Cycles`-is-a-relative-
delta timing model, and a few other things that are easy to get wrong
without knowing to look for them.

Thanks to Stein Pedersen for the original driver below, and to kenchis
for the Linux/macOS port (PR #5, merge `1334643`).

---

SIDBlasterUSB HardSID emulation driver BETA (Windows)
-----------------------------------------------------

The driver is a drop-in replacement for hardsid.dll and should be compatible
with most programs that support HardSID playback. The old driver program that
came with the device (SIDBlaster.exe) must be disabled for the driver to work
properly.

The driver has been tested with ACID64Player Pro v3.6, Sidplay2, Sidplay26/W and VICE2.4

Depending on your system, tunes with high data rates (multispeed tunes or digitunes) may 
play slower if the latency of the USB driver is too high. This can be remedied by setting
the driver write buffer size to a larger value, for instance:

SET SIDBLASTERUSB_WRITEBUFFER_SIZE=8

A larger write buffer may adversely effect the sound quality in tunes that need cycle exact 
emulation since they will be batched together.

I have created a test archive with some players and tunes, some normal speed, some multispeed,
and a couple of digi-tunes. This can be downloaded from csdb.

I have implemented multi-device support, but since I only have a single SIDBlaster, this has
not been properly tested.

I welcome feedback on all aspects of the driver. Other programs, multi-device etc.

Thanks to:

* Wilfred Bos for his assistance in developing this driver and also for extending his
already awesome ACID64Player with SIDBlasterUSB support

* Isildur for testing

* Davey for creating this awesome device.

Oslo, 13.02.2015

stein.pedersen@gmail.com
