// MfcDialogStubs.cpp
//
// The RMT engine is not as cleanly separated from the MFC dialog layer as
// one might hope: a handful of engine files (Song.cpp, IO_Song.cpp,
// IO_Song_ExportAsm.cpp, IO_Importer.cpp, GUI_Song.cpp, Clipboard.cpp,
// SongExporter.cpp, ASMFileExporter.cpp) directly instantiate real MFC
// CDialog-derived classes (declared in EffectsDlg.h / filenewdlg.h /
// importdlgs.h / exportdlgs.h) and call DoModal() on them. Those classes'
// constructors and virtual overrides are normally defined in the
// corresponding real-MFC-only *.cpp files (effectsdlg.cpp, filenewdlg.cpp,
// importdlgs.cpp, exportdlgs.cpp) which are NOT part of this build (they
// are genuine CDialog implementations, out of scope for Fase 0).
//
// This file provides the minimum set of out-of-line definitions
// (constructor + declared virtuals) needed to link those classes into the
// RmtCoreTest binary, exactly mirroring the "always cancelled" behaviour
// CDialog::DoModal() already has in MfcTypes.h. None of this is reachable
// from main-coretest.cpp; it only needs to exist for the link to succeed.
// It intentionally duplicates none of the real dialogs' business logic.
//
// Only compiled when building without real MFC.

#include "PlatformTypes.h"

#include "SongTypes.h"
#include "Song.h"

#include "EffectsDlg.h"
#include "filenewdlg.h"
#include "importdlgs.h"
#include "exportdlgs.h"
#include "SAPFileExportDialog.h"
#include "RmtMidi.h"

// ---------------------------------------------------------------------------
// EffectsDlg.h
// ---------------------------------------------------------------------------

CEffectsDlg::CEffectsDlg(CWnd*) {}
void CEffectsDlg::DoDataExchange(CDataExchange*) {}
BOOL CEffectsDlg::OnInitDialog() { return TRUE; }
void CEffectsDlg::OnOK() {}
void CEffectsDlg::OnCancel() {}

COctaveSelectDlg::COctaveSelectDlg(CWnd*) {}
BOOL COctaveSelectDlg::PreTranslateMessage(MSG*) { return FALSE; }
void COctaveSelectDlg::DoDataExchange(CDataExchange*) {}
void COctaveSelectDlg::OnOK() {}
BOOL COctaveSelectDlg::OnInitDialog() { return TRUE; }

CVolumeSelectDlg::CVolumeSelectDlg(CWnd*) {}
BOOL CVolumeSelectDlg::PreTranslateMessage(MSG*) { return FALSE; }
void CVolumeSelectDlg::DoDataExchange(CDataExchange*) {}
BOOL CVolumeSelectDlg::OnInitDialog() { return TRUE; }
void CVolumeSelectDlg::OnOK() {}

CInstrumentSelectDlg::CInstrumentSelectDlg(CWnd*) {}
BOOL CInstrumentSelectDlg::PreTranslateMessage(MSG*) { return FALSE; }
void CInstrumentSelectDlg::DoDataExchange(CDataExchange*) {}
BOOL CInstrumentSelectDlg::OnInitDialog() { return TRUE; }

CSongTracksOrderDlg::CSongTracksOrderDlg(CWnd*) {}
void CSongTracksOrderDlg::DoDataExchange(CDataExchange*) {}
BOOL CSongTracksOrderDlg::OnInitDialog() { return TRUE; }

CInstrumentChangeDlg::CInstrumentChangeDlg(CWnd*) {}
void CInstrumentChangeDlg::DoDataExchange(CDataExchange*) {}
BOOL CInstrumentChangeDlg::OnInitDialog() { return TRUE; }
void CInstrumentChangeDlg::OnOK() {}

CInsertCopyOrCloneOfSongLinesDlg::CInsertCopyOrCloneOfSongLinesDlg(CWnd*) {}
void CInsertCopyOrCloneOfSongLinesDlg::DoDataExchange(CDataExchange*) {}
BOOL CInsertCopyOrCloneOfSongLinesDlg::OnInitDialog() { return TRUE; }
void CInsertCopyOrCloneOfSongLinesDlg::OnOK() {}

// ---------------------------------------------------------------------------
// filenewdlg.h
// ---------------------------------------------------------------------------

