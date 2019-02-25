// NovacViewerView.cpp : implementation of the CNovacViewerView class
//

#include "stdafx.h"
#include "NovacViewer.h"

#include "NovacViewerDoc.h"
#include "NovacViewerView.h"

#include "UserSettings.h"

#include "View_Scanner.h"

#include "Common/ReportWriter.h"
#include "Common/FluxLogFileHandler.h"
#include "Communication/LinkStatistics.h"

#include "Evaluation/ScanResult.h"

#include "Geometry/GeometryResult.h"

#include "Configuration/ConfigurationFileHandler.h"

#include "PostFlux/PostFluxDlg.h"

#include "Dialogs/ExportDlg.h"
#include "Dialogs/ExportSpectraDlg.h"
#include "Dialogs/ExportEvallogDlg.h"
#include "Dialogs/ImportSpectraDlg.h"
#include "Dialogs/FileTransferDlg.h"
#include "Dialogs/GeometryDlg.h"
#include "Dialogs/SplitPakFilesDlg.h"
#include "Dialogs/MergePakFilesDlg.h"
#include "Dialogs/DataBrowserDlg.h"
#include "Dialogs/PakFileInspector.h"
#include "Dialogs/SummarizeFluxDataDlg.h"

#include "WindMeasurement/PostWindDlg.h"
#include "WindMeasurement/WindSpeedResult.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CMeteorologicalData		g_metData;
extern CConfigurationSetting	g_settings;
extern CUserSettings			g_userSettings;

UINT primaryLanguage;
UINT subLanguage;

CFormView* pView;

/** This critical section tries to make sure that only one
      thread at a time tries to read from one of the small 
      evaluation-log - files */
CCriticalSection g_evalLogCritSect;


// CNovacViewerView

IMPLEMENT_DYNCREATE(CNovacViewerView, CFormView)

BEGIN_MESSAGE_MAP(CNovacViewerView, CFormView)
	ON_MESSAGE(WM_SHOW_MESSAGE,					OnShowMessage)
	ON_MESSAGE(WM_EVAL_SUCCESS,					OnEvalSucess)
	ON_MESSAGE(WM_CHANGED_DATE,					OnChangedDate)

	// Commands from the user
	ON_COMMAND(ID_ANALYSIS_FLUX,						OnMenuAnalysisFlux)
	ON_COMMAND(ID_CONFIGURATION_CONFIGURATION,			OnMenuShowConfigurationDialog)
	ON_COMMAND(ID_SET_LANGUAGE_ENGLISH,					OnMenuSetLanguageEnglish)
	ON_COMMAND(ID_SET_LANGUAGE_ESPA,					OnMenuSetLanguageSpanish)
	ON_COMMAND(ID_ANALYSIS_WIND,						OnMenuAnalysisWind)
	ON_COMMAND(ID_ANALYSIS_BROWSEMEASUREDDATA,			OnMenuAnalysisBrowseData)
	
	// Changing the units
	ON_COMMAND(ID_UNITOFFLUX_KG,						OnChangeUnitOfFluxToKgS)
	ON_COMMAND(ID_UNITOFFLUX_TON,						OnChangeUnitOfFluxToTonDay)
	ON_COMMAND(ID_UNITOFCOLUMNS_PPMM,					OnChangeUnitOfColumnToPPMM)
	ON_COMMAND(ID_UNITOFCOLUMNS_MOLEC_CM2,				OnChangeUnitOfColumnToMolecCm2)

	ON_WM_SIZE()
END_MESSAGE_MAP()

// CNovacViewerView construction/destruction

CNovacViewerView::CNovacViewerView()
	: CFormView(CNovacViewerView::IDD)
{
	pView = this;
	m_evalDataStorage = new CEvaluatedDataStorage();
	m_commDataStorage = new CCommunicationDataStorage();

	m_overView		= NULL;
	m_windOverView	= NULL;
	m_instrumentView= NULL;
}

