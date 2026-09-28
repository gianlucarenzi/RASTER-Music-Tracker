# Building RASTER Music Tracker with CMake

This guide covers building RMT on Windows (MSVC/MinGW) and Linux (MinGW cross-compile).

## Prerequisites

### Windows (MSVC)
- Visual Studio 2022 or later
- CMake 3.25+
- Windows 10 SDK or later

### Windows (MinGW native)
- MinGW-w64 x86_64 compiler
- CMake 3.25+

### Linux (MinGW cross-compile → Windows)
```bash
sudo apt install cmake mingw-w64
```

---

## Building on Windows with MSVC

```bash
cd RASTER-Music-Tracker
mkdir build-msvc
cd build-msvc
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

Output: `out\Rmt.exe`

---

## MinGW (Windows native or Linux cross-compile)

**What builds with MinGW:** the RMT engine and the audio/MIDI backends.
**What does not:** the full tracker. Its GUI (Rmt.cpp, MainFrm.cpp, RmtView.cpp,
RmtDoc.cpp and the dialogs, 18 files) is MFC, and MFC exists only for MSVC:
no MinGW toolchain provides `afxwin.h`. The Qt5 frontend (`-DRMT_USE_QT=ON`)
has no sources yet (MFC -> Qt migration, Fase 3+), so it does not link either
(`undefined reference to main`, on Linux too). For the full tracker use MSVC.

### Cross-compile from Linux

```bash
sudo apt install cmake mingw-w64
cd RASTER-Music-Tracker
cmake -B build-mingw-core -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake \
      -DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON
cmake --build build-mingw-core -j
```

Output in `build-mingw-core/out/`:

| File | Content |
|------|---------|
| `RmtCoreTest.exe` | the RMT engine (Song, Tracks, Instruments, C6502, Pokey, IO, SAP/ASM/WAV export) + the GUI-shared drawing code, without MFC |
| `Rmt.exe` | audio backend test (`main-portaudio.cpp`) |
| `RmtMidiTest.exe` | MIDI backend test (`main-midi.cpp`) |

Notes:

- `mingw-toolchain.cmake` picks `x86_64-w64-mingw32-g++-posix` when present.
  On Debian/Ubuntu the plain `x86_64-w64-mingw32-g++` uses the "win32" thread
  model, which has no `std::thread` / `std::this_thread` (used by the test
  programs).
- PortAudio is optional on Windows: when `portaudio.h` is not found the
  PortAudio backend is left out (`RMT_NO_PORTAUDIO`) and the audio factory uses
  DirectSound. DirectSound and WinMM come with MinGW.
- Without `-DRMT_BUILD_CORE_ONLY=ON` the full MFC build is attempted and stops
  on `afxwin.h` (see above).

### Windows with MinGW native (MSYS2)

Same targets, from an MSYS2 MinGW64 shell:

```bash
cmake -G "MinGW Makefiles" -B build-mingw-core -DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON
cmake --build build-mingw-core
```

(Not tested on Windows; the sources are the same as the cross-compile.)

### Linux native

```bash
cmake -B build-core-linux -DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON
cmake --build build-core-linux
./build-core-linux/out/RmtCoreTest
```

---

## CMake Options

- `-DCMAKE_BUILD_TYPE=Release` - Build optimized release version
- `-DCMAKE_BUILD_TYPE=Debug` - Build with debug symbols
- `-DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake` - Use MinGW toolchain (for cross-compile)

---

## Troubleshooting

### DirectSound/WinMM not found (cross-compile)
This is expected on Linux. The libraries are found in the MinGW sysroot:
- `/usr/x86_64-w64-mingw32/lib/libdsound.a`
- `/usr/x86_64-w64-mingw32/lib/libwinmm.a`

### MinGW not in PATH
Install MinGW: `sudo apt install mingw-w64`

### CMake configuration fails
Check that all prerequisites are installed and in PATH.

---

## Output Artifacts

### Windows executable
- **MSVC:** `build-msvc/out/Rmt.exe` (the full tracker)
- **MinGW:** `build-mingw-core/out/RmtCoreTest.exe`, `Rmt.exe` (audio test),
  `RmtMidiTest.exe` - engine and backends only, no tracker GUI (see above)

All output binaries are in `out/` subdirectory of the build folder.

---

## Next Steps: Phase 2 (POSIX Support)

To enable POSIX support (Linux/macOS), Phase 2 will:
1. Replace MFC with wxWidgets or Qt (cross-platform GUI)
2. Replace DirectSound/WinMM with PortAudio (cross-platform audio)
3. Replace Windows MIDI API with RtMidi (cross-platform MIDI)
4. Add Linux/macOS CI/CD workflows

Phase 1 (current) establishes the CMake foundation needed for Phase 2.

---

## Test Results

MinGW-w64 GCC 10 (posix threads), cross-compiled on Linux, CMake 3.27:

| Configuration | Result |
|---------------|--------|
| `-DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON` | ✅ `RmtCoreTest.exe`, `Rmt.exe`, `RmtMidiTest.exe` build (not run: needs Windows or Wine) |
| default (full MFC GUI) | ❌ 18 MFC files: `afxwin.h` not available with MinGW |
| `-DRMT_USE_QT=ON` | ❌ no Qt5 for MinGW installed; the Qt frontend has no sources yet |

Linux native GCC 10: `-DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON` builds and
`RmtCoreTest` runs ("finished successfully"); `-DRMT_USE_QT=ON` stops at link
time (`undefined reference to main`, no Qt sources yet).

MSVC builds: not tested here.
