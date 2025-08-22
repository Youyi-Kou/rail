
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
	//	AfxMessageBox(_T("����������ѽ���������ϵ����Ա��"), MB_OK | MB_ICONWARNING);
	//	return FALSE;
	//}
	
	g_strMarkDefines[1] = "2�׽�ͷ";
	g_strMarkDefines[2] = "4�׽�ͷ";
	g_strMarkDefines[3] = "6�׽�ͷ";
	g_strMarkDefines[4] = "���Ⱥ���";
	g_strMarkDefines[5] = "����";
	g_strMarkDefines[6] = "�ֳ���";

	g_strMarkDefines[7] = "�ݿ�";
	g_strMarkDefines[8] = "����";

	g_strMarkDefines[9] = "�����µ�";
	g_strMarkDefines[10] = "���͹���µ�";
	g_strMarkDefines[11] = "�Ͻ����ʼ";
	g_strMarkDefines[12] = "�Ͻ�������";
	g_strMarkDefines[13] = "�̸ֵ���ʼ";
	g_strMarkDefines[14] = "�̸ֵ������";

	g_strMarkDefines[30] = "�ϵ�";
	g_strMarkDefines[31] = "����";
	g_strMarkDefines[40] = "���У��";

	g_strGuBieDefines[0] = "�ҹ�";
	g_strGuBieDefines[1] = "���";

	g_strXingBieDefines[0] = "����";
	g_strXingBieDefines[1] = "����";
	g_strXingBieDefines[2] = "����";

	g_strWoundDefines[0] = L"��ȱ�ݻز�";
	g_strWoundDefines[1] = L"����";
	g_strWoundDefines[2] = L"������";
	g_strWoundDefines[4] = L"ˮƽ����";
	g_strWoundDefines[5] = L"��ͷ����";
	g_strWoundDefines[6] = L"���Ⱥ�����";
	g_strWoundDefines[7] = L"��������";
	g_strWoundDefines[17] = L"�ݿ�һ��������";
	g_strWoundDefines[18] = L"�ݿ׶���������";
	g_strWoundDefines[19] = L"�ݿ�����������";
	g_strWoundDefines[20] = L"�ݿ�����������";
	g_strWoundDefines[32] = L"�ݿ��Ҳ�ˮƽ����";
	g_strWoundDefines[33] = L"�ݿ����ˮƽ����";
	g_strWoundDefines[41] = L"����һ��������";
	g_strWoundDefines[42] = L"���׶���������";
	g_strWoundDefines[43] = L"��������������";
	g_strWoundDefines[44] = L"��������������";
	g_strWoundDefines[45] = L"�����Ҳ�ˮƽ����";
	g_strWoundDefines[46] = L"�������ˮƽ����";
	g_strWoundDefines[64] = L"��������";
	g_strWoundDefines[128] = L"б����";
	g_strWoundDefines[256] = L"��׺�������";
	g_strWoundDefines[300] = L"��ͷ�쳣";
	g_strWoundDefines[310] = L"����쳣";
	g_strWoundDefines[320] = L"�����쳣";
	g_strWoundDefines[330] = L"����쳣";
	g_strWoundDefines[400] = L"�ݿ��׿�";
	g_strWoundDefines[410] = L"�����׿�";
	g_strWoundDefines[420] = L"���";
	g_strWoundDefines[1000] = L"�˹�����";

	g_strDegreeDefines[0] = L"����";
	g_strDegreeDefines[1] = L"��������";
	g_strDegreeDefines[2] = L"����";
	g_strDegreeDefines[3] = L"�ᷢ";
	g_strDegreeDefines[4] = L"����";
	g_strDegreeDefines[5] = L"�۶�";

	g_strWoundPlaceDefines[0] = L"-";
	g_strWoundPlaceDefines[1] = L"��ͷ̤����";
	g_strWoundPlaceDefines[2] = L"��ͷ��";
	g_strWoundPlaceDefines[4] = L"��ͷ��";
	g_strWoundPlaceDefines[8] = L"��ͷ��";
	g_strWoundPlaceDefines[14] = L"��ͷ";
	g_strWoundPlaceDefines[16] = L"����";
	g_strWoundPlaceDefines[32] = L"���";
	g_strWoundPlaceDefines[64] = L"���";
	g_strWoundPlaceDefines[128] = L"����";
	g_strWoundPlaceDefines[256] = L"���";
	g_strWoundPlaceDefines[512] = L"��׽���";
	g_strWoundPlaceDefines[1024] = L"��׽���";

	g_strCheckStateDefines[0] = "δ����";
	g_strCheckStateDefines[1] = "������";
	g_strCheckStateDefines[2] = "�Ѹ���";

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

