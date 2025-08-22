
// PosJudge.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PosJudge.h"
#include "PosJudgeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPosJudgeApp

BEGIN_MESSAGE_MAP(CPosJudgeApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPosJudgeApp construction

CPosJudgeApp::CPosJudgeApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


CPosJudgeApp::~CPosJudgeApp()
{

}

// The one and only CPosJudgeApp object

CPosJudgeApp theApp;


// CPosJudgeApp initialization

BOOL CPosJudgeApp::InitInstance()
{
	int sz = sizeof(B_Step4NodejsPT);

	CoInitialize(NULL);
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	Gdiplus::GdiplusStartup(&m_pGdiToken, &m_gdiplusStartupInput, NULL);

	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	time_t timec;
	time(&timec);
	tm* p = localtime(&timec);

	tm tmBegin;
	tmBegin.tm_hour = 0; tmBegin.tm_min = 0; tmBegin.tm_sec = 0;
	tmBegin.tm_year = 2019-1900; tmBegin.tm_mon = 11; tmBegin.tm_mday = 6;
	time_t timeBegin = mktime(&tmBegin);
	double delt = difftime(timec, timeBegin);
	const double tryTime = 60 * 60 * 24 * 15;
	//if (delt >= tryTime)
	//{
	//	AfxMessageBox(_T("软件试用期已结束，请联系管理员！"), MB_OK | MB_ICONWARNING);
	//	return FALSE;
	//}
	
	g_strMarkDefines[1] = "2孔接头";
	g_strMarkDefines[2] = "4孔接头";
	g_strMarkDefines[3] = "6孔接头";
	g_strMarkDefines[4] = "铝热焊缝";
	g_strMarkDefines[5] = "厂焊";
	g_strMarkDefines[6] = "现场焊";

	g_strMarkDefines[7] = "螺孔";
	g_strMarkDefines[8] = "导孔";

	g_strMarkDefines[9] = "尖轨变坡点";
	g_strMarkDefines[10] = "异型轨变坡点";
	g_strMarkDefines[11] = "合金道岔开始";
	g_strMarkDefines[12] = "合金道岔结束";
	g_strMarkDefines[13] = "锰钢道岔开始";
	g_strMarkDefines[14] = "锰钢道岔结束";

	g_strMarkDefines[30] = "上道";
	g_strMarkDefines[31] = "回退";
	g_strMarkDefines[40] = "里程校正";

	g_strGuBieDefines[0] = "右股";
	g_strGuBieDefines[1] = "左股";

	g_strXingBieDefines[0] = "上行";
	g_strXingBieDefines[1] = "下行";
	g_strXingBieDefines[2] = "单线";

	g_strWoundDefines[0] = L"非缺陷回波";
	g_strWoundDefines[1] = L"核伤";
	g_strWoundDefines[2] = L"鱼鳞伤";
	g_strWoundDefines[4] = L"水平裂纹";
	g_strWoundDefines[5] = L"接头伤损";
	g_strWoundDefines[6] = L"铝热焊伤损";
	g_strWoundDefines[7] = L"厂焊伤损";
	g_strWoundDefines[17] = L"螺孔一象限裂纹";
	g_strWoundDefines[18] = L"螺孔二象限裂纹";
	g_strWoundDefines[19] = L"螺孔三象限裂纹";
	g_strWoundDefines[20] = L"螺孔四象限裂纹";
	g_strWoundDefines[32] = L"螺孔右侧水平裂纹";
	g_strWoundDefines[33] = L"螺孔左侧水平裂纹";
	g_strWoundDefines[41] = L"导孔一象限裂纹";
	g_strWoundDefines[42] = L"导孔二象限裂纹";
	g_strWoundDefines[43] = L"导孔三象限裂纹";
	g_strWoundDefines[44] = L"导孔四象限裂纹";
	g_strWoundDefines[45] = L"导孔右侧水平裂纹";
	g_strWoundDefines[46] = L"导孔左侧水平裂纹";
	g_strWoundDefines[64] = L"纵向裂纹";
	g_strWoundDefines[128] = L"斜裂纹";
	g_strWoundDefines[256] = L"轨底横向裂纹";
	g_strWoundDefines[300] = L"轨头异常";
	g_strWoundDefines[310] = L"轨颚异常";
	g_strWoundDefines[320] = L"轨腰异常";
	g_strWoundDefines[330] = L"轨底异常";
	g_strWoundDefines[400] = L"螺孔套孔";
	g_strWoundDefines[410] = L"导孔套孔";
	g_strWoundDefines[420] = L"多孔";
	g_strWoundDefines[1000] = L"人工标伤";

	g_strDegreeDefines[0] = L"疑似";
	g_strDegreeDefines[1] = L"不到轻伤";
	g_strDegreeDefines[2] = L"轻伤";
	g_strDegreeDefines[3] = L"轻发";
	g_strDegreeDefines[4] = L"重伤";
	g_strDegreeDefines[5] = L"折断";

	g_strWoundPlaceDefines[0] = L"-";
	g_strWoundPlaceDefines[1] = L"轨头踏面中";
	g_strWoundPlaceDefines[2] = L"轨头内";
	g_strWoundPlaceDefines[4] = L"轨头中";
	g_strWoundPlaceDefines[8] = L"轨头外";
	g_strWoundPlaceDefines[14] = L"轨头";
	g_strWoundPlaceDefines[16] = L"轨距角";
	g_strWoundPlaceDefines[32] = L"轨颚";
	g_strWoundPlaceDefines[64] = L"轨颚";
	g_strWoundPlaceDefines[128] = L"轨腰";
	g_strWoundPlaceDefines[256] = L"轨底";
	g_strWoundPlaceDefines[512] = L"轨底角内";
	g_strWoundPlaceDefines[1024] = L"轨底角外";

	g_strCheckStateDefines[0] = "未复核";
	g_strCheckStateDefines[1] = "复核中";
	g_strCheckStateDefines[2] = "已复核";

	CPosJudgeDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CPosJudgeApp::ExitInstance()
{
	Gdiplus::GdiplusShutdown(m_pGdiToken);
	return 0;
}

