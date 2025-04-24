
// 2DStableFluids.h : main header file for the 2DStableFluids application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMy2DStableFluidsApp:
// See 2DStableFluids.cpp for the implementation of this class
//

class CMy2DStableFluidsApp : public CWinAppEx
{
public:
	CMy2DStableFluidsApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMy2DStableFluidsApp theApp;
