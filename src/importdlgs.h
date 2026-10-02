#if !defined(RMT_IMPORTDLGS_H)
#define RMT_IMPORTDLGS_H

#include "resource.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// importdlgs.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImportModDlg dialog

class CImportModDlg : public CDialog {
    // Construction
public:
    CImportModDlg(CWnd* pParent = NULL); // standard constructor

    CString m_txtradio1, m_txtradio2;

    // Dialog Data
    enum { IDD = IDD_IMPORTMOD };
    CString m_info;
    BOOL m_check1;
    BOOL m_check2;
    BOOL m_check3;
    BOOL m_check4;
    BOOL m_check5;
    BOOL m_check6;
    BOOL m_check7;
    BOOL m_check8;


    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnCheck2();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CImportModFinishedDlg dialog

class CImportModFinishedDlg : public CDialog {
    // Construction
public:
    CImportModFinishedDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_IMPORTMODFINISHED };
    CStatic m_info2;
    CButton m_okbutt;
    CButton m_check1;
    CString m_info;


    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnCheck1();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CImportTmcDlg dialog

class CImportTmcDlg : public CDialog {
    // Construction
public:
    CImportTmcDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_IMPORTTMC };
    BOOL m_check1;
    BOOL m_check6;
    BOOL m_check7;
    CString m_info;


    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CImportTmcFinishedDlg dialog

class CImportTmcFinishedDlg : public CDialog {
    // Construction
public:
    CImportTmcFinishedDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_IMPORTTMCFINISHED };
    CStatic m_info2;
    CButton m_okbutt;
    CButton m_check1;
    CString m_info;


    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnCheck1();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CTracksLoadDlg dialog

class CTracksLoadDlg : public CDialog {
    // Construction
public:
    CTracksLoadDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_TRACKSLOAD };
    CStatic m_text1;

    int m_trackfrom;
    int m_tracknum;
    int m_radio;

    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};

#endif // !defined(RMT_IMPORTDLGS_H)
