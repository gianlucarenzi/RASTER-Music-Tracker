// main-coretest.cpp
//
// Smoke test for the "RmtCoreTest" target (Fase 0 of the MFC -> Qt
// migration plan): links the whole RMT engine (Song/Tracks/Instruments/
// C6502/Pokey/IO_*/ASM*/SAPFile*/GUI_Song/GUI_Instruments/GuiHelpers/
// Global/TracksControl/ChannelControl/...) outside of MFC, on plain GCC.
//
// This intentionally does not exercise any UI - there is none yet, that's
// a later phase of the plan. It only needs to prove that the engine
// initialises its core global state without crashing.

#include "PlatformTypes.h"
#include "Global.h"
#include "Song.h"
#include "Tracks.h"
#include "Instruments.h"
#include "Tuning.h"

#include <cstdio>

extern CSong g_Song;
extern CAtari g_Atari;
extern TTuningSettings g_tuning;
extern TTuningRatios g_tuningRatios;

int main() {
    std::printf("RmtCoreTest - RASTER Music Tracker engine (non-MFC build)\n");
    std::printf("Version string: %s\n", g_app.GetVersionAndBuild().GetString());

    // Mirror the harmless, non-GUI part of CRmtApp::InitInstance() (Rmt.cpp):
    // detect/initialise the 6502 emulation "DLL" (gracefully reports "not
    // found" here, exactly as it would on a real machine missing
    // sa_c6502.dll - see C6502::Init()) and the tuning tables.
    //
    // Intentionally NOT calling CAtariTrackerDriver::Init()/Play() or
    // CTracks::InitTracks()/CInstruments::InitInstruments() here: those
    // ultimately call CAtari::JSR() -> the native 6502 emulator DLL, a real
    // *runtime hardware/DLL dependency* of the engine (present on every real
    // Windows install alongside Rmt.exe) that is simply out of scope for
    // Fase 0 (MFC/type portability) - not something a Linux build can ever
    // satisfy without a from-scratch 6502 emulator, and orthogonal to
    // whether the engine *compiles and links* outside of MFC.
    g_Atari.Init();
    g_tuning.Initialize(g_Song.IsNTSC());
    g_tuningRatios.Initialize();

    std::printf("Song name: '%s'\n", g_Song.GetName().GetString());
    std::printf("g_Atari / g_tuning / g_tuningRatios initialised OK.\n");
    std::printf("RmtCoreTest finished successfully.\n");
    return 0;
}
