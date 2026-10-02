#if !defined(RMT_EFFECTSDLG_H)
#define RMT_EFFECTSDLG_H


#include "resource.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EffectsDlg.h : header file

/////////////////////////////////////////////////////////////////////////////
// CEffectsDlg dialog

class CEffectsDlg : public CDialog {
    // Construction
public:
    CEffectsDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_EFFECTS };
    CStatic m_p3;
    CStatic m_p2;
    CStatic m_p1;
    CEdit m_edit3;
    CEdit m_edit2;
    CEdit m_edit1;
    CComboBox m_eff_combo;
    CString m_info;

    int m_effai;
    int m_bfro;
    int m_bto;
    int m_ainstr;
    int m_all;

    struct TTrack* m_trackptr;
    struct TTrack* m_trackorig;

    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation

private:
    void PerformEffect();

protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeEffCombo();
    virtual void OnOK();
    afx_msg void OnDefault();
    afx_msg void OnTry();
    afx_msg void OnRestore();
    virtual void OnCancel();
    afx_msg void OnPlaystop();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CSongTracksOrderDlg dialog

class CSongTracksOrderDlg : public CDialog {
    // Construction
public:
    CSongTracksOrderDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_SONGTRACKSORDER };
    CString m_songlinefrom;
    CString m_songlineto;

    int m_tracksorder[8];
    int m_fromtrack, m_totrack;

    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg void OnDefault();
    afx_msg void OnMonostereo();
    afx_msg void OnL1R4();
    afx_msg void OnStereomono();
    afx_msg void OnNothing();
    afx_msg void OnCopyleftright();
    afx_msg void OnCopyrightleft();
    afx_msg void OnClearall();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CInstrumentChangeDlg dialog

class CInstrumentChangeDlg : public CDialog {
    // Construction
public:
    CInstrumentChangeDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_INSTRCHANGE };
    CButton m_check6;
    CEdit m_edit2;
    CEdit m_edit1;
    CButton m_check5;
    CButton m_checkoneinstr;
    CStatic m_ctitle2;
    CStatic m_ctitle;
    CButton m_check4;
    CButton m_check3;
    CButton m_check2;
    CButton m_check1;
    int m_combo1;
    int m_combo2;
    int m_combo3;
    int m_combo4;
    int m_combo5;
    int m_combo6;
    int m_combo7;
    int m_combo8;
    int m_combo9;
    int m_combo10;
    int m_combo11;
    int m_combo12;

    int m_onlytrack;
    int m_onlychannels;
    int m_onlysonglinefrom, m_onlysonglineto;

    void SelChangeComboX();

    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnDefault();
    afx_msg void OnSelchangeComboX();
    afx_msg void OnSelchangeComboInstrs();
    afx_msg void OnSameNoteRanges();
    afx_msg void OnSameVolumeRange();
    afx_msg void OnSameInstrRange();
    virtual void OnOK();
    afx_msg void OnFullRanges();
    afx_msg void OnCheckoneinstrument();
    afx_msg void OnCheckSomeChannelsOnly();
    afx_msg void OnCheckTrackOnly();
    afx_msg void OnCheckSomeSonglinesOnly();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CRenumberTracksDlg dialog

class CRenumberTracksDlg : public CDialog {
    // Construction
public:
    CRenumberTracksDlg(CWnd* pParent = NULL); // standard constructor

    int m_radio;

    // Dialog Data
    enum { IDD = IDD_RENUMBERTRACKS };
    // NOTE: the ClassWizard will add data members here


    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CRenumberInstrumentsDlg dialog

class CRenumberInstrumentsDlg : public CDialog {
    // Construction
public:
    CRenumberInstrumentsDlg(CWnd* pParent = NULL); // standard constructor

    int m_radio;

    // Dialog Data
    enum { IDD = IDD_RENUMBERINSTRUMENTS };
    // NOTE: the ClassWizard will add data members here


    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CInsertCopyOrCloneOfSongLinesDlg dialog

class CInsertCopyOrCloneOfSongLinesDlg : public CDialog {
    // Construction
public:
    CInsertCopyOrCloneOfSongLinesDlg(CWnd* pParent = NULL); // standard constructor
    BOOL ValuesTest();

    // Dialog Data
    enum { IDD = IDD_SONGINSERTCOPYORCLONEOFSONGLINES };
    CStatic m_c_info;
    CStatic m_c_text2;
    CStatic m_c_text1;
    CEdit m_c_volumep;
    CEdit m_c_tuning;
    CEdit m_c_lineto;
    CEdit m_c_linefrom;
    CButton m_c_clone;

    int m_linefrom, m_lineto, m_lineinto;
    BOOL m_clone;
    int m_tuning, m_volumep;

    // Overrides
    // ClassWizard generated virtual function overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnClonetracks();
    afx_msg void OnChangeSonglinerange();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// COctaveSelectDlg dialog

class COctaveSelectDlg : public CDialog {
    // Construction
public:
    COctaveSelectDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_OCTAVESELECT };
    // NOTE: the ClassWizard will add data members here

    CPoint m_pos;
    int m_octave;

    // Overrides
    // ClassWizard generated virtual function overrides
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation

protected:
    // Generated message map functions
    virtual void OnOK();
    afx_msg void OnOctave();
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CInstrumentSelectDlg dialog

class CInstrumentSelectDlg : public CDialog {
    // Construction
public:
    CInstrumentSelectDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_INSTRUMENTSELECT };
    CListBox m_list1;

    CPoint m_pos;
    int m_selected;
    //class CInstruments* m_instrs;

    // Overrides
    // ClassWizard generated virtual function overrides
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeList1();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CVolumeSelectDlg dialog

class CVolumeSelectDlg : public CDialog {
    // Construction
public:
    CVolumeSelectDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_VOLUMESELECT };
    CListBox m_list1;
    BOOL m_respectvolume;

    CPoint m_pos;
    int m_volume;

    // Overrides
    // ClassWizard generated virtual function overrides
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    afx_msg void OnSelchangeList1();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CChannelsSelectionDlg dialog

class CChannelsSelectionDlg : public CDialog {
    // Construction
public:
    CChannelsSelectionDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    enum { IDD = IDD_CHANNELSSELECT };
    // NOTE: the ClassWizard will add data members here

    int m_channelyes;

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

#endif // !defined(RMT_EFFECTSDLG_H)
