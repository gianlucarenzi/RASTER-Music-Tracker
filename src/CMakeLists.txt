# RMT Application Sources

# Platform and dependency detection
message(STATUS "Configuring RMT dependencies...")

# Bitmaps of res/ compiled into the builds without MFC (MfcTypes.h CBitmap)
include(${PROJECT_SOURCE_DIR}/cmake/EmbedResources.cmake)
set(RMT_EMBEDDED_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/RmtEmbeddedResources.cpp)
rmt_embed_resources(${RMT_EMBEDDED_RESOURCES}
    IDB_GFX             ${CMAKE_CURRENT_SOURCE_DIR}/res/asciagr.bmp
    IDR_MAINFRAME       ${CMAKE_CURRENT_SOURCE_DIR}/res/Toolbar.bmp
    IDR_TOOLBARPLAY     ${CMAKE_CURRENT_SOURCE_DIR}/res/toolbarp.bmp
    IDR_TOOLBARBLOCK    ${CMAKE_CURRENT_SOURCE_DIR}/res/toolbar1.bmp
    IDR_TOOLBARCHANNELS ${CMAKE_CURRENT_SOURCE_DIR}/res/bmp00001.bmp
)

# Check if MFC is available (MSVC only)
if(MSVC)
    message(STATUS "MSVC detected, MFC will be linked")
    set(MFC_AVAILABLE TRUE)
else()
    # MFC is Windows/MSVC only
    if(CMAKE_CROSSCOMPILING AND NOT MSVC)
        message(WARNING "Cross-compiling to Windows without MSVC - MFC will not be available")
        message(STATUS "For production builds, use MSVC on Windows or build the Qt5 frontend (-DRMT_USE_QT=ON)")
    elseif(WIN32)
        message(WARNING "Building on Windows without MSVC - MFC may not be available")
    else()
        message(STATUS "Non-Windows platform - MFC not required (Phase 2 will use cross-platform UI)")
    endif()
    set(MFC_AVAILABLE FALSE)
endif()

# Check for Windows audio libraries
if(WIN32)
    find_library(DSOUND_LIB NAMES dsound QUIET)
    find_library(WINMM_LIB NAMES winmm QUIET)
    
    if(DSOUND_LIB)
        message(STATUS "Found DirectSound: ${DSOUND_LIB}")
    else()
        message(WARNING "DirectSound not found - may affect audio")
    endif()
    
    if(WINMM_LIB)
        message(STATUS "Found WinMM: ${WINMM_LIB}")
    else()
        message(WARNING "WinMM not found - may affect MIDI")
    endif()
endif()

# Option to build test/core only (for Phase 1 cross-platform testing)
option(RMT_BUILD_CORE_ONLY "Build only core components without UI (for testing)" OFF)
if(RMT_BUILD_CORE_ONLY)
    message(STATUS "Building core components only (no UI)")
    
    set(RMT_SOURCES
        # Audio Backend (Phase 2) - cross-platform, no MFC
        AudioBackend.cpp
        DirectSoundAudio.cpp
        PortAudioBackend.cpp
        
        # MIDI Backend (Phase 2) - cross-platform, no MFC
        MidiBackend.cpp
        WindowsMidiBackend.cpp
        RtMidiBackend.cpp
        
        # Test main for audio/MIDI backends
        main-portaudio.cpp
    )
    
    set(RMT_HEADERS
        # Audio Backend (Phase 2)
        AudioBackend.h
        DirectSoundAudio.h
        PortAudioBackend.h
        
        # MIDI Backend (Phase 2)
        MidiBackend.h
        WindowsMidiBackend.h
        RtMidiBackend.h
    )
