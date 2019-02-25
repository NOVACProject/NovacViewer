#pragma once
#include "afxdtctl.h"


// CChangeSelectedDateDlg dialog

class CChangeSelectedDateDlg : public CDialog
{
	DECLARE_DYNAMIC(CChangeSelectedDateDlg)

public:
	CChangeSelectedDateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChangeSelectedDateDlg();

// Dialog Data
	enum { IDD = IDD_CHANGE_DATE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChange();
	CMonthCalCtrl m_calendarCtrl;
	virtual BOOL OnInitDialog();
};
