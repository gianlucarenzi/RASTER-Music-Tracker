#include "StdAfx.h"
#include "RmtDoc.h"


/////////////////////////////////////////////////////////////////////////////
// CRmtDoc
// NOTE: Not used in RITMO

IMPLEMENT_DYNCREATE(CRmtDoc, CDocument)

// clang-format off
BEGIN_MESSAGE_MAP(CRmtDoc, CDocument)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
END_MESSAGE_MAP()
// clang-format on

BOOL CRmtDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    return TRUE;
}