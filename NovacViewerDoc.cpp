// NovacViewerDoc.cpp : implementation of the CNovacViewerDoc class
//

#include "stdafx.h"
#include "NovacViewer.h"

#include "NovacViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNovacViewerDoc

IMPLEMENT_DYNCREATE(CNovacViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CNovacViewerDoc, CDocument)
END_MESSAGE_MAP()


// CNovacViewerDoc construction/destruction

CNovacViewerDoc::CNovacViewerDoc()
{
	// TODO: add one-time construction code here

}

CNovacViewerDoc::~CNovacViewerDoc()
{
}

BOOL CNovacViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CNovacViewerDoc serialization

void CNovacViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CNovacViewerDoc diagnostics

#ifdef _DEBUG
void CNovacViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNovacViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CNovacViewerDoc commands
