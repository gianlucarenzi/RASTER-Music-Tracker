# Building RASTER Music Tracker with CMake

The **official build is the Qt5 frontend on Linux / POSIX** (`RMT_USE_QT=ON`,
the default there). On Windows the MFC GUI built with MSVC is still the
default; MinGW builds the engine and the audio/MIDI backends only.

| Platform | Build | GUI | Section |
|----------|-------|-----|---------|
| Linux / POSIX | `cmake -B build-qt` | Qt5 (**official**) | [Qt5 frontend](#official-build-qt5-frontend-linux--posix) |
| Windows | MSVC | MFC | [Windows with MSVC](#windows-with-msvc-mfc) |
| Windows / Linux | MinGW | none (engine, backends, tests) | [MinGW](#mingw-engine-and-backends-only) |

---

## Official build: Qt5 frontend (Linux / POSIX)

### Prerequisites

- CMake 3.25+ and a C++20 compiler (GCC 10 or later)
- Qt 5.12 or later (Core, Widgets)
- PortAudio, for sound (without it the tracker runs silent)
- RtMidi, optional (MIDI input is not wired to the Qt frontend yet)

```bash
sudo apt install cmake qtbase5-dev portaudio19-dev
```

### Build and run

```bash
cmake -B build-qt -DCMAKE_BUILD_TYPE=Release   # RMT_USE_QT is ON by default off Windows
cmake --build build-qt -j
./build-qt/out/rmt song.rmt                    # or the versioned binary: rmt-2.0.0
```

The build is free of compiler warnings with GCC 10 (`-Wall -Wextra`).

### Loading and playing a song

A song can be loaded from the command line at start-up:

```bash
./build-qt/out/rmt legacy/rmt_128/songs/thrust.rmt
```

Or via **File → Load…** (`Ctrl+L`) from the menu bar. Give the window focus
and press **F5** to play.

The file dialogs are Qt `QFileDialog`s: `CFileDialog::DoModal()` asks the
frontend (`IRmtHost::FileDialog`), so the MFC code in `IO_Song.cpp` is
unchanged. They work for songs (Load, Save, Save As: RMT, TXT, RMW),
instruments (RTI) and tracks (TXT), keep the last folder, propose the current
file name and return the chosen file type as with MFC. The filters match
upper case extensions too (`*.rmt *.RMT`), and a name typed without extension
gets the one of the chosen type. Import works for MOD and TMC; Export As
shows its file dialog, but most formats then stop at an options dialog that
is still a stub (see [Status](#status)).

At start-up a message box may say that `tuning.ini` is missing; the default
tuning is used and the message is harmless.

### Menu bar

All 7 top-level menus from `Rmt.rc` (`IDR_MAINFRAME MENU`) are present:

| Menu | Contents |
|------|---------|
| **File** | New, Load, Reload, Save, Save As, Import, Export As, Exit |
| **Edit** | Undo, Redo, Clear Undo & Redo history |
| **Track** | Copy/Paste/Cut/Delete, Info, loop tools, renumber, load/save track, cleanup |
| **Block** | Backup, Copy/Paste/Cut/Delete, Paste special (submenu), Effects, Select all |
| **Instrument** | Copy/Paste/Cut/Delete, Paste special (submenu, 9 items), Info, renumber, load/save, cleanup |
| **Song** | Line operations, 4/8 channel switch, order change, length, optimizations |
| **View** | Configuration, Tuning, toolbar toggles, Play time counter, Volume analyzer, Pokey regs, Instrument active help |
| **Help** | Help Topics, Online Help, About |

Menu items are enabled/disabled automatically via `ON_UPDATE_COMMAND_UI`
handlers (implemented by `QtCCmdUI`, triggered on `QMenu::aboutToShow`).

### Status

- ✅ the main screen, drawn by the original code (tracks, song, instrument,
  info, POKEY registers), window resize, the 16 ms screen timer. The widget
  shows the view's own bitmap (`m_mem_dc`) directly: `RmtQtBridge::Paint()`
  runs `CRmtView::OnDraw()` without its final `StretchBlt`, and the scaling
  (`SCALEPERCENTAGE` > 100) is done by `QPainter` (nearest neighbour; at
  scaled sizes a duplicated row/column may land one pixel apart from the MFC
  version)
- ✅ keyboard (navigation, editing keys), mouse buttons, wheel, cursors
- ✅ a song given on the command line is loaded
- ✅ full menu bar (7 menus, 81 actions, all with handlers; auto-tested)
- ✅ file dialogs (Load / Save / Save As, instrument and track load/save,
  the file choice of Import and Export As), `QFileDialog`
- ✅ File → New (`IDD_FILENEW`): track length 1-256 and mono/stereo, with the
  confirmation for tracks longer than 64 lines
- ✅ File → Import: the MOD and TMC options (`IDD_IMPORTMOD`, `IDD_IMPORTTMC`)
  and the "Import of module finished" dialogs (`IDD_IMPORTMODFINISHED`,
  `IDD_IMPORTTMCFINISHED`); checked with `rmt/imports/axel_f.mod`
  (ProTracker) and `rmt/imports/404_Error.tmc` (Theta Music Composer)
- ⚠️  other dialogs: the MFC dialogs are stubs that answer "cancel"
  (`MfcDialogStubs.cpp`); they must still be rewritten in Qt. This stops
  the export options (all formats but LZSS)
- ✅ 6502 and POKEY emulation built in (`src/emu`), used when `sa_c6502.dll` /
  `apokeysnd.dll` are not there (always outside Windows): the tracker driver
  runs, notes and instruments play inside the engine
- ✅ playback with sound: the song timer (`timeSetEvent`, a thread; a timer
  re-created from its own tick keeps the deadline, so the tempo does not
  drift) runs `CSong::TimerRoutine()`, and the DirectSound buffer the sound
  code streams to is played by PortAudio with real cursors (`MfcAudio.cpp`,
  `RMT_HAVE_PORTAUDIO`). A re-created timer waits for the tick that created
  it to return, and a timer more than 200 ms late restarts from now instead
  of catching up: before, a stall of more than one tick (window move, load,
  debugger) let two ticks run at once, the timers doubled at every stall and
  the process ended at 100% on all cores (thousands of threads, then an
  abort). `CSongTimer::StopTimer()` is called on every exit (`main-qt.cpp`)
  and no tick can start a new timer once it has run, so the song timer no
  longer runs into the destruction of `g_Song` (segfault on exit)
- ✅ CPU (offscreen, gemx.rmt): about 50% of one core idle and while playing
  (was 78% / 66%), almost all of it the 60 fps redraw; `CDC::BitBlt` copies
  clipped rows with `memmove`, `CDC::StretchBlt` precomputes its columns
- ❌ MIDI input
- ❌ toolbars (menu shortcuts are all present)

### Testing without a display

`RMT_QT_GRAB=shot.png` saves the window after 1 s (`RMT_QT_GRAB_MS`) and
quits, `RMT_QT_KEYS=108,106` first presses keys (Linux evdev codes, 63 = F5
play), message boxes are answered automatically; `RMT_AUDIO_DUMP=out.wav`
replaces the sound card with a thread that takes the sound buffer in real time
and writes it to a WAV file:

```bash
QT_QPA_PLATFORM=offscreen RMT_QT_GRAB=shot.png ./build-qt/out/rmt song.rmt
QT_QPA_PLATFORM=offscreen RMT_AUDIO_DUMP=play.wav RMT_QT_KEYS=63 RMT_QT_GRAB_MS=6000 \
    RMT_QT_GRAB=shot.png ./build-qt/out/rmt song.rmt
```

`RMT_QT_COMMANDS` then triggers the menu actions of the given command IDs
(`resource.h`, decimal or `0x` hex), and `RMT_QT_FILEDIALOG` answers the file
dialogs in turn (the file type is taken from the extension; none left:
cancel). Load a song, save it as TXT, load the TXT and save it as RMT:

```bash
QT_QPA_PLATFORM=offscreen RMT_QT_GRAB=1 RMT_QT_GRAB_MS=3000 \
    RMT_QT_COMMANDS=0xE101,0xE104,0xE101,0xE104 \
    RMT_QT_FILEDIALOG=gemx.rmt,/tmp/g.txt,/tmp/g.txt,/tmp/g2.rmt ./build-qt/out/rmt
cmp gemx.rmt /tmp/g2.rmt     # identical
```

`RMT_QT_DIALOG=dialog.png` shows the other dialogs too (without it they are
cancelled in test runs): each is saved after 0.5 s (the first to
`dialog.png`, the next ones to `dialog-2.png`, `dialog-3.png`...) and
confirmed the way a user does it (for the import result: check "I
understand", then OK), through the same checks as a click on OK:

```bash
QT_QPA_PLATFORM=offscreen RMT_QT_GRAB=shot.png RMT_QT_GRAB_MS=3000 \
    RMT_QT_COMMANDS=0xE100 RMT_QT_DIALOG=new.png ./build-qt/out/rmt song.rmt
QT_QPA_PLATFORM=offscreen RMT_QT_GRAB=shot.png RMT_QT_GRAB_MS=5000 \
    RMT_QT_COMMANDS=32856 RMT_QT_FILEDIALOG=rmt/imports/axel_f.mod \
    RMT_QT_DIALOG=import.png ./build-qt/out/rmt      # import.png, import-2.png
```

`ID_FILE_NEW` = `0xE100`, `ID_FILE_OPEN` = `0xE101`, `ID_FILE_SAVE_AS` = `0xE104`, `ID_FILE_IMPORT` =
32856, `ID_FILE_EXPORT_AS` = 32773, `ID_INSTR_LOAD` / `ID_INSTR_SAVE` = 32772
/ 32771, `ID_TRACK_LOAD` / `ID_TRACK_SAVE` = 32888 / 32889.

`RMT_QT_MENU_TEST=1` (use with `RMT_QT_GRAB=1` and `QT_QPA_PLATFORM=offscreen`)
triggers all 81 leaf menu actions programmatically and exits 0 if every action
has a registered handler, 1 otherwise:

```bash
QT_QPA_PLATFORM=offscreen RMT_QT_GRAB=1 RMT_QT_MENU_TEST=1 \
    ./build-qt/out/rmt
# RMT_QT_MENU_TEST: triggered 81 menu actions
# RMT_QT_MENU_TEST: PASS
```

Checked this way with gemx.rmt: after F5 the time counter shows 5.50 s at
5.5 s, the sound correlates 0.985 (chroma) and 0.989 (loudness, 10 ms steps)
with `rmtplay`, with the same delay at the start and at the end (no drift).

`RmtCoreTest --screenshot out.ppm [song.rmt]` draws the main screen without Qt;
`RmtCoreTest --play song.rmt frames regs.txt [out.wav]` plays a song with the
engine (one `CSong::TimerRoutine()` per frame), writes the POKEY registers of
every frame and the sound of the built-in POKEY (see
[Linux native core build](#linux-native-core-build)).

### How it works

The tracker GUI is the MFC code itself (`RmtView.cpp`, `RmtDoc.cpp` and the
GUI-shared drawing code) compiled against `src/MfcTypes.h`, a small
replacement of the MFC classes it uses: a software device context (`CDC`,
`CBitmap`, bitmaps of `src/res` compiled in by `cmake/EmbedResources.cmake`),
message maps that build a real command table, and `CWnd`/`CView` whose window
operations go to an `IRmtHost`. `src/qt/` implements that host with Qt5:

| File | |
|------|---|
| `qt/main-qt.cpp` | start-up and main window; `RMT_QT_GRAB` / `RMT_QT_MENU_TEST` test hooks |
| `qt/RmtQtFrontend.cpp` | `RmtMainWindow` (menu bar, `QtCCmdUI`), `RmtViewWidget` (view, keys/mouse/wheel/focus), `IRmtHost` (timers, message boxes, cursors, key state, title/status bar) |
| `qt/RmtQtKeys.cpp` | key events → Win32 VK codes: on Linux by physical key (scan code), like a US keyboard on Windows |
| `qt/RmtQtDialogs.cpp` | the MFC dialogs rewritten with Qt: `CDialog::DoModal()` → `IRmtHost::DoModal()` → the dialog of `m_nIDTemplate` (`IDD_*`), which reads and writes the dialog's data members; not rewritten yet: cancelled |
| `qt/QtMainFrame.cpp` | the `CMainFrame` members the GUI code uses (MainFrm.cpp builds MFC toolbars) |

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

## Windows with MSVC (MFC)

Prerequisites:
- Visual Studio 2022 or later
- CMake 3.25+
- Windows 10 SDK or later

```bash
cd RASTER-Music-Tracker
mkdir build-msvc
cd build-msvc
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

Output: `out\Rmt.exe`

On Windows `RMT_USE_QT` is OFF by default and the MFC GUI is built.
`-DRMT_USE_QT=ON` selects the Qt5 frontend there too, but that is not tested
yet (see [Remaining work](#remaining-work-on-the-qt-frontend)).

---

## MinGW (engine and backends only)

**What builds with MinGW:** the RMT engine and the audio/MIDI backends.
**What does not:** the full tracker. Its MFC GUI (Rmt.cpp, MainFrm.cpp,
RmtView.cpp, RmtDoc.cpp and the dialogs, 18 files) exists only for MSVC:
no MinGW toolchain provides `afxwin.h`. The Qt5 frontend (`-DRMT_USE_QT=ON`)
needs a Qt5 built for MinGW, not tested yet. For the full MFC tracker use MSVC.

Prerequisites:
- MinGW-w64 x86_64 compiler (Linux: `sudo apt install cmake mingw-w64`)
- CMake 3.25+

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

### Linux native core build

```bash
cmake -B build-core-linux -DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON
cmake --build build-core-linux
./build-core-linux/out/RmtCoreTest
```

---

## Versioning

The version string baked into the binary is derived **at CMake configure time**
from the nearest git tag. A single source of truth; no hardcoded version
strings in C++ source files.

| Situation | Version shown |
|-----------|--------------|
| On an exact tag `v2.0-rc1` | `2.0-rc1` |
| Commits after that tag | `2.0-rc1+dev` |
| No tag reachable | `2.0-dev` |

`RMT_BASE_VERSION` in `CMakeLists.txt` is the numeric `MAJOR.MINOR` used by
the release scripts. The generated header `RmtVersion.h` (build directory)
exposes `RMT_VERSION_FULL` and `RMT_VERSION_STRING`.

### Release candidate workflow

Each push to the main development branch should be tagged as a release
candidate. Use `scripts/push.sh` instead of a bare `git push`:

```bash
./scripts/push.sh            # auto-creates v2.0-rcN (N = last+1) and pushes branch + tag
```

The script reads `RMT_BASE_VERSION` from `CMakeLists.txt`, finds the highest
existing `v<BASE>-rcN`, increments N, creates the tag, and pushes both the
branch and the tag to `origin`.

### Final release

When the release candidate cycle is finished:

```bash
./scripts/release.sh 2.1     # creates v2.1, pushes branch + tag
```

After a release, update `RMT_BASE_VERSION` in `CMakeLists.txt` to the next
development target (e.g. `"2.1"`) so subsequent RC tags follow the new series.

---

## CMake Options

- `-DRMT_USE_QT=ON|OFF` - Qt5 frontend (default ON on Linux/POSIX, OFF on Windows)
- `-DRMT_BUILD_CORE_ONLY=ON` - Build the engine and backends only, no GUI
- `-DRMT_CORE_TEST=ON` - Also build `RmtCoreTest` (with `RMT_BUILD_CORE_ONLY`)
- `-DCMAKE_BUILD_TYPE=Release` - Build optimized release version
- `-DCMAKE_BUILD_TYPE=Debug` - Build with debug symbols
- `-DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake` - Use MinGW toolchain (for cross-compile)

---

## Troubleshooting

### Qt5 not found
Install the Qt5 development package: `sudo apt install qtbase5-dev`, or point
CMake at another Qt5 with `-DCMAKE_PREFIX_PATH=/path/to/Qt5`.

### No sound with the Qt frontend
PortAudio was not found at configure time: `sudo apt install portaudio19-dev`,
then configure again.

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

- **Linux / POSIX (Qt5, official):** `build-qt/out/rmt` and the versioned
  `rmt-<version>`, with `resources/` next to them
- **Windows, MSVC:** `build-msvc/out/Rmt.exe` (the full MFC tracker)
- **MinGW:** `build-mingw-core/out/RmtCoreTest.exe`, `Rmt.exe` (audio test),
  `RmtMidiTest.exe` - engine and backends only, no tracker GUI (see above)

All output binaries are in `out/` subdirectory of the build folder.

---

## Remaining work on the Qt frontend

The Qt5 frontend is the official build on Linux/POSIX and the development
track for the 2.x series. Remaining work:

1. **Native Qt dialogs** — rewrite the `MfcDialogStubs.cpp` dialogs in Qt
   (file dialogs, File → New and Import are done): export options,
   configuration, tuning, song/track/instrument info. A new dialog is a
   case in `RmtQtRunDialog()` (`qt/RmtQtDialogs.cpp`) plus the `IDD` in the
   stub's constructor
2. **MIDI input** — wire `RtMidiBackend` to the Qt event loop
3. **Toolbars** — recreate the MFC rebars as Qt toolbars
4. **Windows Qt build** — package Qt5 for MinGW/MSVC and test
   `-DRMT_USE_QT=ON` on Windows, so Qt can become the default there too

---

## Test Results

Linux native GCC 10: the Qt5 frontend builds without warnings, shows the main
screen with a song, keys move the cursor, all 81 menu actions are verified via
`RMT_QT_MENU_TEST` (checked offscreen with `RMT_QT_GRAB` / `RMT_QT_KEYS`).
`-DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON` builds without warnings and
`RmtCoreTest` runs ("finished successfully", `--screenshot` draws gemx.rmt).

MinGW-w64 GCC 10 (posix threads), cross-compiled on Linux, CMake 3.27:

| Configuration | Result |
|---------------|--------|
| `-DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON` | ✅ `RmtCoreTest.exe`, `Rmt.exe`, `RmtMidiTest.exe` build (not run: needs Windows or Wine) |
| default (full MFC GUI) | ❌ 18 MFC files: `afxwin.h` not available with MinGW |
| `-DRMT_USE_QT=ON` | ❌ no Qt5 for MinGW installed |

MSVC builds: not tested here.
