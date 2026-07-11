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
endif()

set(RMT_SOURCES
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

