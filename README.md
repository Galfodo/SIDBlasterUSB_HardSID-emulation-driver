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

**Linux/macOS CMake, tightened but not test-built here** (no toolchain
in this environment - written to the best of my knowledge, real
verification happens on Andreas's machines): all the fixes above are
plain C++11 standard library (`std::mutex`/`condition_variable`/
`chrono`/`atomic`), nothing Windows-specific, so they carry over as-is -
this was purely a CMake pass. Added `install(TARGETS hardsid LIBRARY
DESTINATION /usr/local/lib)` (matches where AIASS/ASID Player's
Linux/Mac paths already expect to find it - see their `Sid.cpp`) and
`CMAKE_OSX_ARCHITECTURES "x86_64;arm64"` for a universal macOS binary
(CMake raises the arm64 slice's effective minimum OS version
automatically - `CMAKE_OSX_DEPLOYMENT_TARGET` above only constrains the
x86_64 slice). `FindFTD2XX.cmake` and the D2XXLib linux/`__APPLE__`
branches were already in place from Ken's original port - untouched.

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
