// MfcTypes.cpp
//
// Out-of-line pieces of the MfcTypes.h shim: only compiled when building
// without real MFC (see PlatformTypes.h / RmtCoreTest CMake target).

#include "PlatformTypes.h"
#include "resource.h"

// ---------------------------------------------------------------------------
// CString::LoadString() - stand-in for the Windows .rc string table.
// Only the resource IDs actually reached via LoadString() calls in the
// engine + GUI-shared source files are listed here (verified by grep):
//   IDS_RMTVERSION / IDS_RMT_VERSION (IO_Song.cpp)
// The other two (IDS_RMT_AUTHOR, IDS_RMT_REPOSITORY) are only used from
// AboutDialog.cpp, a real MFC dialog not part of this build, but are kept
// here too since they cost nothing and keep the table self-documenting.
// Text taken verbatim from Rmt.rc.
// ---------------------------------------------------------------------------

BOOL CString::LoadString(UINT id) {
    switch (id) {
    case IDS_RMT_VERSION: // == IDS_RMTVERSION
        m_data = "RASTER Music Tracker 1.35";
        return TRUE;
    case IDS_RMT_AUTHOR:
        m_data = "by Radek Sterba, (c) Raster/C.P.U. (2002-2009), VinsCool (2021-2024), JAC! (2024-2026)";
        return TRUE;
    case IDS_RMT_REPOSITORY:
        m_data = "https://github.com/raster-atari-org/RASTER-Music-Tracker";
        return TRUE;
    default:
        m_data = "";
        return FALSE;
    }
}

// ---------------------------------------------------------------------------
// CRmtApp - tiny non-MFC stand-in (real CRmtApp lives in Rmt.h/Rmt.cpp,
// which are real-MFC-only). Mirrors CRmtApp::GetVersionAndBuild() from
// Rmt.cpp so the version string displayed by GUI_Song.cpp is identical.
// ---------------------------------------------------------------------------

CRmtApp g_app;

CString CRmtApp::GetVersionAndBuild() const {
    CString version;
    version.LoadString(IDS_RMTVERSION);
    CString result;
    result.Format("%s (%s %s)", version, __DATE__, __TIME__);
    return result;
}
