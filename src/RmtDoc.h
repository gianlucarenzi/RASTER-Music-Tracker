// RmtDoc.h : interface of the CRmtDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(RMT_RMTDOC_H)
#define RMT_RMTDOC_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRmtDoc : public CDocument {
protected: // create from serialization only
    CRmtDoc() {}
    DECLARE_DYNCREATE(CRmtDoc)

    // Attributes
public:
    // Operations
public:
    // Overrides
    // ClassWizard generated virtual function overrides
public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar){};

    // Implementation
public:
    virtual ~CRmtDoc() {}

protected:
    // Generated message map functions
protected:
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


#endif // !defined(RMT_RMTDOC_H)
