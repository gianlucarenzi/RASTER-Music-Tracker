#if !defined(AFX_CONFIGDLG_H__71CF2041_5206_11D7_BEB0_00600854AFCA__INCLUDED_)
#define AFX_CONFIGDLG_H__71CF2041_5206_11D7_BEB0_00600854AFCA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "resource.h"

#include "General.h"
#include "TrackerDriverVersion.h"

extern CString g_defaultSongsPath;       // Default path for songs
extern CString g_defaultInstrumentsPath; // Default path for instruments
extern CString g_defaultTracksPath;      // Default path for tracks

extern CString g_lastLoadPath_Songs;
extern CString g_lastLoadPath_Instruments;
extern CString g_lastLoadPath_Tracks;

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog dialog

class COptionsDialog : public CDialog {
    // Construction
public:
    COptionsDialog(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    //{{AFX_DATA(COptionsDialog)
    enum { IDD = IDD_CONFIG };
    CComboBox m_keyboard_c_layout;
    CComboBox m_midi_c_device;
    CComboBox m_trackerDriver_c_Version;
    BOOL m_midi_TouchResponse;
    int m_midi_VolumeOffset;
    int m_trackLinePrimaryHighlight;
    int m_trackLineSecondaryHighlight;
    int m_scaling_percentage;
    TrackerDriverVersion m_trackerDriverVersion;
    BOOL m_ntsc;
    BOOL m_doSmoothScrolling;
    BOOL m_displayflatnotes;
    BOOL m_usegermannotation;
    BOOL m_midi_NoteOff;
    BOOL m_keyboard_updowncontinue;
    BOOL m_nohwsoundbuffer;
    BOOL m_tracklinealtnumbering;
    BOOL m_keyboard_rememberoctavesandvolumes;
    BOOL m_keyboard_escresetatarisound;
    BOOL m_keyboard_askwhencontrol_s;
    BOOL m_viewDebugDisplay;
    //}}AFX_DATA

    int m_midi_device;
    KeyboardLayout m_keyboard_layout;

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COptionsDialog)
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
                                                     //}}AFX_VIRTUAL

    // Implementation
protected:
    // Generated message map functions
    //{{AFX_MSG(COptionsDialog)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnMidiTouchResponseClicked();
    afx_msg void OnPaths();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    //afx_msg void OnBnClickedDisplayflatnotes();
};

/////////////////////////////////////////////////////////////////////////////
// COptionsPathsDlg dialog

class COptionsPathsDlg : public CDialog {
    // Construction
public:
    COptionsPathsDlg(CWnd* pParent = NULL); // standard constructor

    void BrowsePath(int itemID);

    // Dialog Data
    //{{AFX_DATA(COptionsPathsDlg)
    enum { IDD = IDD_PATHS };
    CString m_path_songs;
    CString m_path_instruments;
    CString m_path_tracks;
    //}}AFX_DATA


    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COptionsPathsDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
                                                     //}}AFX_VIRTUAL

    // Implementation
protected:
    // Generated message map functions
    //{{AFX_MSG(COptionsPathsDlg)
    afx_msg void OnButton1();
    afx_msg void OnButton2();
    afx_msg void OnButton3();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG_H__71CF2041_5206_11D7_BEB0_00600854AFCA__INCLUDED_)