CNovacViewerView::~CNovacViewerView()
{
	delete m_evalDataStorage;
	delete m_commDataStorage;
	delete m_overView;
	delete m_windOverView;
	delete m_instrumentView;

	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CPropertyPage *page = m_scannerPages[i];
		delete page;
	}
	m_scannerPages.RemoveAll();
}

void CNovacViewerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MASTERFRAME,			m_masterFrame);
	DDX_Control(pDX, IDC_STATUS_MESSAGE_LIST,	m_statusListBox);
	DDX_Control(pDX, IDC_STATUS_STATIC,			m_statusFrame);
}

BOOL CNovacViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CNovacViewerView::OnInitialUpdate()
{
	CString message;
	CRect rect, rect2, tabRect;
	CString fileName, windFieldFile, userSettingsFile;
	CString path, serialNumber, dateStr;
	FileHandler::CFluxLogFileHandler fluxLogReader;

	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	m_common.GetExePath();

	// Initialize the master-frame
	m_sheet.Construct("", this);

	// Read the configuration file
	fileName.Format("%sconfiguration.xml", m_common.m_exePath);
	FileHandler::CConfigurationFileHandler reader;
	reader.ReadConfigurationFile(g_settings, &fileName);

	// Read the user settings
	userSettingsFile.Format("%s\\user.ini", m_common.m_exePath);
	g_userSettings.ReadSettings(&userSettingsFile);

	// If there's no output-directory specified
	if(strlen(g_settings.outputDirectory) <= 2){
		g_settings.outputDirectory.Format(m_common.m_exePath);
	}

	// Check if there's any flux-logs with data from earlier today
	dateStr.Format("%04d.%02d.%02d", m_common.GetYear(), m_common.GetMonth(), m_common.GetDay());
	for(unsigned int it = 0; it < g_settings.scannerNum; ++it){
		serialNumber.Format(g_settings.scanner[it].spec[0].serialNumber);
		path.Format("%sOutput\\%s\\%s\\FluxLog_%s_%s.txt", g_settings.outputDirectory, dateStr, serialNumber, serialNumber, dateStr);

		m_evalDataStorage->AddData(serialNumber, NULL);

		if(IsExistingFile(path)){
			// Try to read the flux-log
			fluxLogReader.m_fluxLog.Format(path);
			if(FAIL == fluxLogReader.ReadFluxLog())
				continue;

			if(fluxLogReader.m_fluxesNum > 0){
				// Copy the read-in data to the m_evalDataStorage
				int fluxesNum = fluxLogReader.m_fluxesNum;

				for(int it2 = 0; it2 < fluxesNum; ++it2){
					Evaluation::CFluxResult &fl = fluxLogReader.m_fluxes[it2];
					CSpectrumInfo &info					= fluxLogReader.m_scanInfo[it2];
					m_evalDataStorage->AppendFluxResult(it, fl.m_startTime, fl.m_flux, fl.m_fluxOk, info.m_batteryVoltage, info.m_temperature, info.m_exposureTime);
				}

				// Insert the last used wind-field for the current spectrometer
				CWindFieldAndPlumeHeight windField;
				if(fluxLogReader.m_fluxes[fluxesNum-1].m_plumeHeight > 0 && fluxLogReader.m_fluxes[fluxesNum-1].m_plumeHeight < 5000){
					windField.SetPlumeHeight(fluxLogReader.m_fluxes[fluxesNum-1].m_plumeHeight,			fluxLogReader.m_fluxes[fluxesNum-1].m_plumeHeightSource);
				}else{
					windField.SetPlumeHeight(1000,	MET_DEFAULT);
				}
				if(fluxLogReader.m_fluxes[fluxesNum-1].m_windDirection > -180 && fluxLogReader.m_fluxes[fluxesNum-1].m_windDirection <= 360){
					windField.SetWindDirection(fluxLogReader.m_fluxes[fluxesNum-1].m_windDirection,	fluxLogReader.m_fluxes[fluxesNum-1].m_windDirectionSource);
				}else{
					windField.SetWindDirection(0,		MET_DEFAULT);
				}
				if(fluxLogReader.m_fluxes[fluxesNum-1].m_windSpeed > -1 && fluxLogReader.m_fluxes[fluxesNum-1].m_windSpeed <= 30){
					windField.SetWindSpeed(fluxLogReader.m_fluxes[fluxesNum-1].m_windSpeed,					fluxLogReader.m_fluxes[fluxesNum-1].m_windSpeedSource);
				}else{
					windField.SetWindSpeed(10,			MET_DEFAULT);
				}

				g_metData.SetWindField(serialNumber, windField);
			}
		}
	}

	// Try to find and read in a wind-field file, if any can be found...
	if(g_settings.windSourceSettings.windFieldFile.GetLength() > 0 && IsExistingFile(g_settings.windSourceSettings.windFieldFile)){
		if(0 == g_metData.ReadWindFieldFromFile(g_settings.windSourceSettings.windFieldFile)){
			ShowMessage("Successfully read in wind-field from file");
			this->PostMessage(WM_NEW_WINDFIELD, NULL, NULL);
		}
	}

	// Check if there is any old status-log file from which we can learn anything...
//	ScanStatusLogFile();

	// Initialize the controls of the screen
	InitializeControls();

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	CTabCtrl *tabPtr = m_sheet.GetTabControl();
    int i = 0;
	for(i = 0; i < tabPtr->GetItemCount() - 1; ++i){
		tabPtr->GetItemRect(i, &tabRect);
		m_toolTip.AddTool(tabPtr, IDD_VIEW_SCANNERSTATUS, &tabRect, IDD_VIEW_SCANNERSTATUS);
	}
	tabPtr->GetItemRect(i, &tabRect);
	m_toolTip.AddTool(tabPtr, IDD_VIEW_OVERVIEW, &tabRect, IDD_VIEW_OVERVIEW);
	tabPtr->SetToolTips(&m_toolTip);
	tabPtr->EnableToolTips(TRUE);

	m_toolTip.SetMaxTipWidth(INT_MAX);
	m_toolTip.Activate(TRUE);

	message.Format("%s", m_common.GetString(MSG_PLEASE_PRESS_START));
	ShowMessage(message);

	// initialize the default wind field
	g_metData.defaultWindField.SetPlumeHeight(1000, MET_DEFAULT);
	g_metData.defaultWindField.SetWindDirection(0, MET_DEFAULT);
	g_metData.defaultWindField.SetWindSpeed(10, MET_DEFAULT);

	m_dateChangeDlg.Create(IDD_CHANGE_DATE_DIALOG);
	m_dateChangeDlg.ShowWindow(SW_SHOW);

	// update the window
	UpdateData(FALSE);
}


