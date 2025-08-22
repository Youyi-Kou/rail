
// PosJudge.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "Gdiplus.h"
using namespace Gdiplus;

// CPosJudgeApp:
// See PosJudge.cpp for the implementation of this class
//

class CPosJudgeApp : public CWinApp
{
public:
	CPosJudgeApp();
	virtual ~CPosJudgeApp();

// Overrides
public:
	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;

	ULONG_PTR m_pGdiToken;

	virtual BOOL InitInstance();

	virtual int ExitInstance(); // default will 'delete this'
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPosJudgeApp theApp;