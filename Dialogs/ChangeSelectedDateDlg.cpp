// ChangeSelectedDateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacViewer.h"
#include "ChangeSelectedDateDlg.h"
#include "../Common/DateTime.h"
#include "../DataImporter.h"
#include "../Common/Common.h"

extern CDateTime			g_selectedDate;		   // <-- The date we're looking at
extern CDataImporter		*g_dataThread;

// CChangeSelectedDateDlg dialog

IMPLEMENT_DYNAMIC(CChangeSelectedDateDlg, CDialog)
CChangeSelectedDateDlg::CChangeSelectedDateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChangeSelectedDateDlg::IDD, pParent)
{
}

CChangeSelectedDateDlg::~CChangeSelectedDateDlg()
{
}

void CChangeSelectedDateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MONTHCALENDAR1, m_calendarCtrl);
}


BEGIN_MESSAGE_MAP(CChangeSelectedDateDlg, CDialog)
	ON_BN_CLICKED(IDOK,		OnBnClickedChange)
END_MESSAGE_MAP()


// CChangeSelectedDateDlg message handlers

void CChangeSelectedDateDlg::OnBnClickedChange()
{
	SYSTEMTIME dateTime;

	m_calendarCtrl.GetCurSel(&dateTime);

	g_selectedDate.year = dateTime.wYear;
	g_selectedDate.month = dateTime.wMonth;
	g_selectedDate.day = dateTime.wDay;

	g_dataThread->PostThreadMessage(WM_CHANGED_DATE, NULL, NULL);
}

BOOL CChangeSelectedDateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// move the window to the upper right hand corner of the screen
	CRect windowRect;
	GetWindowRect(windowRect);
	int windowheight	= windowRect.Height();
	int windowwidth		= windowRect.Width();
	int screenWidth		= GetSystemMetrics(SM_CXSCREEN);
	int screenHeight	= GetSystemMetrics(SM_CYSCREEN);
	windowRect.right	= screenWidth; 
	windowRect.left		= screenWidth - windowwidth;
	windowRect.bottom	= screenHeight - 50;
	windowRect.top		= windowRect.bottom - windowheight;
	MoveWindow(windowRect);

	// show the correct time
	SYSTEMTIME dateTime;
	dateTime.wYear = g_selectedDate.year;
	dateTime.wMonth= g_selectedDate.month;
	dateTime.wDay  = g_selectedDate.day;

	m_calendarCtrl.SetCurSel(&dateTime);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
