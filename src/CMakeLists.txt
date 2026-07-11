# RMT Application Sources

# Platform and dependency detection
message(STATUS "Configuring RMT dependencies...")

# Check if MFC is available (MSVC only)
if(MSVC)
    message(STATUS "MSVC detected, MFC will be linked")
    set(MFC_AVAILABLE TRUE)
else()
    # MFC is Windows/MSVC only
    if(CMAKE_CROSSCOMPILING AND NOT MSVC)
        message(WARNING "Cross-compiling to Windows without MSVC - MFC will not be available")
        message(STATUS "For production builds, use MSVC on Windows or implement Phase 2 (wxWidgets)")
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
    # Full build with UI
    set(RMT_SOURCES
        # C++ Sources - Audio Backend (Phase 2)
        AudioBackend.cpp
        DirectSoundAudio.cpp
        PortAudioBackend.cpp
    
        # C++ Sources - MIDI Backend (Phase 2)
        MidiBackend.cpp
        WindowsMidiBackend.cpp
        RtMidiBackend.cpp
    
        # C++ Sources
        AboutDialog.cpp
    ASMFileBuilder.cpp
    ASMFile.cpp
    ASMFileExporter.cpp
    AssemblerTypes.cpp
    AtariBinaries.cpp
    Atari.cpp
    AtariIO.cpp
    AtariTrackerDriver.cpp
    C6502.cpp
    ChannelControl.cpp
    Clipboard.cpp
    ConfigDlg.cpp
    effectsdlg.cpp
    exportdlgs.cpp
    filenewdlg.cpp
    FilePathDlg.cpp
    Fraction.cpp
    Global.cpp
    GuiHelpers.cpp
    GUI_Instruments.cpp
    GUI_Song.cpp
    import.cpp
    importdlgs.cpp
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
    MainFrm.cpp
    Midi_Song.cpp
    Notes.cpp
    Pokey.cpp
    PokeyRederer.cpp
    PokeyStream.cpp
    RmtCommandLineInfo.cpp
    Rmt.cpp
    RmtDoc.cpp
    RmtMidi.cpp
    RmtTest.cpp
    RmtView.cpp
    RuntimeException.cpp
    SAPFile.cpp
    SAPFileExportDialog.cpp
    SAPFileExporter.cpp
    Shell.cpp
    SongContainer.cpp
    Song.cpp
    Song_DumpSong.cpp
    SongExport.cpp
    SongExporter.cpp
    SongExporterTest.cpp
    SongTimer.cpp
    StdAfx.cpp
    TracksControl.cpp
    Tracks.cpp
    Tuning.cpp
    TuningDlg.cpp
    TuningTypes.cpp
    Undo.cpp
    VUPlayer.cpp
    WaveFile.cpp
    WaveFileExporter.cpp
    
    # C Sources (embedded emulator/utilities)
    asap.c
    asap-patch.cpp
    astil.c
    info_dlg.c
    wasap.c
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

# Create executable
add_executable(Rmt WIN32 ${RMT_SOURCES})

# Set target properties
set_target_properties(Rmt PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Preprocessor definitions
target_compile_definitions(Rmt PRIVATE ${RMT_DEFINES})

# Include directories
target_include_directories(Rmt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

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

# Link PortAudio library (search was done earlier)
if(RMT_BUILD_CORE_ONLY OR NOT WIN32)
    if(PortAudio_FOUND)
        target_link_libraries(Rmt PRIVATE ${PortAudio_LIBRARIES})
        target_include_directories(Rmt PRIVATE ${PortAudio_INCLUDE_DIRS})
    elseif(PORTAUDIO_LIB)
        target_link_libraries(Rmt PRIVATE ${PORTAUDIO_LIB})
    else()
        message(WARNING "PortAudio not found - audio functionality may be limited")
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
    set_target_properties(Rmt PROPERTIES
        OUTPUT_NAME "Rmt.exe"
    )
endif()

message(STATUS "Rmt executable target configured")
message(STATUS "  Sources: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Defines: ${RMT_DEFINES}")

# Create separate MIDI test executable (core-only builds)
if(RMT_BUILD_CORE_ONLY)
    add_executable(RmtMidiTest WIN32
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
    
    # Apply same configuration as main Rmt executable
    set_target_properties(RmtMidiTest PROPERTIES
        VERSION ${PROJECT_VERSION}
        OUTPUT_NAME "RmtMidiTest.exe"
    )
    
    target_compile_definitions(RmtMidiTest PRIVATE ${RMT_DEFINES})
    target_include_directories(RmtMidiTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    
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

# Build status
message(STATUS "")
if(MFC_AVAILABLE)
    message(STATUS "✓ MFC Available - Full UI build")
else()
    message(STATUS "✗ MFC Not Available")
    if(CMAKE_CROSSCOMPILING)
        message(STATUS "  This is expected during cross-compilation (Phase 1)")
        message(STATUS "  Full builds require Windows+MSVC")
        message(STATUS "  Phase 2 will use wxWidgets for cross-platform support")
    endif()
endif()

