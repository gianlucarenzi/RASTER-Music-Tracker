#if !defined(RMT_FILENEWDLG_H)
#define RMT_FILENEWDLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileNewDlg.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFileNewDlg dialog

class CFileNewDlg : public CDialog {
    // Construction
public:
    CFileNewDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_FILENEW };
    int m_maxTrackLength;    // How many notes/beats per track 1 - 256
    int m_comboMonoOrStereo; // 0 = mono, 1 = stereo


    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual void OnOK();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CChangeMaxtracklenDlg dialog

class CChangeMaxtracklenDlg : public CDialog {
    // Construction
public:
    CChangeMaxtracklenDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_CHANGEMAXTRACKLEN };
    CString m_info;
    int m_maxtracklen;


    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    // NOTE: the ClassWizard will add member functions here
    DECLARE_MESSAGE_MAP()
};

#endif // !defined(RMT_FILENEWDLG_H)
