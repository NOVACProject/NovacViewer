// NovacViewerView.h : interface of the CNovacViewerView class
//


#pragma once

#include "afxwin.h"

#include "Configuration/Configuration.h"

#include "Configuration/Configuration.h"
#include "Configuration/EvaluationConfigurationDlg.h"
#include "Dialogs/ConfigurationDlg.h"
#include "EvaluatedDataStorage.h"
#include "CommunicationDataStorage.h"
#include "MeteorologicalData.h"
#include "View_OverView.h"
#include "View_WindMeasOverView.h"
#include "View_Instrument.h"
#include "View_Scanner.h"
#include "Dialogs/ChangeSelectedDateDlg.h"
#include "ResizablePropertySheet.h"

class CNovacViewerView : public CFormView
{
protected: // create from serialization only
	CNovacViewerView();
	DECLARE_DYNCREATE(CNovacViewerView)

public:
	enum{ IDD = IDD_NOVACVIEWER_FORM };

// Attributes
public:
	CNovacViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

	afx_msg void OnSize(UINT nType, int cx, int cy);

	/** Fixes the layout of the components to fit the screen resolution*/
	void				 SetLayout();

// Implementation
public:
	virtual ~CNovacViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnShowMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEvalSucess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangedDate(WPARAM wParam, LPARAM lParam);
	
	// analysis
	afx_msg void OnMenuAnalysisFlux();
	afx_msg void OnMenuAnalysisWind();
	afx_msg void OnMenuAnalysisBrowseData();
	
	// configuration
	afx_msg void OnMenuShowConfigurationDialog();
	
	// language
	afx_msg void OnMenuSetLanguageEnglish();
	afx_msg void OnMenuSetLanguageSpanish();
	afx_msg void OnUpdateSetLanguageEnglish(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSetLanguageSpanish(CCmdUI *pCmdUI);

	// units
	afx_msg void OnChangeUnitOfFluxToKgS();
	afx_msg void OnChangeUnitOfFluxToTonDay();
	afx_msg void OnChangeUnitOfColumnToPPMM();
	afx_msg void OnChangeUnitOfColumnToMolecCm2();
	afx_msg void OnUpdateChangeUnitOfFluxToKgS(CCmdUI *pCmdUI);
	afx_msg void OnUpdateChangeUnitOfFluxToTonDay(CCmdUI *pCmdUI);
	afx_msg void OnUpdateChangeUnitOfColumnToPPMM(CCmdUI *pCmdUI);
	afx_msg void OnUpdateChangeUnitOfColumnToMolecCm2(CCmdUI *pCmdUI);

public:
	// --------------- DIALOG COMPONENTS ------------------------------
	// the status messages
	CListBox m_statusListBox;
	CStatic	 m_statusFrame;

	// the tool tips
	CToolTipCtrl  m_toolTip;

	// The property pages
	CArray<CView_Scanner *, CView_Scanner *> m_scannerPages;

	// The overview page
	CView_OverView					*m_overView;

	// The wind-measurements overview page
	CView_WindMeasOverView	*m_windOverView;
	bool										m_showWindOverView;

	// The instrument page
	CView_Instrument				*m_instrumentView;
	bool										m_instrumentViewVisible; // true if the Instrument-page is shown

	// The property sheet, holds the property pages
	CResizablePropertySheet	m_sheet;

	/** */
	CTabCtrl  m_spectrometerTab;
	
	/** */
	CStatic m_masterFrame;	

	CChangeSelectedDateDlg m_dateChangeDlg;
	// ---------------------- AUXILLIARY FUNCTIONS -----------------------

	/** Called when the configuration file has been read */
	int InitializeControls();

	/** Forwards messages to the different views */
	void ForwardMessage(int message, WPARAM wParam, LPARAM lParam);
	
private:
	// --------------- DATA STRUCTURES FOR SHOWING THE EVALUATION RESULT -------------

	/** This object holds all the evaluated data that we need for plotting */
	CEvaluatedDataStorage *m_evalDataStorage;

	/** This object holds the communication status */
	CCommunicationDataStorage *m_commDataStorage;

	/** A common object for doing common things */
	Common m_common;

	/** This class contains critical sections of code */
//	CCriticalSection m_critSect;

public:
	afx_msg void OnViewChangeSelectedDate();
};

#ifndef _DEBUG  // debug version in NovacViewerView.cpp
inline CNovacViewerDoc* CNovacViewerView::GetDocument() const
   { return reinterpret_cast<CNovacViewerDoc*>(m_pDocument); }
#endif