else()
    # Full build with UI.
    #
    # Sources are split by role (Fase 1 of the MFC -> Qt migration plan):
    #   - RMT_CORE_SOURCES:       the engine, no GUI dependency at all.
    #   - RMT_GUI_SHARED_SOURCES: "quasi-decoupled" rendering code that draws
    #                             on g_mem_dc/g_gfx_dc (CDC* under MFC,
    #                             CompatDC* under Qt once Fase 2 lands) -
    #                             compiled into BOTH the MFC and the Qt build.
    #   - RMT_GUI_MFC_SOURCES:    genuine CWinApp/CFrameWnd/CDocument/CView/
    #                             CDialog MFC code, plus RmtMidi.cpp (legacy
    #                             Win32 MIDI, not yet merged into MidiBackend -
    #                             see Fase 9) and the WASAP cluster used only
    #                             by RmtTest.cpp. Windows/MFC only.
    #   - RMT_GUI_QT_SOURCES:     the new Qt5 frontend. Empty until Fase 3+.
    set(RMT_CORE_SOURCES
        # Audio Backend (Phase 2) - cross-platform, no MFC
        AudioBackend.cpp
        DirectSoundAudio.cpp
        PortAudioBackend.cpp

        # MIDI Backend (Phase 2) - cross-platform, no MFC
        MidiBackend.cpp
        WindowsMidiBackend.cpp
        RtMidiBackend.cpp

        # Engine
        emu/Cpu6502.cpp
        emu/PokeySound.cpp
        ASMFileBuilder.cpp
        ASMFile.cpp
        ASMFileExporter.cpp
        AssemblerTypes.cpp
        AtariBinaries.cpp
        Atari.cpp
        AtariIO.cpp
        AtariTrackerDriver.cpp
        C6502.cpp
        Clipboard.cpp
        Fraction.cpp
        Instruments.cpp
        IOHelpers.cpp
        IO_Importer.cpp
        IO_Instruments.cpp
        IO_Song.cpp
        IO_Song_ExportAsm.cpp
        IO_Tracks.cpp
        Keyboard2NoteMapping.cpp
        LZSSFile.cpp
        lzss_sap.cpp
        Midi_Song.cpp
        Notes.cpp
        Pokey.cpp
        PokeyRederer.cpp
        PokeyStream.cpp
        RuntimeException.cpp
        SAPFile.cpp
        SAPFileExporter.cpp
        Shell.cpp
        SongContainer.cpp
        Song.cpp
        Song_DumpSong.cpp
        SongExport.cpp
        SongExporter.cpp
        SongExporterTest.cpp
        SongTimer.cpp
        Tracks.cpp
        Tuning.cpp
        TuningTypes.cpp
        Undo.cpp
        VUPlayer.cpp
        WaveFile.cpp
        WaveFileExporter.cpp
    )

    set(RMT_GUI_SHARED_SOURCES
        ChannelControl.cpp
        GuiHelpers.cpp
        GUI_Instruments.cpp
        GUI_Song.cpp
        TracksControl.cpp
        Global.cpp
    )

    set(RMT_GUI_MFC_SOURCES
        AboutDialog.cpp
        ConfigDlg.cpp
        effectsdlg.cpp
        exportdlgs.cpp
        filenewdlg.cpp
        FilePathDlg.cpp
        importdlgs.cpp
        MainFrm.cpp
        RmtCommandLineInfo.cpp
        Rmt.cpp
        RmtDoc.cpp
        RmtMidi.cpp
        RmtTest.cpp
        RmtView.cpp
        SAPFileExportDialog.cpp
        StdAfx.cpp
        TuningDlg.cpp

        # WASAP cluster: used only by RmtTest.cpp (raw WndProc/CreateDialog,
        # not just MFC types), no other engine file references it.
        asap.c
        asap-patch.cpp
        astil.c
        info_dlg.c
        wasap.c
    )

    # The Qt5 frontend: the MFC view/document (RmtView.cpp, RmtDoc.cpp)
    # compiled against MfcTypes.h, hosted by a Qt main window (qt/).
    set(RMT_GUI_QT_SOURCES
        qt/main-qt.cpp
        qt/RmtQtFrontend.cpp
        qt/RmtQtFrontend.h
        qt/RmtQtKeys.cpp
        qt/QtMainFrame.cpp
        RmtView.cpp
        RmtDoc.cpp
    )

    if(RMT_USE_QT)
        set(RMT_SOURCES ${RMT_CORE_SOURCES} ${RMT_GUI_SHARED_SOURCES} ${RMT_GUI_QT_SOURCES})
        if(NOT MFC_AVAILABLE)
            # Non-MFC type shim (Fase 0)
            list(APPEND RMT_SOURCES MfcTypes.cpp MfcAudio.cpp MfcDialogStubs.cpp ${RMT_EMBEDDED_RESOURCES})
        endif()
    else()
        set(RMT_SOURCES ${RMT_CORE_SOURCES} ${RMT_GUI_SHARED_SOURCES} ${RMT_GUI_MFC_SOURCES})
    endif()

