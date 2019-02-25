// NovacViewer.h : main header file for the NovacViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CNovacViewerApp:
// See NovacViewer.cpp for the implementation of this class
//

class CNovacViewerApp : public CWinApp
{
public:
	CNovacViewerApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CNovacViewerApp theApp;