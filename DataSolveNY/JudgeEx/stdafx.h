
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

//#define _EXPORT_TO_MYSQL

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WM_WEBINFO_IN (WM_USER + 10)
#define WM_WEBINFO_RECREATE (WM_USER + 11)

#define WM_STEP (WM_USER+100)

#define WM_BEGIN  (WM_USER+110)
#define WM_FINISH (WM_USER+111)


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include "DataEntities.h"
#include "PublicFunc.h"
#include <afxcontrolbars.h>
using namespace std;

//Œª÷√±Íº«
extern map<uint16_t,char*> g_strMarkDefines;

extern map<uint8_t,char*> g_strGuBieDefines;

extern map<uint8_t,char*> g_strXingBieDefines;

extern map<uint16_t, const TCHAR*> g_strWoundDefines;

extern map<uint16_t, const TCHAR*> g_strDegreeDefines;

extern map<uint16_t, const TCHAR*> g_strWoundPlaceDefines;

extern map<uint8_t,char*> g_strCheckStateDefines;