set(RMT_HEADERS
    # Audio Backend (Phase 2)
    AudioBackend.h
    DirectSoundAudio.h
    PortAudioBackend.h
    
    # MIDI Backend (Phase 2)
    MidiBackend.h
    WindowsMidiBackend.h
    RtMidiBackend.h
    
    AboutDialog.h
    ASMFileBuilder.h
    ASMFile.h
    ASMFileExporter.h
    AssemblerTypes.h
    AtariBinaries.h
    Atari.h
    AtariIO.h
    AtariTrackerDriver.h
    C6502.h
    ChannelControl.h
    Clipboard.h
    ConfigDlg.h
    EffectsDlg.h
    ExportDlgs.h
    FileNewDlg.h
    FilePathDlg.h
    Fraction.h
    General.h
    Global.h
    GuiHelpers.h
    InstrumentTypes.h
    Instruments.h
    IOHelpers.h
    IO_Importer.h
    IO_Instruments.h
    IO_Song.h
    IO_Song_ExportAsm.h
    IO_Tracks.h
    Keyboard2NoteMapping.h
    LZSSFile.h
    lzssp.h
    lzss_sap.h
    MainFrm.h
    Memory.h
    Notes.h
    Pokey.h
    PokeyRederer.h
    PokeyStream.h
    Resource.h
    RmtCommandLineInfo.h
    Rmt.h
    RmtDoc.h
    RmtFile.h
    RmtMidi.h
    RmtTest.h
    RmtView.h
    RuntimeException.h
    SAPFile.h
    SAPFileExportDialog.h
    SAPFileExporter.h
    Shell.h
    SongContainer.h
    Song.h
    Song_DumpSong.h
    SongExport.h
    SongExporter.h
    SongExporterTest.h
    SongTimer.h
    SongTypes.h
    StdAfx.h
    TrackerDriverVersion.h
    tracker_obx.h
    TracksControl.h
    Tracks.h
    TracksTypes.h
    TrackTypes.h
    Tuning.h
    TuningDlg.h
    TuningTypes.h
    Undo.h
    VUPlayer.h
    WaveFile.h
    WaveFileExporter.h
    asap.h
    asap-patch.h
    astil.h
    info_dlg.h
    wasap.h
)
endif()

# Platform-specific preprocessor definitions
set(RMT_DEFINES
    NO_CONSOL_SOUND
    NO_VOL_ONLY
    STEREO
)

if(WIN32)
    list(APPEND RMT_DEFINES
        _WINDOWS
        WIN32
        _AFXDLL
    )
endif()

# Search for optional libraries BEFORE creating executable
# PortAudio cross-platform audio library
if(RMT_BUILD_CORE_ONLY OR NOT WIN32)
    find_package(PortAudio QUIET)
    if(NOT PortAudio_FOUND)
        find_library(PORTAUDIO_LIB portaudio QUIET)
    endif()
endif()

