// NovacViewerDoc.h : interface of the CNovacViewerDoc class
//


#pragma once

class CNovacViewerDoc : public CDocument
{
protected: // create from serialization only
	CNovacViewerDoc();
	DECLARE_DYNCREATE(CNovacViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CNovacViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


