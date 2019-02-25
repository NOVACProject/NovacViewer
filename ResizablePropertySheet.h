#pragma once



// CResizablePropertySheet

class CResizablePropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CResizablePropertySheet)

public:
	CResizablePropertySheet();
	CResizablePropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CResizablePropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CResizablePropertySheet();

protected:
	DECLARE_MESSAGE_MAP()
	
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
   BOOL   m_bNeedInit;
   CRect  m_rCrt;
   int    m_nMinCX;
   int    m_nMinCY;

};


