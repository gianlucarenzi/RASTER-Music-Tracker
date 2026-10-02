// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(RMT_MAINFRM_H)
#define RMT_MAINFRM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMainFrame : public CFrameWnd {
protected: // create from serialization only
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

    // Attributes
public:
    // Operations
public:
    // Overrides
    // ClassWizard generated virtual function overrides
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    // Implementation
public:
    virtual ~CMainFrame();

    void OnSelChangedComboSkipLinesAfterNoteInsert();
    void OnRestoreFocusToMainWindow();

    CToolBar m_wndToolBar;

    CStatusBar m_wndStatusBar;
    CToolBar m_ToolBarPlay;
    CToolBar m_ToolBarChannels;
    CToolBar m_ToolBarBlock;
    CReBar m_wndReBar;

    CComboBox m_comboSkipLinesAfterNoteInsert;

protected: // control bar embedded members
           // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnClose();
    //afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


#endif // !defined(RMT_MAINFRM_H)