# RtMidi cross-platform MIDI library
if(RMT_BUILD_CORE_ONLY OR NOT WIN32)
    # Try pkg-config first
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(RTMIDI QUIET rtmidi)
    endif()

    # If pkg-config didn't find it, try manual search
    if(NOT RTMIDI_FOUND)
        find_library(RTMIDI_LIB rtmidi QUIET)
        find_path(RTMIDI_INCLUDE rtmidi.h QUIET)
    endif()
endif()

# Windows without PortAudio headers (e.g. MinGW cross-compile): drop the
# PortAudio backend, AudioBackendFactory falls back to DirectSound
set(RMT_NO_PORTAUDIO FALSE)
if(WIN32 AND NOT PortAudio_FOUND AND NOT PORTAUDIO_LIB)
    find_path(PORTAUDIO_INCLUDE portaudio.h)
    if(NOT PORTAUDIO_INCLUDE)
        set(RMT_NO_PORTAUDIO TRUE)
        list(REMOVE_ITEM RMT_SOURCES PortAudioBackend.cpp)
        list(APPEND RMT_DEFINES RMT_NO_PORTAUDIO)
        message(STATUS "PortAudio not found: Windows build uses DirectSound only")
    endif()
endif()

# Qt5 native GUI frontend (Fase 1+ of the MFC -> Qt migration)
if(RMT_USE_QT)
    find_package(Qt5 5.12 REQUIRED COMPONENTS Core Widgets)
endif()

# std::thread (song timer of MfcAudio.cpp, backends)
find_package(Threads REQUIRED)

# Create executable
add_executable(Rmt WIN32 ${RMT_SOURCES})
target_link_libraries(Rmt PRIVATE Threads::Threads)

# Set target properties
# VERSION/SOVERSION on an executable only make a versioned file plus a
# symlink (Rmt.exe-1.35.0 -> Rmt.exe): not wanted on Windows
if(NOT WIN32)
    set_target_properties(Rmt PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )
endif()

# Preprocessor definitions
target_compile_definitions(Rmt PRIVATE ${RMT_DEFINES})
if(MFC_AVAILABLE)
    # Tells PlatformTypes.h (included by the engine + GUI-shared sources) to
    # resolve to real <afxwin.h> instead of the non-MFC MfcTypes.h shim.
    # Without this, the MSVC/MFC build would silently take the wrong branch.
    target_compile_definitions(Rmt PRIVATE RMT_HAS_MFC)
endif()

