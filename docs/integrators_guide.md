# Integrating hardsid.dll / libhardsid.so / libhardsid.dylib

Notes for anyone writing host software against this driver (not the API
reference - see `hardsid_dll_readme.txt` for the function list). Written
after finding several of these the hard way while working on AIASS and
SIDBlaster-ASID-Player - see this fork's top-level README for the actual
bug fixes these observations came out of.

## Loading the library

**Windows**: either link against `hardsid.lib` at compile time, or load
dynamically with `LoadLibrary("hardsid.dll")` (a bare filename, not a
full path) + `GetProcAddress` per function. The bare filename matters:
Windows' default DLL search order already checks the calling
executable's own directory first, so `LoadLibrary("hardsid.dll")` finds
a copy sitting next to your .exe automatically. Building an explicit
full path yourself (e.g. `GetModuleFileName` + append `"hardsid.dll"`)
works too but adds nothing - and if that path construction has a bug,
you get a confusing "can't load library" instead of it just working.

**Linux/macOS**: `dlopen("/usr/local/lib/libhardsid.so", RTLD_NOW)` (or
`.dylib` on macOS) + `dlsym` per function, or link directly against
`-lhardsid` if it's installed where the linker looks (this fork's
CMake now has an `install()` rule targeting `/usr/local/lib` to match).
`hardsid.hpp` declares the functions with plain `extern "C"` linkage -
no calling-convention decoration needed here, unlike Windows (see
below), because POSIX/Unix has one standard C ABI, not the
cdecl-vs-stdcall split x86 Windows has.

## Calling convention (Windows only)

The Windows exports use `__stdcall`. If you're binding function
pointers yourself (`GetProcAddress`) rather than linking against
`hardsid.lib`, your function pointer typedefs must say so (`typedef
Uint8 (__stdcall* lpHardSID_Read)(...)` or the equivalent `CALLBACK`
macro) - get this wrong and you'll get stack corruption that often
doesn't crash immediately, making it a nasty one to debug. Not a
concern on Linux/macOS.

## POSIX: you must call `HardSID_Uninitialize()` yourself before unload/exit

**This is the one that actually bites people.** On Windows, the
background dispatcher thread gets shut down automatically:
`dllmain.cpp`'s `DllMain` calls `Uninitialize()` on
`DLL_PROCESS_DETACH`, which fires when `FreeLibrary()` drops the last
reference or the process exits. There is no `DllMain` equivalent wired
up for the `.so`/`.dylib` build - nothing calls `Uninitialize()` for you
on Linux/macOS. `HardSID_Uninitialize()` is exported specifically to
cover this gap (see `hardsiddll.cpp` - it's a no-op on Windows, since
`DllMain` already handled it there):

```cpp
#if defined(__linux__) || defined(__APPLE__)
  HardSID_Uninitialize();
#endif
```

Call this before your process exits, and *definitely* before you
`dlclose()` the library explicitly (e.g. if your host supports
unloading/reloading plugins at runtime) - `Uninitialize()` joins the
background dispatcher thread before returning. Skip it and `dlclose()`
can unmap code out from under a thread that's still executing it: a
real, not theoretical, crash. Both AIASS and SIDBlaster-ASID-Player
already do this correctly in their destructors - copy that pattern.

## `Cycles` is a relative delta, not a frame position

Every write/read/delay takes a `Cycles` argument (SID clock cycles).
It's relative to the *previous* call on that device, not an absolute
position or a "cycles into this frame" value - if you accumulate a
growing counter and pass that, timing will drift and get worse the
longer playback runs. Concretely:

- Two writes 8 cycles apart: pass `8`, then `8` again - not `8`, then
  `16`.
- The minimum meaningful value for HardSID-class devices is `8` - don't
  go lower.
- If you're writing a buffer-draining thread that needs to track
  real-time playback and your `Cycles` value is a small fixed constant
  per call (rather than a computed real-elapsed-time delta), your
  timing accuracy comes from *how often you call the driver*, not from
  anything the driver does internally. A tight polling loop isn't
  incidental overhead in that design, it's the actual timing mechanism -
  don't "optimize" it into a sleep/yield without understanding that
  first (ask us how we found this out).

