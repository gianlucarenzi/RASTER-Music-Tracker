// RmtQtDialogs.h - the dialogs of RITMO, in Qt
//
// IRmtHost::DoModal (RmtQtFrontend.cpp) calls RmtQtRunDialog() for every
// CDialog::DoModal(); it shows the Qt version of the dialog selected by
// dlg->m_nIDTemplate, reads and writes the dialog's data members (the
// DoDataExchange() of the dialog classes) and returns IDOK / IDCANCEL.
// Dialogs not written yet are cancelled.

#pragma once

#include "StdAfx.h"

class QWidget;

INT_PTR RmtQtRunDialog(QWidget* parent, CDialog* dlg);
