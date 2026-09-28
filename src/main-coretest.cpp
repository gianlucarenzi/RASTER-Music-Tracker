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
#include "resource.h"
#include "AtariTrackerDriver.h"

#include <cstdio>
#include <cstring>

extern CSong g_Song;
extern CAtari g_Atari;
extern TTuningSettings g_tuning;
extern TTuningRatios g_tuningRatios;

// --screenshot: draw the main screen like CRmtView::DrawAll() into the
// software CDC of MfcTypes.h and write it as a PPM image
static int Screenshot(const char* out, int w, int h, const char* song)
{
    CBitmap gfxBitmap;
    if (!gfxBitmap.LoadBitmap(MAKEINTRESOURCE(IDB_GFX))) {
        std::fprintf(stderr, "cannot load the IDB_GFX bitmap\n");
        return 1;
    }
    CDC gfxDC;
    gfxDC.CreateCompatibleDC(nullptr);
    gfxDC.SelectObject(&gfxBitmap);
    g_gfx_dc = &gfxDC;

    // CRmtView::Resize() at 100% scaling
    g_width = w;
    g_height = h;
    g_tracklines = (g_height - (CSongScreenLayout::TRACKS_Y + 3 * 16) - 40) / 16;
    g_line_y = g_tracklines / 2;

    CBitmap memBitmap;
    memBitmap.Create(w, h);
    CDC memDC;
    memDC.CreateCompatibleDC(nullptr);
    memDC.SelectObject(&memBitmap);
    CPen pen(PS_SOLID, 1, CRGBColor::LINES);
    memDC.SelectObject(&pen);
    g_mem_dc = &memDC;

    // CRmtApp::InitInstance(): tracker driver, empty song
    g_Atari.Init(g_Song.IsNTSC());
    g_AtariTrackerDriver = new CAtariTrackerDriver(g_Atari);
    g_AtariTrackerDriver->LoadRMTRoutines(g_trackerDriverVersion);
    g_AtariTrackerDriver->Init();
    g_Song.ClearSong(8);

    g_activepart = g_active_ti = Part::PART_TRACKS;
    if (song && !g_Song.FileOpen(song, FALSE)) {
        std::fprintf(stderr, "cannot open %s\n", song);
        return 1;
    }

    // CRmtView::DrawAll()
    g_Song.RespectBoundaries();
    memDC.FillSolidRect(0, 0, w, h, CRGBColor::BACKGROUND);
    g_Song.DrawInfo();
    g_Song.DrawSong();
    g_Song.DrawAnalyzer();
    g_Song.DrawPlayTimeCounter();
    if (g_active_ti == Part::PART_TRACKS) g_Song.DrawTracks();
    else g_Song.DrawInstrument();

    FILE* f = std::fopen(out, "wb");
    if (!f) { std::perror(out); return 1; }
    std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++) {
        uint32_t p = memBitmap.Bits()[i];
        unsigned char rgb[3] = { (unsigned char)(p >> 16), (unsigned char)(p >> 8), (unsigned char)p };
        std::fwrite(rgb, 1, 3, f);
    }
    std::fclose(f);
    std::printf("Screenshot %dx%d written to %s\n", w, h, out);
    return 0;
}

int main(int argc, char** argv) {
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

    if (argc >= 3 && !std::strcmp(argv[1], "--screenshot"))
        return Screenshot(argv[2], 1280, 800, argc >= 4 ? argv[3] : nullptr);

    std::printf("Song name: '%s'\n", g_Song.GetName().GetString());
    std::printf("g_Atari / g_tuning / g_tuningRatios initialised OK.\n");
    std::printf("RmtCoreTest finished successfully.\n");
    return 0;
}
