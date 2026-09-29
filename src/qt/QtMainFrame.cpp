// QtMainFrame.cpp - CMainFrame outside MFC
//
// MainFrm.cpp builds the MFC toolbars, rebar and status bar from the .rc
// resources and cannot be compiled without MFC. In the Qt frontend the frame
// is RmtMainWindow (RmtQtFrontend.cpp); these are the CMainFrame members the
// rest of the GUI code references.

#include "StdAfx.h"
#include "MainFrm.h"

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
END_MESSAGE_MAP()

CMainFrame::CMainFrame() {}

CMainFrame::~CMainFrame() {}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT&) { return TRUE; }

int CMainFrame::OnCreate(LPCREATESTRUCT) { return 0; }

void CMainFrame::OnClose() {}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO*) {}

void CMainFrame::OnSelChangedComboSkipLinesAfterNoteInsert() {}

void CMainFrame::OnRestoreFocusToMainWindow() {}
