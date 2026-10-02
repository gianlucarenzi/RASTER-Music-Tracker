#include "StdAfx.h"
#include "RmtDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRmtDoc
// NOTE: Not used in RASTER Music Tracker

IMPLEMENT_DYNCREATE(CRmtDoc, CDocument)

// clang-format off
BEGIN_MESSAGE_MAP(CRmtDoc, CDocument)
	//{{AFX_MSG_MAP(CRmtDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
// clang-format on

BOOL CRmtDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    return TRUE;
}