## `HardSID_Try_Write` and the BUSY retry loop

`HardSID_Try_Write` is non-blocking - it returns `HSID_USB_WSTATE_BUSY`
if the internal command buffer is full instead of blocking. Callers are
expected to retry, but **with a bounded timeout**, not an unconditional
loop:

```cpp
auto start = std::chrono::steady_clock::now();
while (HardSID_Try_Write(dev, cycles, reg, data) == HSID_USB_WSTATE_BUSY) {
    std::this_thread::yield();
    if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(500)) {
        // give up / report an error - do not spin forever
        break;
    }
}
```

`yield()` between attempts, not a fixed-duration `sleep()` - a sleep
longer than roughly a millisecond is enough to visibly slow down
playback in a tight write loop (Windows' default timer granularity
rounds `sleep()` up to ~15ms unless something has called
`timeBeginPeriod`/an equivalent, which is far too coarse for
SID-register timing).

## Checking the DLL version

`HardSID_Version()` returns a `Uint16` (not `int` - watch your function
pointer typedef if binding dynamically, see above). Compare it as a
plain integer against the hex constants documented in
`hardsid_dll_readme.txt` (e.g. `>= 0x0203` for `HardSID_SetSIDType`/
`SetSerial`/`SetWriteBufferSize` availability, `>= 0x0204` for this
fork's dispatcher fixes - see the top-level README). Log both the
decimal and hex form somewhere visible; hex is what you'll be comparing
against in code, decimal is what most people read on screen.

## Full example: open, play a tone, close

Ties together everything above - dynamic loading on both platforms,
version/device checks, the bounded `Try_Write` retry, and the POSIX
`Uninitialize()` requirement. Builds as-is on Windows and POSIX from the
same source.

```cpp
#include <cstdio>
#include <chrono>
#include <thread>

#if defined(_WIN32)
    #include <windows.h>
    #define HSID_CALL __stdcall
#else
    #include <dlfcn.h>
    #define HSID_CALL
#endif

typedef unsigned char  Uint8;
typedef unsigned short Uint16;

enum { HSID_USB_WSTATE_OK = 1, HSID_USB_WSTATE_BUSY, HSID_USB_WSTATE_ERROR };

typedef Uint16 (HSID_CALL *lpHardSID_Version)(void);
typedef Uint8  (HSID_CALL *lpHardSID_Devices)(void);
typedef Uint8  (HSID_CALL *lpHardSID_Try_Write)(Uint8 device, int cycles, Uint8 reg, Uint8 data);
typedef void   (HSID_CALL *lpHardSID_Reset)(Uint8 device);
typedef void   (HSID_CALL *lpHardSID_Uninitialize)(void);

static lpHardSID_Version      HardSID_Version      = nullptr;
static lpHardSID_Devices      HardSID_Devices      = nullptr;
static lpHardSID_Try_Write    HardSID_Try_Write    = nullptr;
static lpHardSID_Reset        HardSID_Reset        = nullptr;
static lpHardSID_Uninitialize HardSID_Uninitialize = nullptr;

#if defined(_WIN32)
static HMODULE g_lib = nullptr;
#else
static void*   g_lib = nullptr;
#endif

// Bounded BUSY retry - see "HardSID_Try_Write and the BUSY retry loop" above.
static bool WriteWithTimeout(Uint8 device, int cycles, Uint8 reg, Uint8 data) {
    auto start = std::chrono::steady_clock::now();
    while (HardSID_Try_Write(device, cycles, reg, data) == HSID_USB_WSTATE_BUSY) {
        std::this_thread::yield();
        if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(500)) {
            return false;  // gave up - caller decides whether that's fatal
        }
    }
    return true;
}

static bool OpenDriver() {
#if defined(_WIN32)
    g_lib = LoadLibrary("hardsid.dll");  // bare name - see "Loading the library"
    if (!g_lib) return false;
    HardSID_Version      = (lpHardSID_Version)     GetProcAddress(g_lib, "HardSID_Version");
    HardSID_Devices      = (lpHardSID_Devices)     GetProcAddress(g_lib, "HardSID_Devices");
    HardSID_Try_Write    = (lpHardSID_Try_Write)   GetProcAddress(g_lib, "HardSID_Try_Write");
    HardSID_Reset        = (lpHardSID_Reset)       GetProcAddress(g_lib, "HardSID_Reset");
    HardSID_Uninitialize = (lpHardSID_Uninitialize)GetProcAddress(g_lib, "HardSID_Uninitialize");
#else
    #if defined(__APPLE__)
    g_lib = dlopen("/usr/local/lib/libhardsid.dylib", RTLD_NOW);
    #else
    g_lib = dlopen("/usr/local/lib/libhardsid.so", RTLD_NOW);
    #endif
    if (!g_lib) return false;
    HardSID_Version      = (lpHardSID_Version)     dlsym(g_lib, "HardSID_Version");
    HardSID_Devices      = (lpHardSID_Devices)     dlsym(g_lib, "HardSID_Devices");
    HardSID_Try_Write    = (lpHardSID_Try_Write)   dlsym(g_lib, "HardSID_Try_Write");
    HardSID_Reset        = (lpHardSID_Reset)       dlsym(g_lib, "HardSID_Reset");
    HardSID_Uninitialize = (lpHardSID_Uninitialize)dlsym(g_lib, "HardSID_Uninitialize");
#endif
    // One or more exports missing usually means a stale/wrong-version DLL,
    // not a code bug - check this before dereferencing any of them.
    return HardSID_Version && HardSID_Devices && HardSID_Try_Write
        && HardSID_Reset && HardSID_Uninitialize;
}

static void CloseDriver() {
#if defined(_WIN32)
    // DllMain's DLL_PROCESS_DETACH already calls Uninitialize() for us.
    if (g_lib) FreeLibrary(g_lib);
#else
    // No such hook on POSIX - see "POSIX: you must call HardSID_Uninitialize()" above.
    if (HardSID_Uninitialize) HardSID_Uninitialize();
    if (g_lib) dlclose(g_lib);
#endif
    g_lib = nullptr;
}

int main() {
    if (!OpenDriver()) {
        std::fprintf(stderr, "could not load hardsid driver (missing DLL or export)\n");
        return 1;
    }

    Uint16 version = HardSID_Version();
    std::printf("hardsid driver version: %u (0x%04X)\n", version, version);
    if (version < 0x0203) {
        std::fprintf(stderr, "driver too old (need >= 0x0203)\n");
        CloseDriver();
        return 1;
    }

    Uint8 deviceCount = HardSID_Devices();
    std::printf("devices found: %u\n", deviceCount);
    if (deviceCount == 0) {
        CloseDriver();
        return 1;
    }

    const Uint8 device = 0;
    HardSID_Reset(device);

    // Typical "play a tone" sequence: frequency, then waveform+gate-on,
    // then (after the tone has sounded) gate-off. Cycles is a delta since
    // the *previous* call on this device (see "Cycles is a relative
    // delta" above) - 8 is the minimum meaningful value for HardSID-class
    // devices, used here for every write since these aren't rapid
    // back-to-back register pokes.
    WriteWithTimeout(device, 8, 0x00, 0x18);  // voice 1 freq lo
    WriteWithTimeout(device, 8, 0x01, 0x10);  // voice 1 freq hi - some audible pitch
    WriteWithTimeout(device, 8, 0x18, 0x0F);  // master volume (max), filter off
    WriteWithTimeout(device, 8, 0x04, 0x11);  // voice 1: triangle waveform, gate on

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    WriteWithTimeout(device, 8, 0x04, 0x10);  // voice 1: gate off (release phase)

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    CloseDriver();
    return 0;
}
```

## Write buffering (`HardSID_SetWriteBufferSize`)

This batches multiple register writes into one USB transfer before
sending, trading a small amount of added latency for less USB overhead
at high write rates. It's a *separate* concept from the internal
command queue `HardSID_Try_Write` uses (that one's about backpressure
between your thread and the dispatcher thread, not about USB packet
sizing) - the two interact but aren't the same knob. If you're chasing
either lower latency or lower CPU/USB overhead, check which layer
you're actually looking at before tuning either one.
