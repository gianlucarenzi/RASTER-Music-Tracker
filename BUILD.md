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
no MinGW toolchain provides `afxwin.h`. The Qt5 frontend (`-DRMT_USE_QT=ON`,
see below) needs a Qt5 built for MinGW, not tested yet. For the full MFC
tracker use MSVC.

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

## Qt5 frontend (Linux / POSIX) - work in progress

The tracker GUI is the MFC code itself (`RmtView.cpp`, `RmtDoc.cpp` and the
GUI-shared drawing code) compiled against `src/MfcTypes.h`, a small
replacement of the MFC classes it uses: a software device context (`CDC`,
`CBitmap`, bitmaps of `src/res` compiled in by `cmake/EmbedResources.cmake`),
message maps that build a real command table, and `CWnd`/`CView` whose window
operations go to an `IRmtHost`. `src/qt/` implements that host with Qt5:

| File | |
|------|---|
| `qt/main-qt.cpp` | start-up of `CRmtApp::InitInstance()`, main window |
| `qt/RmtQtFrontend.cpp` | `RmtMainWindow`, `RmtViewWidget` (shows the view, passes keys/mouse/wheel/focus), `IRmtHost` (timers, message boxes, cursors, key state, title/status bar) |
| `qt/RmtQtKeys.cpp` | key events -> Win32 VK codes: on Linux by physical key (scan code), like a US keyboard on Windows |
| `qt/QtMainFrame.cpp` | the `CMainFrame` members the GUI code uses (MainFrm.cpp builds MFC toolbars) |

```bash
sudo apt install qtbase5-dev
cmake -B build-qt                  # RMT_USE_QT is ON by default off Windows
cmake --build build-qt -j
./build-qt/out/rmt song.rmt
```

Status:

- ✅ the main screen, drawn by the original code (tracks, song, instrument,
  info, POKEY registers), window resize, the 16 ms screen timer
- ✅ keyboard (navigation, editing keys), mouse buttons, wheel, cursors
- ✅ a song given on the command line is loaded
- ❌ menus and toolbars (next step: from `Rmt.rc`, dispatched through the
  message maps)
- ❌ dialogs: the MFC dialogs are stubs that answer "cancel"
  (`MfcDialogStubs.cpp`), they have to be rewritten in Qt
- ✅ 6502 and POKEY emulation built in (`src/emu`), used when `sa_c6502.dll` /
  `apokeysnd.dll` are not there (always outside Windows): the tracker driver
  runs, notes and instruments play inside the engine
- ❌ sound output: the DirectSound device of `MfcTypes.h` is silent and the song
  timer (`timeSetEvent`) does not run yet (next: PortAudio + a timer thread)
- ❌ MIDI input

Test without a display: `RMT_QT_GRAB=shot.png` saves the window after 1 s and
quits, `RMT_QT_KEYS=108,106` first presses keys (Linux evdev codes), message
boxes are answered automatically:

```bash
QT_QPA_PLATFORM=offscreen RMT_QT_GRAB=shot.png ./build-qt/out/rmt song.rmt
```

`RmtCoreTest --screenshot out.ppm [song.rmt]` draws the main screen without Qt;
`RmtCoreTest --play song.rmt frames regs.txt [out.wav]` plays a song with the
engine (one `CSong::TimerRoutine()` per frame), writes the POKEY registers of
every frame and the sound of the built-in POKEY.

### Built-in 6502 and POKEY (`src/emu`)

| File | Replaces | |
|------|----------|---|
| `emu/Cpu6502.cpp` | `sa_c6502.dll` | NMOS 6502, documented opcodes, cycle counted (page crossing, branches), flat 64 KB memory; `C6502_JSR` runs until the RTS or the cycles |
| `emu/PokeySound.cpp` | `apokeysnd.dll` | one or two POKEYs (`PutByte` 0x10.. = second chip), cycle based model (clocks, 16 bit, filters, polys, distortions), 44100 Hz, 2 interleaved channels |

Derived from the emulation of `AT2019/ATARI-Driver/RmtSkeleton/tools/rmtplay`.
`C6502.cpp` and `Pokey.cpp` load the DLLs first, as before, and fall back to
these. Checked with `RmtCoreTest --play` on gemx.rmt against `rmtplay` (an
independent player and 6502 core): over 800 frames AUDC and AUDCTL are
identical and AUDF within 1 (RMT recomputes its frequency tables from the
tuning, rmtplay has the original tables); the sound correlates 0.98 (chroma)
and 0.97 (loudness per frame).

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
| `-DRMT_USE_QT=ON` | ❌ no Qt5 for MinGW installed |

Linux native GCC 10: `-DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON` builds and
`RmtCoreTest` runs ("finished successfully", `--screenshot` draws gemx.rmt);
the Qt5 frontend builds and shows the main screen with a song, keys move the
cursor (checked offscreen with `RMT_QT_GRAB` / `RMT_QT_KEYS`).

MSVC builds: not tested here.