# Include directories
target_include_directories(Rmt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

# Qt5 native GUI frontend
if(RMT_USE_QT)
    set_target_properties(Rmt PROPERTIES AUTOMOC ON AUTORCC ON AUTOUIC OFF)
    target_link_libraries(Rmt PRIVATE Qt5::Core Qt5::Widgets)
    target_compile_definitions(Rmt PRIVATE RMT_QT_GUI=1)
    target_include_directories(Rmt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qt)
endif()

# Windows-specific libraries
if(WIN32)
    # Link against Windows audio libraries if found
    if(DSOUND_LIB)
        target_link_libraries(Rmt PRIVATE dsound)
    endif()
    if(WINMM_LIB)
        target_link_libraries(Rmt PRIVATE winmm)
    endif()
    
    # If libraries not found but we're cross-compiling, still link by name
    # (MinGW cross-compiler should handle these)
    if(CMAKE_CROSSCOMPILING)
        if(NOT DSOUND_LIB)
            target_link_libraries(Rmt PRIVATE dsound)
        endif()
        if(NOT WINMM_LIB)
            target_link_libraries(Rmt PRIVATE winmm)
        endif()
    endif()
endif()

# Link PortAudio library (search was done earlier). Builds without MFC play
# the DirectSound buffer of the sound code with it (MfcAudio.cpp)
if(RMT_BUILD_CORE_ONLY OR NOT WIN32)
    if(PortAudio_FOUND)
        target_link_libraries(Rmt PRIVATE ${PortAudio_LIBRARIES})
        target_include_directories(Rmt PRIVATE ${PortAudio_INCLUDE_DIRS})
    elseif(PORTAUDIO_LIB)
        target_link_libraries(Rmt PRIVATE ${PORTAUDIO_LIB})
    else()
        message(WARNING "PortAudio not found - audio functionality may be limited")
    endif()
    if((PortAudio_FOUND OR PORTAUDIO_LIB) AND NOT MFC_AVAILABLE)
        target_compile_definitions(Rmt PRIVATE RMT_HAVE_PORTAUDIO)
    endif()
endif()

# Link RtMidi library (search was done earlier)
if(RMT_BUILD_CORE_ONLY OR NOT WIN32)
    if(RTMIDI_FOUND)
        target_link_libraries(Rmt PRIVATE ${RTMIDI_LIBRARIES})
        target_include_directories(Rmt PRIVATE ${RTMIDI_INCLUDE_DIRS})
        target_compile_definitions(Rmt PRIVATE RTMIDI_AVAILABLE=1)
        target_compile_options(Rmt PRIVATE ${RTMIDI_CFLAGS})
        message(STATUS "Found RtMidi via pkg-config: ${RTMIDI_LIBRARIES}")
    elseif(RTMIDI_LIB AND RTMIDI_INCLUDE)
        target_link_libraries(Rmt PRIVATE ${RTMIDI_LIB})
        target_include_directories(Rmt PRIVATE ${RTMIDI_INCLUDE})
        target_compile_definitions(Rmt PRIVATE RTMIDI_AVAILABLE=1)
        message(STATUS "Found RtMidi: ${RTMIDI_LIB}")
    else()
        message(WARNING "RtMidi not found - MIDI functionality will be disabled")
        message(STATUS "To install: sudo apt install librtmidi-dev")
    endif()
endif()

# Platform-specific compiler flags
if(MSVC)
    # Use multi-threaded runtime
    set_property(TARGET Rmt PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
    
    # Disable specific warnings if needed
    target_compile_options(Rmt PRIVATE /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang specific options
    target_compile_options(Rmt PRIVATE
        -Wall
        -Wextra
        -Wno-unknown-pragmas
        -Wno-unused-parameter
    )
endif()

# Output executable name
set_target_properties(Rmt PROPERTIES
    OUTPUT_NAME "Rmt"
)

if(NOT WIN32)
    if(RMT_USE_QT)
        set_target_properties(Rmt PROPERTIES OUTPUT_NAME "rmt")    # the Qt tracker
    else()
        set_target_properties(Rmt PROPERTIES OUTPUT_NAME "Rmt.exe")
    endif()
endif()

message(STATUS "Rmt executable target configured")
message(STATUS "  Sources: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Defines: ${RMT_DEFINES}")

# Create separate MIDI test executable (core-only builds)
if(RMT_BUILD_CORE_ONLY)
    set(RMT_MIDITEST_SOURCES
        main-midi.cpp
        # Audio Backend
        AudioBackend.cpp
        DirectSoundAudio.cpp
        PortAudioBackend.cpp
        # MIDI Backend
        MidiBackend.cpp
        WindowsMidiBackend.cpp
        RtMidiBackend.cpp
    )
    if(RMT_NO_PORTAUDIO)
        list(REMOVE_ITEM RMT_MIDITEST_SOURCES PortAudioBackend.cpp)
    endif()
    add_executable(RmtMidiTest WIN32 ${RMT_MIDITEST_SOURCES})
    target_link_libraries(RmtMidiTest PRIVATE Threads::Threads)

    # Apply same configuration as main Rmt executable
    if(WIN32)
        set_target_properties(RmtMidiTest PROPERTIES OUTPUT_NAME "RmtMidiTest")
    else()
        set_target_properties(RmtMidiTest PROPERTIES
            VERSION ${PROJECT_VERSION}
            OUTPUT_NAME "RmtMidiTest.exe"
        )
    endif()
    
    target_compile_definitions(RmtMidiTest PRIVATE ${RMT_DEFINES})
    if(WIN32)
        target_link_libraries(RmtMidiTest PRIVATE winmm dsound)
    endif()
    target_include_directories(RmtMidiTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR})
    
    # Link audio libraries
    if(PortAudio_FOUND)
        target_link_libraries(RmtMidiTest PRIVATE ${PortAudio_LIBRARIES})
        target_include_directories(RmtMidiTest PRIVATE ${PortAudio_INCLUDE_DIRS})
    elseif(PORTAUDIO_LIB)
        target_link_libraries(RmtMidiTest PRIVATE ${PORTAUDIO_LIB})
    endif()
    
    # Link MIDI libraries
    if(RTMIDI_FOUND)
        target_link_libraries(RmtMidiTest PRIVATE ${RTMIDI_LIBRARIES})
        target_include_directories(RmtMidiTest PRIVATE ${RTMIDI_INCLUDE_DIRS})
        target_compile_definitions(RmtMidiTest PRIVATE RTMIDI_AVAILABLE=1)
        target_compile_options(RmtMidiTest PRIVATE ${RTMIDI_CFLAGS})
    elseif(RTMIDI_LIB AND RTMIDI_INCLUDE)
        target_link_libraries(RmtMidiTest PRIVATE ${RTMIDI_LIB})
        target_include_directories(RmtMidiTest PRIVATE ${RTMIDI_INCLUDE})
        target_compile_definitions(RmtMidiTest PRIVATE RTMIDI_AVAILABLE=1)
    endif()
    
    # Compiler flags
    if(MSVC)
        set_property(TARGET RmtMidiTest PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
        target_compile_options(RmtMidiTest PRIVATE /permissive-)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(RmtMidiTest PRIVATE
            -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter
        )
    endif()
    
    message(STATUS "RmtMidiTest executable configured (MIDI backend test)")
endif()

# ---------------------------------------------------------------------------
# RmtCoreTest (Fase 0 of the MFC -> Qt migration plan)
#
# Compiles the *real* RMT engine (Song/Tracks/Instruments/C6502/Pokey/IO_*/
# ASM*/SAPFile*/...) plus the "GUI-shared" rendering files (GUI_Song.cpp,
# GUI_Instruments.cpp, GuiHelpers.cpp, TracksControl.cpp, ChannelControl.cpp,
# Global.cpp) outside of MFC, on plain GCC/Linux - something that has never
# been done before (previously RMT_BUILD_CORE_ONLY only compiled the new
# audio/MIDI backends, not the engine itself). It intentionally excludes the
# genuine MFC GUI files (Rmt.cpp, MainFrm.cpp, RmtDoc.cpp, RmtView.cpp,
# RmtCommandLineInfo.cpp, RmtMidi.cpp, StdAfx.cpp) and every real CDialog
# implementation (AboutDialog.cpp, ConfigDlg.cpp, effectsdlg.cpp,
# exportdlgs.cpp, filenewdlg.cpp, FilePathDlg.cpp, importdlgs.cpp,
# SAPFileExportDialog.cpp, TuningDlg.cpp) - those stay MFC-only, unmodified.
#
# Usage:
#   cmake -B build-core-linux -DRMT_BUILD_CORE_ONLY=ON -DRMT_CORE_TEST=ON
#   cmake --build build-core-linux
option(RMT_CORE_TEST "Build RmtCoreTest: compile the RMT engine + GUI-shared sources without MFC (Fase 0 smoke test)" OFF)
if(RMT_CORE_TEST)
    message(STATUS "Building RmtCoreTest (engine + GUI-shared sources, no MFC, no Qt yet)")

    set(RMT_ENGINE_SOURCES
        # ---- engine (no GUI) ----
        emu/Cpu6502.cpp
        emu/PokeySound.cpp
        AssemblerTypes.cpp
        AtariBinaries.cpp
        Atari.cpp
        AtariIO.cpp
        AtariTrackerDriver.cpp
        C6502.cpp
        Clipboard.cpp
        Fraction.cpp
        Instruments.cpp
        IOHelpers.cpp
        IO_Importer.cpp
        IO_Instruments.cpp
        IO_Song.cpp
        IO_Song_ExportAsm.cpp
        IO_Tracks.cpp
        Keyboard2NoteMapping.cpp
        LZSSFile.cpp
        lzss_sap.cpp
        Midi_Song.cpp
        Notes.cpp
        Pokey.cpp
        PokeyRederer.cpp
        PokeyStream.cpp
        RuntimeException.cpp
        SAPFile.cpp
        SAPFileExporter.cpp
        Shell.cpp
        SongContainer.cpp
        Song.cpp
        Song_DumpSong.cpp
        SongExport.cpp
        SongExporter.cpp
        SongExporterTest.cpp
        SongTimer.cpp
        Tracks.cpp
        Tuning.cpp
        TuningTypes.cpp
        Undo.cpp
        VUPlayer.cpp
        WaveFile.cpp
        WaveFileExporter.cpp
        ASMFileBuilder.cpp
        ASMFile.cpp
        ASMFileExporter.cpp

        # ---- "GUI-shared" rendering files (draw on g_mem_dc/g_gfx_dc) ----
        ChannelControl.cpp
        GuiHelpers.cpp
        GUI_Instruments.cpp
        GUI_Song.cpp
        TracksControl.cpp
        Global.cpp
    )

    if(NOT MFC_AVAILABLE)
        list(APPEND RMT_ENGINE_SOURCES ${RMT_EMBEDDED_RESOURCES})
        # MfcTypes.h/MfcTypes.cpp/MfcDialogStubs.cpp are the non-MFC type
        # shim (Fase 0): only meaningful - and only safe to compile - when
        # NOT building against real MFC (they define e.g. CString::LoadString()
        # and CEffectsDlg::CEffectsDlg(), which would clash with the real MFC
        # implementations otherwise).
        list(APPEND RMT_ENGINE_SOURCES
            MfcTypes.cpp
            MfcAudio.cpp
            MfcDialogStubs.cpp
        )
    endif()

    add_executable(RmtCoreTest ${RMT_ENGINE_SOURCES} main-coretest.cpp)
    target_link_libraries(RmtCoreTest PRIVATE Threads::Threads)

    target_compile_definitions(RmtCoreTest PRIVATE ${RMT_DEFINES})
    if(MFC_AVAILABLE)
        target_compile_definitions(RmtCoreTest PRIVATE RMT_HAS_MFC)
    endif()

    target_include_directories(RmtCoreTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR})

    if(RMT_BUILD_CORE_ONLY OR NOT WIN32)
        if(PortAudio_FOUND)
            target_link_libraries(RmtCoreTest PRIVATE ${PortAudio_LIBRARIES})
            target_include_directories(RmtCoreTest PRIVATE ${PortAudio_INCLUDE_DIRS})
        elseif(PORTAUDIO_LIB)
            target_link_libraries(RmtCoreTest PRIVATE ${PORTAUDIO_LIB})
        endif()
    endif()

    if(MSVC)
        set_property(TARGET RmtCoreTest PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
        target_compile_options(RmtCoreTest PRIVATE /permissive-)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(RmtCoreTest PRIVATE
            -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter
        )
    endif()

    message(STATUS "RmtCoreTest executable configured (engine + GUI-shared, Fase 0 smoke test)")
endif()

# Build status
message(STATUS "")
if(MFC_AVAILABLE)
    message(STATUS "✓ MFC Available - Full UI build")
else()
    message(STATUS "✗ MFC Not Available")
    if(CMAKE_CROSSCOMPILING)
        message(STATUS "  This is expected during cross-compilation (Phase 1)")
        message(STATUS "  Full builds require Windows+MSVC")
        message(STATUS "  Or build the Qt5 frontend: cmake -DRMT_USE_QT=ON")
    endif()
endif()