// CNovacViewerView diagnostics

#ifdef _DEBUG
void CNovacViewerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CNovacViewerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CNovacViewerDoc* CNovacViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNovacViewerDoc)));
	return (CNovacViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CNovacViewerView message handlers


int CNovacViewerView::InitializeControls(){
	// Initialize the master frame
	CRect rect, rect2;
	CView_Scanner *page;
	CString site;
	unsigned int i;
	TCITEM tcItem;
	m_showWindOverView = false;

	// Add the pages, one for every spectrometer configured
	if(g_settings.scannerNum == 0){
			page = new CView_Scanner();
			page->Construct(IDD_VIEW_SCANNERSTATUS);

			page->m_evalDataStorage = this->m_evalDataStorage;
			page->m_commDataStorage = this->m_commDataStorage;
			page->m_scannerIndex				= 0;
			page->m_serial.Format("unknown");

			m_sheet.AddPage(page);
			m_scannerPages.Add(page);
	}else{
		for(i = 0; i < g_settings.scannerNum; ++i){
			page = new CView_Scanner();
			page->Construct(IDD_VIEW_SCANNERSTATUS);

			page->m_evalDataStorage = this->m_evalDataStorage;
			page->m_commDataStorage = this->m_commDataStorage;
			page->m_scannerIndex	= i;
			page->m_serial.Format("%s", g_settings.scanner[i].spec[0].serialNumber);
			page->m_siteName.Format("%s", g_settings.scanner[i].site);

			m_sheet.AddPage(page);
			m_scannerPages.Add(page);

			// if this system can make wind-measurements then show the wind-overView page
			if(g_settings.scanner[i].spec[0].channelNum == 2 || g_settings.scanner[i].instrumentType == INSTR_HEIDELBERG){
				m_showWindOverView = true;
			}
		}
	}

	// ...add the overview page...
	m_overView = new CView_OverView();
	m_overView->Construct(IDD_VIEW_OVERVIEW);
	m_overView->m_evalDataStorage = this->m_evalDataStorage;
	m_overView->m_commDataStorage = this->m_commDataStorage;
	m_sheet.AddPage(m_overView);

	// ...add the instrument overview page...
	m_instrumentView = new CView_Instrument();
	m_instrumentView->Construct(IDD_VIEW_INSTRUMENT_OVERVIEW);
	m_instrumentView->m_evalDataStorage = this->m_evalDataStorage;
	m_instrumentView->m_commDataStorage	= this->m_commDataStorage;
	m_sheet.AddPage(m_instrumentView);
	m_instrumentViewVisible	= true;

	// At last, add the wind-measurements overview page, 
	//	if there is at least one instrument which is capable of making wind-measurements
	if(m_showWindOverView){
		m_windOverView = new CView_WindMeasOverView();
		m_windOverView->Construct(IDD_VIEW_WIND_OVERVIEW);
		m_windOverView->m_evalDataStorage = this->m_evalDataStorage;
		m_sheet.AddPage(m_windOverView);
	}

	// Create the sheet and move it to it's position on the screen
	m_sheet.Create(this, WS_CHILD | WS_VISIBLE);
	m_sheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// Move everything into place...
	m_masterFrame.GetWindowRect(rect);
	GetWindowRect(rect2);
	m_sheet.MoveWindow(rect2.left, rect.top - rect2.top, rect2.Width(), rect.Height());
	m_masterFrame.GetWindowRect(rect);

	//for(i = 0; i < m_scannerPages.GetCount(); ++i){
	//	CPropertyPage *page = m_scannerPages.GetAt(i);
	//	page->GetWindowRect(rect2);
	//	rect2.right		= rect.right - 10;
	//	rect2.top			-= rect.top;
	//	rect2.bottom	-= rect.top;
	//	page->MoveWindow(rect2);
	//}

	// Get the tab control and set the title of each tab to the serial number of the spectrometer
	CTabCtrl *tabPtr = m_sheet.GetTabControl();
	if(g_settings.scannerNum <= 0){
		
		// Get the 'item' of the tab.
		tcItem.mask = TCIF_TEXT;

		// Set the text in the 'item'
		site.Format("Unknown Scanner");
		tcItem.pszText = site.GetBuffer(256);

		// Update the tab with the updated 'item'
		tabPtr->SetItem(0, &tcItem);
	}else{
		for(i = 0; i < g_settings.scannerNum; ++i){
			tcItem.mask = TCIF_TEXT;

			// Set the text in the 'item'. !!!! The serial-string is necessary
			//	otherwise this function changes the global object !!!!!!!!!!!
			site.Format("%s", g_settings.scanner[i].site);
			tcItem.pszText = site.GetBuffer(256);
			
			// Update the tab with the updated 'item'
			tabPtr->SetItem(i, &tcItem);
		}
	}
	return 0;
}


LRESULT CNovacViewerView::OnShowMessage(WPARAM wParam, LPARAM lParam){
	CString *msg = (CString *)wParam;
	CString logFile,dateStr,logPath;

	if(msg == NULL)
		return 0;

	// add the message to the log file
	m_common.GetDateText(dateStr);
	logPath.Format("%sOutput\\%s", g_settings.outputDirectory,dateStr);

	CreateDirectory(logPath,NULL);
	logFile.Format("%s\\StatusLog.txt",logPath);
	FILE *f = fopen(logFile, "a+");
	if(f != NULL){
		fprintf(f, "%s\n", *msg);
		fclose(f);
	}

	// update the status message listbox
	int nItems = m_statusListBox.GetCount();
	if(nItems > 100){
		m_statusListBox.DeleteString(100);
	}
	m_statusListBox.InsertString(0, *msg);

	if(strlen(*msg) > 15){
		// Find the longest string in the list box.
		CString str;
		CSize sz;
		int dx = 0;
		TEXTMETRIC tm;
		CDC* pDC = m_statusListBox.GetDC();
		CFont* pFont = m_statusListBox.GetFont();

		// Select the listbox font, save the old font
		CFont* pOldFont = pDC->SelectObject(pFont);
		// Get the text metrics for avg char width
		pDC->GetTextMetrics(&tm); 

		for (int i = 0; i < m_statusListBox.GetCount(); i++)
		{
			m_statusListBox.GetText(i, str);
			sz = pDC->GetTextExtent(str);

			// Add the avg width to prevent clipping
			sz.cx += tm.tmAveCharWidth;

			if (sz.cx > dx)
				dx = sz.cx;
		}
		// Select the old font back into the DC
		pDC->SelectObject(pOldFont);
		m_statusListBox.ReleaseDC(pDC);

		// Set the horizontal extent so every character of all strings can be scrolled to.
		m_statusListBox.SetHorizontalExtent(dx);
	}

	delete msg;

	return 0;
}
void CNovacViewerView::ForwardMessage(int message, WPARAM wParam, LPARAM lParam){
	unsigned int i;

	// 1. forward the message to the flux-overview, if it is selected
	if(m_overView->m_hWnd != NULL)
		m_overView->PostMessage(message, wParam, lParam);

	// 2. forward the message to the instrument-overview, if it is selected
	if(m_instrumentView->m_hWnd != NULL)
		m_instrumentView->PostMessage(message, wParam, lParam);

	// 3. Find the scanner view to forward to...
	if(wParam != NULL){
		CString *serial = (CString *)wParam;

		// 3a. look for the correct spectrometer
		for(i = 0; i < g_settings.scannerNum; ++i){
			if(Equals(*serial, g_settings.scanner[i].spec[0].serialNumber))
				break;
		}
		if(i == g_settings.scannerNum)
			return; // <-- nothing found.

		// 3b. forward the message to the correct scanner view, if it is selected
		if(m_scannerPages[i]->m_hWnd != NULL)
			m_scannerPages[i]->PostMessage(message, wParam, lParam);
	}else{
		// if not to any specific scanner view then just forward to the one which is shown right now
		for(i = 0; i < g_settings.scannerNum; ++i){
			if(m_scannerPages[i]->m_hWnd != NULL){
				m_scannerPages[i]->PostMessage(message, wParam, lParam);
				break;
			}
		}
	}
}

LRESULT CNovacViewerView::OnChangedDate(WPARAM wParam, LPARAM lParam){
	m_evalDataStorage->Clear();
	return 0;
}

LRESULT CNovacViewerView::OnEvalSucess(WPARAM wParam, LPARAM lParam){
	// the serial number of the spectrometer that has sucessfully evaluated one scan
	CString *serial = (CString *)wParam;
	Evaluation::CScanResult *result = (Evaluation::CScanResult *)lParam;

	if(result->GetCorruptedNum() == 0)
		m_evalDataStorage->SetStatus(*serial, STATUS_GREEN);
	else if(result->GetCorruptedNum() < 5)
		m_evalDataStorage->SetStatus(*serial, STATUS_YELLOW);
	else
		m_evalDataStorage->SetStatus(*serial, STATUS_RED);

	m_evalDataStorage->AddData(*serial, result);

	// forward the message to the correct scanner view
	if(m_overView->m_hWnd != NULL){
		m_overView->PostMessage(WM_EVAL_SUCCESS, wParam, NULL);
	}
	if(m_instrumentView->m_hWnd != NULL){
		m_instrumentView->PostMessage(WM_EVAL_SUCCESS, wParam, NULL);
	}
	for(int i = 0; i < g_settings.scannerNum; ++i){
		if(Equals(*serial, g_settings.scanner[i].spec[0].serialNumber)){
			if(m_scannerPages[i]->m_hWnd != NULL){
				Evaluation::CScanResult *copiedResult = new Evaluation::CScanResult();
				*copiedResult = *result;
				m_scannerPages[i]->PostMessage(WM_EVAL_SUCCESS, wParam, (LPARAM)copiedResult);
			}
		}
	}

	// clean up the results...
	delete result;

	return 0;
}

void CNovacViewerView::OnMenuAnalysisFlux()
{
	CPostFluxDlg fluxDlg;
	fluxDlg.DoModal();
}

void CNovacViewerView::OnMenuAnalysisBrowseData(){
	Dialogs::CDataBrowserDlg dlg;
	dlg.DoModal();
}

void CNovacViewerView::OnMenuAnalysisWind()
{
	Dialogs::CPostWindDlg windDlg;
	windDlg.DoModal();
}

void CNovacViewerView::OnMenuSetLanguageEnglish()
{
	g_userSettings.m_language = LANGUAGE_ENGLISH;
	g_userSettings.WriteToFile();
	
	::SetThreadLocale(MAKELCID(MAKELANGID(0x0409, SUBLANG_DEFAULT),SORT_DEFAULT));
	primaryLanguage = 0x0409;

	Common common;
	MessageBox(common.GetString(MSG_YOU_HAVE_TO_RESTART), "Change of language", MB_OK);
}
void CNovacViewerView::OnMenuSetLanguageSpanish()
{
	g_userSettings.m_language = LANGUAGE_SPANISH;
	g_userSettings.WriteToFile();

	::SetThreadLocale(MAKELCID(MAKELANGID(0x0c0a, SUBLANG_DEFAULT),SORT_DEFAULT));
	primaryLanguage = 0x0c0a;

	Common common;
	MessageBox(common.GetString(MSG_YOU_HAVE_TO_RESTART), "Change of language", MB_OK);
}

void CNovacViewerView::OnUpdateSetLanguageEnglish(CCmdUI *pCmdUI)
{
	if(primaryLanguage == 0x0c0a) // 0x0c0a == spanish...
		pCmdUI->SetCheck(0);
	else
		pCmdUI->SetCheck(1);
}

void CNovacViewerView::OnUpdateSetLanguageSpanish(CCmdUI *pCmdUI)
{
	if(primaryLanguage == 0x0c0a)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacViewerView::OnChangeUnitOfFluxToKgS(){
	g_userSettings.m_fluxUnit = UNIT_KGS;
	g_userSettings.WriteToFile();

	// Re-draw the graphs
	if(m_overView->m_hWnd != NULL){
		m_overView->DrawFlux();
		return;
	}
	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CView_Scanner *page = (CView_Scanner *)m_scannerPages[i];
		if(page->m_hWnd != NULL){
			page->DrawFlux();
			return;
		}
	}
}
void CNovacViewerView::OnChangeUnitOfFluxToTonDay(){
	g_userSettings.m_fluxUnit = UNIT_TONDAY;
	g_userSettings.WriteToFile();

	// Re-draw the graphs
	if(m_overView->m_hWnd != NULL){
		m_overView->DrawFlux();
		return;
	}
	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CView_Scanner *page = (CView_Scanner *)m_scannerPages[i];
		if(page->m_hWnd != NULL){
			page->DrawFlux();
			return;
		}
	}
}

