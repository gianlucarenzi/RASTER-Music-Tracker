// RmtQtDialogs.h - the MFC dialogs of RMT, rewritten with Qt
//
// IRmtHost::DoModal (RmtQtFrontend.cpp) calls RmtQtRunDialog() for every
// CDialog::DoModal(); it shows the Qt version of the dialog selected by
// dlg->m_nIDTemplate, reads and writes the dialog's data members like the
// MFC DoDataExchange() and returns IDOK / IDCANCEL. Dialogs not rewritten
// yet are cancelled.

#pragma once

#include "StdAfx.h"

class QWidget;

INT_PTR RmtQtRunDialog(QWidget* parent, CDialog* dlg);