CFileNewDlg::CFileNewDlg(CWnd*) {}
void CFileNewDlg::DoDataExchange(CDataExchange*) {}
void CFileNewDlg::OnOK() {}

// ---------------------------------------------------------------------------
// importdlgs.h
// ---------------------------------------------------------------------------

CImportModDlg::CImportModDlg(CWnd*) {}
void CImportModDlg::DoDataExchange(CDataExchange*) {}
BOOL CImportModDlg::OnInitDialog() { return TRUE; }
void CImportModDlg::OnOK() {}

CImportModFinishedDlg::CImportModFinishedDlg(CWnd*) {}
void CImportModFinishedDlg::DoDataExchange(CDataExchange*) {}
BOOL CImportModFinishedDlg::OnInitDialog() { return TRUE; }

CImportTmcDlg::CImportTmcDlg(CWnd*) {}
void CImportTmcDlg::DoDataExchange(CDataExchange*) {}
BOOL CImportTmcDlg::OnInitDialog() { return TRUE; }

CImportTmcFinishedDlg::CImportTmcFinishedDlg(CWnd*) {}
void CImportTmcFinishedDlg::DoDataExchange(CDataExchange*) {}
BOOL CImportTmcFinishedDlg::OnInitDialog() { return TRUE; }

CTracksLoadDlg::CTracksLoadDlg(CWnd*) {}
void CTracksLoadDlg::DoDataExchange(CDataExchange*) {}
void CTracksLoadDlg::OnOK() {}
BOOL CTracksLoadDlg::OnInitDialog() { return TRUE; }

// ---------------------------------------------------------------------------
// exportdlgs.h
// ---------------------------------------------------------------------------

CExportStrippedRMTDialog::CExportStrippedRMTDialog(CWnd*) {}
void CExportStrippedRMTDialog::DoDataExchange(CDataExchange*) {}
BOOL CExportStrippedRMTDialog::OnInitDialog() { return TRUE; }

CExpMSXDlg::CExpMSXDlg(CWnd*) {}
void CExpMSXDlg::DoDataExchange(CDataExchange*) {}
BOOL CExpMSXDlg::OnInitDialog() { return TRUE; }
void CExpMSXDlg::OnOK() {}

CExportAsmDlg::CExportAsmDlg(CWnd*) {}
void CExportAsmDlg::DoDataExchange(CDataExchange*) {}
void CExportAsmDlg::OnOK() {}
BOOL CExportAsmDlg::OnInitDialog() { return TRUE; }

CExportRelocatableAsmForRmtPlayer::CExportRelocatableAsmForRmtPlayer(CWnd*) {}
void CExportRelocatableAsmForRmtPlayer::DoDataExchange(CDataExchange*) {}
BOOL CExportRelocatableAsmForRmtPlayer::OnInitDialog() { return TRUE; }

// ---------------------------------------------------------------------------
// SAPFileExportDialog.h
//
// Only ever reached through the static Show() helper (SongExporter.cpp);
// our stub never actually constructs a CSAPFileExportDialog, so it always
// reports "user cancelled" - exactly like every other stubbed dialog here.
// ---------------------------------------------------------------------------

bool CSAPFileExportDialog::Show(const CSong&, CSAPFile&) {
    return false;
}

// ---------------------------------------------------------------------------
// RmtMidi.h
//
// Global.cpp defines a real global 'CRmtMidi g_Midi;' instance, so its
// constructor/destructor must link. Full MIDI I/O behaviour is out of scope
// for Fase 0 (see migration plan Fase 9 - RmtMidi legacy -> MidiBackend);
// MidiInit()/MidiOn()/MidiOff()/MidiRestart() are declared but never called
// from any file compiled in this target, so they are intentionally left
// undefined here.
// ---------------------------------------------------------------------------

CRmtMidi::CRmtMidi() {
    m_MidiIsOn = FALSE;
    m_MidiInHandle = nullptr;
    m_MidiInDeviceName[0] = '\0';
    m_MidiInDeviceId = 0;
    m_TouchResponse = FALSE;
    m_VolumeOffset = 0;
    m_NoteOff = FALSE;
    for (int i = 0; i < 16; i++) {
        m_LastNoteOnChannel[i] = -1;
        m_NoteVolumeOnChannel[i] = 0;
        m_InstrumentOnChannel[i] = 0;
    }
}

CRmtMidi::~CRmtMidi() {}