void CNovacViewerView::OnUpdateChangeUnitOfFluxToKgS(CCmdUI *pCmdUI)
{
	if(g_userSettings.m_fluxUnit == UNIT_KGS)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacViewerView::OnUpdateChangeUnitOfFluxToTonDay(CCmdUI *pCmdUI)
{
	if(g_userSettings.m_fluxUnit == UNIT_TONDAY)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacViewerView::OnChangeUnitOfColumnToPPMM(){
	g_userSettings.m_columnUnit = UNIT_PPMM;
	g_userSettings.WriteToFile();

	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CView_Scanner *page = (CView_Scanner *)m_scannerPages[i];
		if(page->m_hWnd != NULL){
			page->DrawColumn();
			return;
		}
	}
}

void CNovacViewerView::OnChangeUnitOfColumnToMolecCm2(){
	g_userSettings.m_columnUnit = UNIT_MOLEC_CM2;
	g_userSettings.WriteToFile();

	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CView_Scanner *page = (CView_Scanner *)m_scannerPages[i];
		if(page->m_hWnd != NULL){
			page->DrawColumn();
			return;
		}
	}
}

void CNovacViewerView::OnUpdateChangeUnitOfColumnToPPMM(CCmdUI *pCmdUI)
{
	if(g_userSettings.m_columnUnit == UNIT_PPMM)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacViewerView::OnUpdateChangeUnitOfColumnToMolecCm2(CCmdUI *pCmdUI)
{
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacViewerView::OnMenuShowConfigurationDialog()
{
	ConfigurationDialog::CConfigurationDlg dlg;
	INT_PTR ret = dlg.DoModal();
}


void CNovacViewerView::SetLayout(){
	if(this->m_hWnd == NULL || m_statusFrame.m_hWnd == NULL || m_statusListBox.m_hWnd == NULL || m_masterFrame.m_hWnd == NULL || m_sheet.m_hWnd == NULL)
		return;

	// Resize this window to the size of the main-window
	int bottomMargin	= 10;
	int screenHeight	= GetSystemMetrics(SM_CYSCREEN);
	int screenWidth		= GetSystemMetrics(SM_CXSCREEN);
	
	CRect statusFrameRect, statusListRect, masterFrameRect, thisRect;

	// Get the size of this window
	GetWindowRect(thisRect);
	
	if(thisRect.Height() < 650){
		thisRect.bottom = thisRect.top + 650;
	}

	// Put the status-list frame where it should be
	m_statusFrame.GetWindowRect(statusFrameRect);
	int height				= statusFrameRect.Height();
	statusFrameRect.left	= 0;
	statusFrameRect.right	= thisRect.Width()	- statusFrameRect.left - thisRect.left;
	statusFrameRect.bottom	= thisRect.Height() - bottomMargin;
	statusFrameRect.top		= statusFrameRect.bottom - height;
	m_statusFrame.MoveWindow(statusFrameRect);

	// Put the status-list box where it should be
	this->m_statusListBox.GetWindowRect(statusListRect);
	statusListRect.left		= statusFrameRect.left		+ 10;
	statusListRect.top		= statusFrameRect.top		+ 20;
	statusListRect.right	= statusFrameRect.right		- 10;
	statusListRect.bottom	= statusFrameRect.bottom	- 10;
	m_statusListBox.MoveWindow(statusListRect);

	// Put the master frame where it should be
	this->m_masterFrame.GetWindowRect(masterFrameRect);
	masterFrameRect.left		= 10;
	masterFrameRect.top			= 0;
	masterFrameRect.right		= thisRect.right		- 10;
	masterFrameRect.bottom		= statusFrameRect.top	- 10;
	m_masterFrame.MoveWindow(masterFrameRect);

	// Move the property sheet into place	
	m_masterFrame.GetWindowRect(masterFrameRect);
	m_sheet.MoveWindow(10, masterFrameRect.top - thisRect.top, thisRect.Width(), masterFrameRect.Height());
	
}

void CNovacViewerView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// Fix the layout of the main components to fit the screen
	SetLayout();
}