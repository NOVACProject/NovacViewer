// NovacViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "NovacViewer.h"
#include "MainFrm.h"

#include "NovacViewerDoc.h"
#include "NovacViewerView.h"

#include "DataImporter.h"

#include "Dialogs/SelectionDialog.h"
#include "VolcanoInfo.h"

extern CVolcanoInfo g_volcanoes;
CDataImporter		*g_dataThread;
int					g_volcano;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNovacViewerApp

BEGIN_MESSAGE_MAP(CNovacViewerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
//	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
//	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CNovacViewerApp construction

CNovacViewerApp::CNovacViewerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CNovacViewerApp object

CNovacViewerApp theApp;

// CNovacViewerApp initialization

BOOL CNovacViewerApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CNovacViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CNovacViewerView));
	AddDocTemplate(pDocTemplate);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
				
	// --------------- Initialize the program ---------------
	unsigned int k;

	// First we need to know which volcano we should look at...
	Dialogs::CSelectionDialog volcanoDialog;
	CString volcano;
	for(k = 0; k < g_volcanoes.m_volcanoNum; ++k)
		volcanoDialog.m_option[k].Format(g_volcanoes.m_name[k]);
	volcanoDialog.m_windowText.Format("Select a volcano to view.");
	volcanoDialog.m_currentSelection = &volcano;
	INT_PTR ret = volcanoDialog.DoModal();
	
	// find the index of the volcano
	for(k = 0; k < g_volcanoes.m_volcanoNum; ++k){
		if(Equals(g_volcanoes.m_name[k], volcano)){
			g_volcano = k;
			break;
		}
	}
	

	if(IDCANCEL == ret) // if the user clicked 'cancel' then don't insert anything
		return TRUE;

	// Start the data-importing thread
	g_dataThread = (CDataImporter *)AfxBeginThread(RUNTIME_CLASS(CDataImporter), THREAD_PRIORITY_BELOW_NORMAL, 0, 0, NULL);

	while(!g_dataThread->m_fInitialized){
		Sleep(1000);
	}

	// --------------- Now we can start the interface -------------------------		

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
			
	// The one and only window has been initialized, so show and update it
	
	// Get the resolution of the screen
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	if(cx <= 1024){
		m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	}else{
		int width = cx;
		int height = 750;
		m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
		m_pMainWnd->MoveWindow((cx-width)/2, (cy-height)/2, width, height);
	}
	m_pMainWnd->UpdateWindow();


	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CNovacViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CNovacViewerApp message handlers


BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString version, copyRight;

	version.Format("Version %d.%02d . Built: %s", CVersion::majorNumber, CVersion::minorNumber, __DATE__);
	SetDlgItemText(IDC_LABEL_VERSION, version);
	
	copyRight.Format("Copyright (C) 2008 - %d Optical Remote Sensing Group", Common::GetYear());
	SetDlgItemText(IDC_LABEL_COPYRIGHT, copyRight);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
