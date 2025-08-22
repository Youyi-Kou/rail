
// TLTSDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PosJudge.h"
#include "PosJudgeDlg.h"
#include "afxdialogex.h"

#include "PublicFunc.h"
#include "DataSolveLib.h"

#include <process.h>
#include <fstream>
#include <thread>
#include <io.h>

#include "DlgLocate.h"
#include "CDlgWoundCompare.h"
#include "Coding.h"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

#define TIMER_SOLVINGDATA 1

// CPosJudgeDlg 对话框

const uint32_t bits[32] =
{ BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6, BIT7, BIT8, BIT9,
BIT10, BIT11, BIT12, BIT13, BIT14, BIT15, BIT16, BIT17, BIT18, BIT19,
BIT20, BIT21, BIT22, BIT23, BIT24, BIT25, BIT26, BIT27, BIT28, BIT29,
BIT30, BIT31
};

const int clrChannels[][4] = {
	{ 255, 96, 255, 1 },
	{ 189, 72, 189, 1 },
	{ 255, 0, 80, 1 },
	{ 189, 0, 60, 1 },

	{ 0, 255, 255, 1 },//B1
	{ 0, 189, 189, 1 },
	{ 0, 128, 255, 1 },//b1
	{ 0, 96, 189, 1 },

	{ 0, 255, 0, 1 },//C
	{ 255, 128, 0, 1 },//c

	{ 255, 255, 0, 1 },
	{ 0, 0, 0, 1 },

	{ 255, 255, 255, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 255, 255, 1 },
	{ 160, 160, 96, 1 }
};

const uint8_t clrBK[] = { 205, 205, 205 };

uint32_t pmCounts[30] = { 0 };
map<const TCHAR*, int> mapWoundNameToIndex;
map<uint16_t, int> mapIsObserve;


//const CString strTypes[] = { "其他", "厂焊焊缝", "铝热焊缝", "轨头核伤",
//"鱼鳞伤", "螺孔斜裂纹", "螺孔水平裂纹", "轨腰（轨颚）水平裂纹",  "轨腰斜裂纹",
//"轨底横向裂纹" };
uint32_t wdCount[100] = { 0 };

vector<Gdiplus::SolidBrush*> brushes;
CString g_strVersion = _T("测试软件x64 v1.2");

CPosJudgeDlg::CPosJudgeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPosJudgeDlg::IDD, pParent)
	, m_szFileB(_T(""))
	, m_strBeginTime(_T(""))
	, m_strEndTime(_T(""))
	, m_strProgress(_T(""))
	, m_strTime(_T(""))
	, m_strRetOld(_T(""))
	, m_strRetNew(_T(""))
	, m_strProgress2(_T(""))
	, m_strWalk(_T(""))
	, m_iAlarmCount(0)
	, m_iWoundCount(0)
	, m_strSection(_T(""))
	, m_strTotalWork(_T(""))
	, m_bSelectAll(TRUE)
	, m_iExport(0)
	, m_strText(_T(""))
	, m_strFileType(_T(""))
	, m_iStartMeterIndex(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pFileA = NULL;
	m_pFileB = NULL;

	m_bSolving = FALSE;
	m_bOpen = FALSE;
	m_bLoaded = FALSE;
	m_bReadingB = FALSE;
	m_iBlock = -1;
	m_iStep = -1;


	m_iCurrentWound = -1;
	m_iCurrentBlock = -1;

	m_BrushBK = new CBrush(RGB(0, 0, 0));

	m_bExported = FALSE;

	m_iMode = 1;

	m_bShowingDialog = false;

	m_brushBK = new SolidBrush(Color(0x00, 0x00, 0x00));
	m_brushText = new SolidBrush(Color(0x8F, 0x8F, 0x8F));

	m_penBlockGrid = new Pen(Color(0x8F, 0x8F, 0x8F), 0.3f);
	m_penBlockGrid->SetDashStyle(Gdiplus::DashStyle::DashStyleDot);

	m_penMark = new Pen(Color(0x8F, 0x00, 0x00), 0.3f);
	m_penMark->SetDashStyle(Gdiplus::DashStyle::DashStyleDot);

	m_penBackBlue = new	Pen(Color(0x68, 0x83, 0x8B), 1.0f);
	m_penBackGreen = new Pen(Color(0x00, 0xFF, 0x00), 1.0f);

	m_penWound = new Pen(Color(0xFF, 0x00, 0x00), 1.0f);
	m_brushWound = new SolidBrush(Color(0xFF, 0x00, 0x00));

	m_penRect = new Pen(Color(0xFF, 0x00, 0x00, 1.0f));


	m_vPMs = NULL;
	m_iPMCount = 0;

	m_vPMs2 = NULL;
	m_iPMCount2 = 0;

	m_vWounds = NULL;
	m_vBA = NULL;

	m_isYoloInit = FALSE;

	//m_oSql = NULL;

	m_bSolveFolder = FALSE;


	/*std::string folder = "E:/新建文件夹/";
	for (int i = 1; i < 17; i++)
	{
		std::string oldfileName = folder + "old" + std::to_string(i);
		std::string newfileName = folder + "new" + std::to_string(i);

		BLOCK oldBlock, newoldBlock;
		byte bytesOld[160], bytesNew[160];

		FILE* pFile = fopen(oldfileName.c_str(), "rb");
		fread(&oldBlock, 160, 1, pFile);
		fclose(pFile);

		pFile = fopen(newfileName.c_str(), "rb");
		fread(&newoldBlock, 160, 1, pFile);
		fclose(pFile);
	}*/
}

CPosJudgeDlg:: ~CPosJudgeDlg()
{
	m_iMode = 1;
	delete m_brushBK;
	delete m_brushText;
	delete m_penBlockGrid;
	delete m_penMark;
	delete m_penBackBlue;
	delete m_penBackGreen;
	delete m_penWound;
	delete m_brushWound;
	delete m_penRect;
	m_brushBK = NULL;

	//delete m_oSql;
}

void CPosJudgeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_szFileB);
	DDX_Text(pDX, IDC_CURRENT, m_strProgress);
	DDX_Control(pDX, IDC_WAVE, m_oPlot);
	DDX_Control(pDX, IDC_WAVE2, m_oPlot2);
	//DDX_Control(pDX, IDC_TIPS, m_oTips);
	DDX_Text(pDX, IDC_STATIC_TIME, m_strTime);
	DDX_Text(pDX, IDC_R1, m_strRetOld);
	DDX_Text(pDX, IDC_R2, m_strRetNew);
	DDX_Text(pDX, IDC_CURRENT2, m_strProgress2);
	DDX_Text(pDX, IDC_EDIT_BLOCK, m_iBlock);
	DDX_Text(pDX, IDC_EDIT_STEP, m_iStep);

	DDX_Text(pDX, IDC_EDIT_WOUND, m_iWoundCount);
	DDX_Text(pDX, IDC_EDIT_ALARM, m_iAlarmCount);

	DDX_Text(pDX, IDC_EDIT_WALK, m_strWalk);

	DDX_Control(pDX, IDC_LIST1, m_oList);

	DDX_Text(pDX, IDC_EDIT_SECTION, m_strSection);
	DDX_Text(pDX, IDC_EDIT_TOTALWORK, m_strTotalWork);
	DDX_Check(pDX, IDC_CHECK_ALL, m_bSelectAll);
	DDX_Text(pDX, IDC_STATIC_TEXT, m_strText);
	DDX_Text(pDX, IDC_EDIT_DATA_TYPE, m_strFileType);
	DDX_Text(pDX, IDC_EDIT_STARTSOLVE_INDEX, m_iStartMeterIndex);
}

BEGIN_MESSAGE_MAP(CPosJudgeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDB_Judge, &CPosJudgeDlg::OnBnClickedJudge)
	ON_BN_CLICKED(IDC_BUTTON2, &CPosJudgeDlg::OnBnClickedBrowse)

	ON_MESSAGE(WM_FINISH, ONMessageFinish)
	ON_MESSAGE(WM_STEP, ONMessageStep)
	ON_BN_CLICKED(IDB_EXPORT, &CPosJudgeDlg::OnBnClickedExport)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDB_LOCATE, &CPosJudgeDlg::OnBnClickedLocate)

	ON_WM_CONTEXTMENU()

	ON_COMMAND(ID_WD_CHECK, &CPosJudgeDlg::OnWdCheck)
	ON_COMMAND(ID_POS_CHECK, &CPosJudgeDlg::OnPosCheck)
	ON_UPDATE_COMMAND_UI(ID_WD_CHECK, &CPosJudgeDlg::OnUpdateWdCheck)
	ON_UPDATE_COMMAND_UI(ID_POS_CHECK, &CPosJudgeDlg::OnUpdatePosCheck)
	ON_BN_CLICKED(IDB_REVISE, &CPosJudgeDlg::OnBnClickedRevise)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_ALL, &CPosJudgeDlg::OnClickedCheckAll)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDB_COMPARE, &CPosJudgeDlg::OnBnClickedCompare)
	//ON_BN_CLICKED(IDB_EXPORT_S, &CPosJudgeDlg::OnBnClickedExportS)
	ON_BN_CLICKED(IDC_BTN_REMOVEFROM_DB, &CPosJudgeDlg::OnBnClickedBtnRemovefromDb)
	ON_BN_CLICKED(IDC_BTN_INSERT_TO_DB, &CPosJudgeDlg::OnBnClickedBtnInsertToDb)
	ON_BN_CLICKED(IDB_SOLVEFOLDER, &CPosJudgeDlg::OnBnClickedSolvefolder)
END_MESSAGE_MAP()

// CPosJudgeDlg 消息处理程序

BOOL CPosJudgeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();	

	TCHAR sz[MAX_PATH];
	::GetModuleFileName(NULL, sz, MAX_PATH);
	CString strPath(sz);
	strPath.Replace('\\', '/');
	m_strExeFolder = strPath.Left(strPath.ReverseFind('/'));
	m_strRetFolder = m_strExeFolder.Mid(m_strExeFolder.ReverseFind('/') + 1);
	char szPath[MAX_PATH] = "";
	CStringA strRetFolderAscii = UnicodeToAscii_CSTR(m_strExeFolder);
	for (int i = 0; i < strRetFolderAscii.GetLength(); ++i)
	{
		szPath[i] = strRetFolderAscii[i];
	}
	AlgInit(szPath, strRetFolderAscii.GetLength());

	//SetSaveRawIamge(1);

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	LoadConfig();

	m_pen.CreatePen(PS_DASHDOT, 1, RGB(0x2F, 0x2F, 0x2F));
	m_pen2.CreatePen(PS_DASHDOT, 1, RGB(0x0, 0x0, 0x0));

	// TODO: 在此添加额外的初始化代码
	m_fontTitle.CreatePointFont(160, _T("宋体"));
	GetDlgItem(IDC_RET_OLD)->SetFont(&m_fontTitle, 1);
	GetDlgItem(IDC_RET_NEW)->SetFont(&m_fontTitle, 1);

	//m_fontTip.CreatePointFont(100, _T("宋体"));

	m_fontTip.CreateFont(-15, 7, 0, 0, 500, FALSE, FALSE,
		0, GB2312_CHARSET, OUT_DEFAULT_PRECIS,//ANSI_CHARSET
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		VARIABLE_PITCH | FF_SCRIPT, _T("宋体"));


	m_strRetNew = "伤损类型";
	GetDlgItem(IDC_R1)->SetFont(&m_fontTip, 1);
	GetDlgItem(IDC_R2)->SetFont(&m_fontTip, 1);


	m_oList.SetExtendedStyle(m_oList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_SHOWSELALWAYS | LVS_EX_CHECKBOXES);
	m_oList.SetTextBkColor(RGB(202, 202, 202));
	m_oList.InsertColumn(0, _T("                        伤损类型"), 0, 140);
	m_oList.InsertColumn(1, _T("伤损数量"), 0, 65);
	int i = 0;
	for (auto itr = g_strWoundDefines.begin(); itr != g_strWoundDefines.end(); ++itr)
	{
		m_oList.InsertItem(i, itr->second);
		m_oList.SetItemText(i, 1, _T("0"));
		m_oList.SetCheck(i, TRUE);
		m_oList.SetItemData(i, itr->first);
		mapIsObserve.insert(make_pair(itr->first, 1));
		mapWoundNameToIndex.insert(make_pair(itr->second, i));
		++i;
	}

	SetMode(1);

	char szAlgV[200] = { 0 };
	GetAlgVersion(szAlgV);
	g_strVersion += " (";
	g_strVersion += szAlgV;
	g_strVersion += ")";
	SetWindowText(g_strVersion);

	//m_oSplit.SetDropDownMenu(IDR_MENU1, 0);
	UpdateData(FALSE);
	GetDlgItem(IDC_CHECK_ALL)->BringWindowToTop();

	//m_oSql = new CMySqlHPE(szPath);
	//m_oSql->ConnectDatabase("127.0.0.1", "tswebisud", "tspttiger", "rail_algtest");

	//SolveFolder();

	return FALSE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPosJudgeDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CString strFile = m_strExeFolder + "/config.ini";
	CStringA strFileA = UnicodeToAscii_CSTR(strFile);
	FILE* pFile = fopen(strFileA, "w");
	if (pFile != NULL)
	{
		fprintf(pFile, "%s\n", UnicodeToAscii_CSTR(m_szFileB));
		fprintf(pFile, "%d\n", m_iExport);
		fclose(pFile);
	}
	CDialogEx::OnClose();
}


void CPosJudgeDlg::SetMode(int iMode)
{
	m_iMode = iMode;
}

void CPosJudgeDlg::LoadConfig()
{
	m_iExport = FALSE;
	ifstream file;
	file.open(m_strExeFolder + _T("\\config.ini"));
	char sz[4000] = { 0 };
	if (file.is_open())
	{
		file.getline(sz, 4000);
		m_szFileB = sz;
		file.getline(sz, 4000);
		m_iExport = atoi(sz);
		file.close();
	}

	for (int i = 0; i < 16; ++i)
	{
		m_R[i] = clrChannels[i][0];
		m_G[i] = clrChannels[i][1];
		m_B[i] = clrChannels[i][2];
		m_fWidth[i] = clrChannels[i][3];
	}

	Gdiplus::Color colors[16];
	for (int i = 0; i < 16; ++i)
	{
		colors[i].SetFromCOLORREF(RGB(m_R[i], m_G[i], m_B[i]));
		Gdiplus::SolidBrush* brush = ::new Gdiplus::SolidBrush(colors[i]);
		brushes.push_back(brush);
	}

	LOGFONT lf;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
	memcpy(lf.lfFaceName, "宋体", 5);
	lf.lfHeight = 10;
	m_FontText = new Gdiplus::Font(GetDC()->GetSafeHdc(), &lf);

	this->SetBackgroundColor(RGB(clrBK[0], clrBK[1], clrBK[2]), TRUE);
}

void CPosJudgeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPosJudgeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	//ShowMark(m_iCurrentMark);
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPosJudgeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPosJudgeDlg::ExportWounds()
{
	if (m_iExport == 1)//输出到二进制
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/wound.bin";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pFile = fopen(strTemp, "wb");
		fwrite(&m_iWoundCountDetect, 4, 1, pFile);
		fwrite(m_vWounds, sizeof(Wound4Nodejs), m_iWoundCountDetect, pFile);
		fclose(pFile);
	}
	else if (m_iExport == 2)
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/wound.txt";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pFile = fopen(strTemp, "w");
		for (int i = 0; i < m_iWoundCountDetect; ++i)
		{
			fprintf(pFile, "%d,%d,%d\n", m_vWounds[i].Block, m_vWounds[i].Step, m_vWounds[i].Type);
		}
		fclose(pFile);
	}
}

void CPosJudgeDlg::ExportPMs()
{
	if (m_iExport == 1)//输出到二进制
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/mark.bin";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pFile = fopen(strTemp, "wb");
		fwrite(&m_iPMCount, 4, 1, pFile);
		fwrite(m_vPMs, sizeof(PM), m_iPMCount, pFile);
		fclose(pFile);
	}
	else if (m_iExport == 2)
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/mark.txt";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pFile = fopen(strTemp, "w");
		for (int i = 0; i < m_iPMCount; ++i)
		{
			fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d\n", m_vPMs[i].Block, m_vPMs[i].Mark, m_vPMs[i].Walk2, m_vPMs[i].Size, m_vPMs[i].ScrewHoleCount, m_vPMs[i].Manual);
			//fprintf(pFile, "%d,%d,%d,%.6lf,%d,%d,%d\n", m_vPMs[i].Block, m_vPMs[i].Step, m_vPMs[i].Mark, m_vPMs[i].Walk2, m_vPMs[i].Size, m_vPMs[i].ScrewHoleCount, m_vPMs[i].Manual);
		}
		fclose(pFile);

		/*
		CString strFileName2 = m_strExeFolder + "/" + m_strFileName + "/mark_text.txt";
		strTemp = UnicodeToAscii_CSTR(strFileName2);
		pFile = fopen(strTemp, "w");
		for (int i = 0; i < m_iPMCount; ++i)
		{
			if (m_vPMs[i].Mark != PM_SEW_CH && m_vPMs[i].Mark != PM_SEW_LRH && m_vPMs[i].Mark != PM_SEW_LIVE &&
				m_vPMs[i].Mark != PM_JOINT2 && m_vPMs[i].Mark != PM_JOINT4 && m_vPMs[i].Mark != PM_JOINT6 && m_vPMs[i].Mark != PM_JOINT_LEFT && m_vPMs[i].Mark != PM_JOINT_RIGHT && m_vPMs[i].Mark != PM_JOINT
				)
			{
				continue;
			}
			fprintf(pFile, "Block = %d, Mark = %d, Step2 = %d, MiddleStep = %d, Manual = %d,  IsFindWave = %d, DeltStep = %d, Overlapped = %d\n",
				m_vPMs[i].Block, m_vPMs[i].Mark, m_vPMs[i].Step2, m_vPMs[i].MiddleStep, m_vPMs[i].Manual, m_vPMs[i].IsFindWave,
				(int)(m_vPMs[i].BiggerStep - m_vPMs[i].LessStep),
				m_vPMs[i].IsOverlapped
			);
		}
		fclose(pFile);


		CString strFileName3 = m_strExeFolder + "/" + m_strFileName + "/mark_manual.txt";
		strTemp = UnicodeToAscii_CSTR(strFileName3);
		pFile = fopen(strTemp, "w");
		for (int i = 0; i < m_iPMCount2; ++i)
		{
			if (m_vPMs2[i].Mark != PM_SEW_CH && m_vPMs2[i].Mark != PM_SEW_LRH && m_vPMs2[i].Mark != PM_SEW_LIVE && m_vPMs2[i].Mark != PM_SELFDEFINE &&
				m_vPMs2[i].Mark != PM_JOINT2 && m_vPMs2[i].Mark != PM_JOINT4 && m_vPMs2[i].Mark != PM_JOINT6 && m_vPMs2[i].Mark != PM_JOINT_LEFT && m_vPMs2[i].Mark != PM_JOINT_RIGHT && m_vPMs2[i].Mark != PM_JOINT
				)
			{
				continue;
			}
			if (m_vPMs2[i].Mark == PM_SELFDEFINE && m_vPMs2[i].Data < 1 || m_vPMs2[i].Data > 3)
			{
				continue;
			}

			fprintf(pFile, "Block = %d, Mark = %d, Data = %d, Step2 = %d, MiddleStep = %d, Manual = %d,  IsFindWave = %d, DeltStep = %d, Overlapped = %d\n",
				m_vPMs2[i].Block, m_vPMs2[i].Mark, m_vPMs2[i].Data, m_vPMs2[i].Step2, m_vPMs2[i].MiddleStep, m_vPMs2[i].Manual, m_vPMs2[i].IsFindWave,
				(int)(m_vPMs2[i].BiggerStep - m_vPMs2[i].LessStep),
				m_vPMs2[i].IsOverlapped
			);
		}
		fclose(pFile);
		*/
	}
}

void CPosJudgeDlg::ExportFileHead()
{
	if (m_iExport == 1)
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/filehead.bin";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pFile = fopen(strTemp, "wb");
		fprintf(pFile, "%d", m_Head2.direction);
		fclose(pFile);
	}
	else if (m_iExport == 2)
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/filehead.txt";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pfile = fopen(strTemp, "w");

		fprintf(pfile, "%d", m_Head2.direction);
		fclose(pfile);
	}
}

void CPosJudgeDlg::ExportBackAction()
{
	if (m_iExport == 1)
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/back.bin";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pFile = fopen(strTemp, "wb");
		fwrite(&m_iBackCount, 4, 1, pFile);
		fwrite(m_vBA, sizeof(BackAction), m_iBackCount, pFile);
		fclose(pFile);
	}
	else if (m_iExport == 2)
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/back.txt";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pfile = fopen(strTemp, "w");
		for (int i = 0; i < m_iBackCount; ++i)
		{
			if (m_vBA[i].Pos1.Block >= m_iBlockCount)
			{
				continue;
			}

			fprintf(pfile, "%d, %lf\n", m_vBA[i].Pos1.Block, m_vBlockHeads[m_vBA[i].Pos1.Block + 1].Walk - m_vBA[i].Pos1.Walk);
		}
		fclose(pfile);
	}
}

void CPosJudgeDlg::ExportBlockHead(BLOCK_B4Nodejs* pBlocks)
{
	if (m_iExport == 1)
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/block.bin";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pFile = fopen(strTemp, "wb");
		fwrite(&m_iBlockCount, 4, 1, pFile);
		for (int i = 0; i < m_iBlockCount; ++i)
		{
			fwrite(&pBlocks[i], sizeof(BLOCK_B4Nodejs), 1, pFile);
		}
		fclose(pFile);
	}
	else if (m_iExport == 2)
	{
		CString strFileName = m_strExeFolder + "/" + m_strFileName + "/block.txt";
		CStringA strTemp = UnicodeToAscii_CSTR(strFileName);
		FILE* pfile = fopen(strTemp, "w");
		for (int i = 0; i < m_iBlockCount; ++i)
		{
			fprintf(pfile, "%d,%.6lf,%lf,%lf,%d,%d,%d\n", m_vBlockHeads[i].Index, m_vBlockHeads[i].Walk, m_vBlockHeads[i].gpsLog, m_vBlockHeads[i].gpsLat, m_vBlockHeads[i].BlockHead.swNum, m_vBlockHeads[i].BlockHead.railType, m_vBlockHeads[i].BlockHead.railH);
		}
		fclose(pfile);
	}
}

void DoJudge(void* param)
{
	CPosJudgeDlg* dlg = (CPosJudgeDlg*)param;
	BeginAnalyse(dlg->m_iStartMeterIndex);
	uint32_t woundCount = 0, blockCount = 0, currentUseL = 0;
	uint8_t isFinish = 0;
	while (isFinish == 0)
	{
		GetAnalyseProcess(&currentUseL, &blockCount, &woundCount, &isFinish);
		dlg->m_iCurrentBlock = blockCount;
		PostMessage(dlg->GetSafeHwnd(), WM_STEP, 0, 0);
		Sleep(1);
	}
	GetWalk(&dlg->m_minWalk, &dlg->m_maxWalk);
	GetResultCount(&dlg->m_iWoundCountDetect, &dlg->m_iBackCount, &dlg->m_iPMCount, &dlg->m_iBlockCount);
	dlg->m_vWounds = new Wound4Nodejs[dlg->m_iWoundCountDetect];
	dlg->m_vPMs = new Position_Mark_RAW[dlg->m_iPMCount];
	dlg->m_vBA = new	BackAction[dlg->m_iBackCount];
	BLOCK_B4Nodejs* pBlocks = new BLOCK_B4Nodejs[dlg->m_iBlockCount];
	//GetResult(NULL, 0, dlg->m_vWounds, dlg->m_vBA, dlg->m_vPMs, pBlocks);

	GetResult2(NULL, 0, NULL, 0, NULL, 0, NULL, 0, dlg->m_vPMs, &dlg->m_iPMCount, dlg->m_vWounds, &dlg->m_iWoundCountDetect, dlg->m_vBA, &dlg->m_iBackCount, pBlocks, &dlg->m_iBlockCount);
	BlockData_B block;
	for (int i = 0; i < dlg->m_iBlockCount; ++i)
	{
		block.Index = pBlocks[i].Index;
		block.IndexL = pBlocks[i].IndexL;
		block.IndexL2 = pBlocks[i].IndexL2;
		block.AStartPos = pBlocks[i].AStartPos;
		block.BStartPos = pBlocks[i].BStartPos;
		block.Walk = pBlocks[i].Walk;
		block.Walk2 = pBlocks[i].Walk2;
		block.FrameCount = pBlocks[i].FrameCount;
		block.FrameCountRead = pBlocks[i].FrameCountRead;
		block.gpsLat = pBlocks[i].gpsLat;
		block.gpsLog = pBlocks[i].gpsLog;
		block.BlockHead = pBlocks[i].blockHead;
		dlg->m_vBlockHeads.push_back(block);
	}

	uint32_t manualMarkCount, loseCoupleCount, erCount, loseDetectCount;
	GetDQCount(&manualMarkCount, &loseCoupleCount, &erCount, &loseDetectCount);
	dlg->m_iPMCount2 = manualMarkCount;
	if (dlg->m_vPMs2 != nullptr)
		delete[] dlg->m_vPMs2;
	dlg->m_vPMs2 = new Position_Mark_RAW[manualMarkCount];
	GetDQ(dlg->m_vPMs2, nullptr, nullptr, nullptr);


	if (dlg->m_iExport)
	{
		CString strFolder = dlg->m_strExeFolder + "\\" + dlg->m_strFileName;
		CreateDirectory(strFolder, NULL);
		dlg->ExportFileHead();
		dlg->ExportBlockHead(pBlocks);
		dlg->ExportPMs();
		dlg->ExportBackAction();
		dlg->ExportWounds();
	}
	delete pBlocks;

	

#ifndef _DEBUG
	uint8_t isTestEqu = 0;
	int sztWound = 0, sztAlarm = 0;
	GetFileAttr(isTestEqu, sztWound, sztAlarm);
	dlg->m_iWoundCount = sztWound;
	dlg->m_iAlarmCount = sztAlarm;
	dlg->m_strFileType = isTestEqu == 0 ? _T("作业数据") : _T("试块数据");
#endif // !_DEBUG	
	PostMessage(dlg->GetSafeHwnd(), WM_FINISH, 0, 0);
}


void CPosJudgeDlg::ShowBlock(int iBlock)
{
	if (m_bLoaded == FALSE)
	{
		return;
	}
	m_iStep = 0;
	UpdateData(FALSE);

	m_vBlocksToShow.clear();
	m_vSteps.clear();

	if (ReadABData(iBlock) > 0)
	{
		DrawData(); DrawData2();
	}
}

void CPosJudgeDlg::ShowWound(int iWound)
{
	if (m_bLoaded == FALSE)
	{
		return;
	}
	if (iWound < 0 || iWound >= m_iWoundCountDetect)
	{
		return;
	}

	//if (!m_bOpen)
	//{
	//	OpenFileAB();
	//}

	CStringA strA = m_vWounds[iWound].Result;
	CStringW strW = AsciiToUnicode_CSTR(strA);
	m_strRetOld.Format(_T("疑似%s(%s), IsJoint = %d, IsSew = %d, Place = %s, Degree = %s"), strW, g_strWoundDefines[m_vWounds[iWound].Type], m_vWounds[iWound].IsJoint, m_vWounds[iWound].IsSew,
		g_strWoundPlaceDefines[m_vWounds[iWound].Place],
		g_strDegreeDefines[m_vWounds[iWound].Degree]);
	m_iBlock = m_vWounds[iWound].Block + 1;
	m_iStep = m_vWounds[iWound].Step;
	int km = m_vWounds[iWound].Walk;
	double m = m_vWounds[iWound].Walk * 1000 - km * 1000;
	m_strWalk.Format(_T("K%d+%.3lf"), km, m);
	m_strProgress2.Format(_T("%d/%d"), iWound + 1, m_iWoundCountDetect);

	CString according = AsciiToUnicode_CSTR(m_vWounds[iWound].According);
	m_strRetNew = according;
	m_strRetNew.Append(_T("\r\n\r\n") + m_strFileName + _T("\r\n"));
	m_strText.Format(_T("[%d, %d, %d, %d]"), m_vWounds[iWound].Step2, m_vWounds[iWound].StepLeft, m_vWounds[iWound].Row1, m_vWounds[iWound].Row2);
	m_strRetNew.Append(m_strText);
	UpdateData(FALSE);

	int iBlock = m_vWounds[iWound].Block;
	m_iCurrentWound = iWound;

	m_vBlocksToShow.clear();
	Sleep(1);
	m_vSteps.clear();
	Sleep(1);
	if (ReadABData(iBlock) > 0)
	{
		DrawData();
		DrawData2();
	}

	m_oPlot.SetFocus();

	
	/*
	m_woundMySql.testSetID = "4f1b607b-373f-11ea-b924-1831bf303c6c";
	m_woundMySql.oldPath = m_Head2.FileName;
	m_woundMySql.dataPathWithoutAlgversion = m_strDataPathNoVersion;
	m_woundMySql.woundType = m_vWounds[iWound].Type;
	m_woundMySql.step1 = m_vWounds[iWound].Step2;
	m_woundMySql.step2 = m_vWounds[iWound].StepLeft;

	CStringA strAccording = m_vWounds[iWound].According;
	strAccording.Replace("dB", "");
	std::map<char, char> channels;
	for (int i = 0; i < strAccording.GetLength(); ++i)
	{
		if (strAccording.GetAt(i) >= 'A' && strAccording.GetAt(i) <= 'G' || strAccording.GetAt(i) >= 'a' && strAccording.GetAt(i) <= 'c')
		{
			channels[strAccording.GetAt(i)] = 'x';
		}
	}
	strAccording = "";
	for (auto itr = channels.begin(); itr != channels.end(); ++itr)
	{
		strAccording += itr->first;
		strAccording += ",";
	}
	if (channels.size() > 0)
	{
		strAccording = strAccording.Left(strAccording.GetLength() - 1);
	}
	m_woundMySql.channels = (char*)strAccording.GetBuffer(strAccording.GetLength());

	time_t currentTime;
	currentTime = time(NULL);
	tm* tm = localtime(&currentTime);
	char szTime[20] = "";
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	m_woundMySql.lastModifyTime = szTime;


	std::wstring strPlace = g_strWoundPlaceDefines[m_vWounds[iWound].Place];
	m_woundMySql.woundPlace = UnicodeToAscii(strPlace);
	m_woundMySql.recheckLevel = "试块数据";
	m_woundMySql.mi_num = m_vWounds[iWound].Block;

	if (((CButton *)GetDlgItem(IDC_AUTO_INSERT))->GetCheck() == 1)
	{
	//	m_oSql->AddWound(m_woundMySql);
	}
	*/
}



void CPosJudgeDlg::OnBnClickedBtnInsertToDb()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_oSql->AddWound(m_woundMySql);
}


void CPosJudgeDlg::OnBnClickedBtnRemovefromDb()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_oSql->DeleteWound(m_woundMySql);
}


uint32_t CPosJudgeDlg::ReadABData(int iBeginBlock, int halfScreenStep/* = 500*/)
{
	if (m_bReadingB == TRUE)
	{
		return 0;
	}
	m_bReadingB = TRUE;
	m_vBlocksToShow.clear();
	m_vSteps.clear();
	CStringA strA = UnicodeToAscii_CSTR(m_szFileB);
	std::string strFileB(strA);
	BlockData_B	block;

	if (iBeginBlock == 0)
	{
		int i = iBeginBlock;
		while (i < iBeginBlock + 2)
		{
			SolveTPB_C(strFileB, m_iSizeB, m_vBlockHeads[i], block);
			m_vBlocksToShow.push_back(block);
			++i;
		}
	}
	else if (iBeginBlock == m_Head.block - 1)
	{
		int i = iBeginBlock - 1;
		while (i < iBeginBlock + 1)
		{
			SolveTPB_C(strFileB, m_iSizeB, m_vBlockHeads[i], block);
			m_vBlocksToShow.push_back(block);
			++i;
		}
	}
	else
	{
		int i = iBeginBlock - 1;
		while (i < iBeginBlock + 2)
		{
			SolveTPB_C(strFileB, m_iSizeB, m_vBlockHeads[i], block);
			m_vBlocksToShow.push_back(block);
			++i;
		}
	}

	int iStepCount = 0;
	for (int i = 0; i < m_vBlocksToShow.size(); ++i)
	{
		BlockData_B& block = m_vBlocksToShow[i];
		for (int j = 0; j < m_vBlocksToShow[i].vBStepDatas.size(); ++j)
		{
			B_Step& step = m_vBlocksToShow[i].vBStepDatas[j];
			m_vSteps.push_back(step);
		}

		if (m_iMode == 1)
		{
			if (block.Index == m_vWounds[m_iCurrentWound].Block)
			{
				m_iStepIndex = iStepCount + m_vWounds[m_iCurrentWound].Step;
			}
		}

		iStepCount += m_vBlocksToShow[i].BlockHead.row;
	}
	m_bReadingB = FALSE;
	return m_vSteps.size();
 }
//
//uint32_t CPosJudgeDlg::ReadABData2(int centerStep, std::vector<B_Step>& vStep, int halfScreenStep = 500)
//{
//	if (m_bReadingB == TRUE)
//	{
//		return 0;
//	}
//	m_bReadingB = TRUE;
//
//
//	CStringA strA = UnicodeToAscii_CSTR(m_szFileB);
//	std::string strFileB(strA);
//	BlockData_B	block;
//
//	int bs = centerStep - halfScreenStep;
//	int es = centerStep + halfScreenStep;
//
//	Pos pos1 = FindStepInBlock(bs, m_vBlockHeads, 0);
//	Pos pos2 = FindStepInBlock(es, m_vBlockHeads, 0);
//	int i = pos1.Block;
//	VBDB vBlocks;
//	while (i <= pos2.Block && i < m_vBlockHeads.size())
//	{
//		SolveTPB_C(strFileB, m_iSizeB, m_vBlockHeads[i], block);
//		vBlocks.push_back(block);
//		++i;
//	}
//
//
//	int iStepCount = 0;
//	for (int i = 0; i < vBlocks.size(); ++i)
//	{
//		BlockData_B& block = vBlocks[i];
//		for (int j = 0; j < vBlocks[i].vBStepDatas.size(); ++j)
//		{
//			B_Step& step = vBlocks[i].vBStepDatas[j];
//			m_vSteps.push_back(step);
//		}
//		iStepCount += m_vBlocksToShow[i].BlockHead.row;
//	}
//	m_bReadingB = FALSE;
//	return m_vSteps.size();
//}


LRESULT CPosJudgeDlg::ONMessageFinish(WPARAM wParam, LPARAM lParam)
{
	/*
	A_Frame4Nodejs* pFrames = new A_Frame4Nodejs[20000];
	uint32_t frameCount;
	CStringA strA = UnicodeToAscii_CSTR(m_szFileB.Left(m_szFileB.GetLength() - 1) + _T("A"));
	std::string strFileA(strA);
	SolveTPA((char*)strFileA.c_str(), m_vBlockHeads[17].AStartPos, 17, m_vBlockHeads[17].IndexL2, m_iSizeA, pFrames, &frameCount);
	*/

	m_strDataPathNoVersion = m_Head2.NewFileName;
	int lsxIndex = m_strDataPathNoVersion.find_last_of('_');
	m_strDataPathNoVersion = m_strDataPathNoVersion.substr(0, lsxIndex);

	m_strSection.Format(_T("K%.3lf - K%.3lf"), m_minWalk, m_maxWalk);
	m_strTotalWork.Format(_T("%.3lf Km"), 0.001 * m_vBlockHeads.size());

	for (int i = 0; i < g_strWoundDefines.size(); ++i)
	{
		wdCount[i] = 0;
	}

	for (int i = 0; i < m_iWoundCountDetect; ++i)
	{
		++wdCount[mapWoundNameToIndex[g_strWoundDefines[m_vWounds[i].Type]]];
	}

	for (int i = 0; i < g_strWoundDefines.size(); ++i)
	{
		CString strTemp;
		strTemp.Format(_T("%d"), wdCount[i]);
		m_oList.SetItemText(i, 1, strTemp);
	}

	int woundTypes[100] = { 0 }, woundTypesCount[100] = { 0 };
	int woundTypeKinds = 0;

	GetRealTimeAlgSolveDetail(woundTypes, woundTypesCount, &woundTypeKinds);
	
	m_bLoaded = m_iBlockCount > 0;
	KillTimer(TIMER_SOLVINGDATA);

	UpdateData(FALSE);
	this->Invalidate();
	GetDlgItem(IDB_Judge)->EnableWindow();
	GetDlgItem(IDC_BTN_BROWSE)->EnableWindow();

	for (int i = 0; i < m_iBlockCount; ++i)
	{
		if (m_vBlockHeads[i].AStartPos < 0 || m_vBlockHeads[i].FrameCountRead < m_vBlockHeads[i].FrameCount)
		{
			CString strAlarm;
			strAlarm.Format(_T("第%d米块数据A超缺失！"), i + 1);
			if (this->m_bSolveFolder == FALSE)
			{
				AfxMessageBox(strAlarm, MB_OK | MB_ICONWARNING);
			}
			
			break;
		}
	}

	GetResult_CPP(m_fileInfo);

	CString strText = g_strVersion + _T(" - ") + m_Head2.FileName;
	SetWindowText(strText);
	if (m_bSolveFolder == FALSE)
	{
		AfxMessageBox(_T("解析完成！"), MB_OK | MB_ICONINFORMATION);
	}
	m_iCurrentWound = -1;
	m_bSolving = FALSE;
	return 0L;
}

void CPosJudgeDlg::OnBnClickedJudge()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSolveFolder = FALSE; 
	SolveFile();
}



void CPosJudgeDlg::OnBnClickedBrowse()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bShowingDialog == true)
	{
		CWnd* pWnd = FindWindow(NULL, _T("打开"));
		pWnd->BringWindowToTop();
		return;
	}
	OPENFILENAME ofn;
	TCHAR szPath[MAX_PATH];
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szPath;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = _T("B超文件(*.tpB;*.ptB)\0*.tpB;*.ptB\0");
	ofn.nFilterIndex = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	m_bShowingDialog = true;
	if (GetOpenFileName(&ofn))
	{
		m_szFileB = ofn.lpstrFile;
		m_szFileB.Replace('\\', '/');
		UpdateData(FALSE);
	}
	m_bShowingDialog = false;
}

void	CPosJudgeDlg::DrawText(Graphics*g, CString strText, PointF point)
{
	//char *CStr = (char*)(LPCTSTR)strText;
	//size_t len = strText.GetLength() + 1;
	//size_t converted = 0;
	//wchar_t *WStr = (wchar_t*)malloc(len*sizeof(wchar_t));
	//mbstowcs_s(&converted, WStr, len, CStr, _TRUNCATE);
	g->DrawString(strText, strText.GetLength(), m_FontText, point, m_brushText);
	//free(WStr);
}

void CPosJudgeDlg::DrawData()
{
	CRect rc;
	m_oPlot.GetClientRect(&rc);
	int iStepsToDrawCount = m_vSteps.size();

	int iBeginStep = 0, iEndStep = iStepsToDrawCount - 1;

	Gdiplus::Graphics* g = Gdiplus::Graphics::FromHWND(m_oPlot.GetSafeHwnd());
	g->FillRectangle(m_brushBK, rc.left, rc.top, rc.Width(), rc.Height());

	double dx = 1.0 * (rc.right - rc.left) / (iStepsToDrawCount - 1);
	double dy = 1.0 * (rc.bottom - 10 - rc.top) / (65);

	double tx = rc.right - dx * (m_iStepIndex - iBeginStep);
	if (m_iMode == 2)
	{
		tx = 0;
	}

	if (m_vSteps[m_iStepIndex].Mark.Mark & FORK)//是道岔
	{
		PointF pt(tx - 5, 1);
		CString strText;
		int iBlock = m_vWounds[m_iCurrentWound].Block;
		strText.Format(_T("Y-%04X"), m_vBlockHeads[iBlock].BlockHead.swNum);
		//uint8_t		BitS;			// BIT0:道岔右左，BIT1：道岔直曲，BIT2:道岔后前
		if (m_vBlockHeads[iBlock].BlockHead.BitS & BIT0)	strText += _T("左");
		else strText += _T("右");

		if (m_vBlockHeads[iBlock].BlockHead.BitS & BIT1)	strText += _T("曲");
		else strText += _T("直");
		DrawText(g, strText, pt);
	}
	else if (m_vSteps[m_iStepIndex].Mark.Mark & CURVE)
	{
		PointF pt(tx + 20, 1);
		CString strText = _T("$");
		DrawText(g, strText, pt);
	}

	int order[16] = { CH_A1, CH_A2, CH_a1, CH_a2, CH_B1, CH_B2, CH_b1, CH_b2, CH_G, CH_D, CH_E, CH_d, CH_e, CH_C, CH_c, CH_F };
	float yMark = rc.bottom - 20;
	int p = 0;
	vector<B_Step> vSteps;
	for (int j = iBeginStep; j < iEndStep; ++j)
	{
		B_Step & s = m_vSteps[j];
		int x = rc.right - dx * p;
		if (iBeginStep)
		{
		}
		for (int k = 0; k < s.vRowDatas.size(); ++k)
		{
			float y = dy * s.vRowDatas[k].Row + rc.top;
			RectF rcF(x, y, dx, dy);
			//CRect rc2(x, y, x + 1, y + dy);
			for (int m = 0; m < 16; ++m)
			{
				if (s.vRowDatas[k].Point.Draw1 & bits[order[m]])
				{
					g->FillRectangle(brushes[order[m]], rcF);
				}
			}

			uint32_t mark = s.Mark.Mark;
			PointF ptMark((float)x, yMark);
			if (mark & SEW)
			{
				DrawText(g, _T("#"), ptMark);
			}
			else if (mark & SEW2)
			{
				uint16_t data = mark >> 20;
				CString strData;
				if (data == 0)
				{
					strData = "*";
				}
				else
				{
					strData.Format(_T("*%d"), data);
				}
				DrawText(g, strData, ptMark);
			}
			else if (mark & CK_KM)
			{
				DrawText(g, _T("K"), ptMark);
			}

			uint16_t wound = s.Wound.W_Mark;
			if (wound & W_MAN)
			{
				PointF pt1(ptMark.X - 4, ptMark.Y + 7);
				PointF pt2(ptMark.X + 4, ptMark.Y + 7);
				if (wound & W_D_SER)
				{
					GraphicsPath* path = new GraphicsPath();
					path->AddLine(pt1, pt2);
					path->AddLine(pt2, ptMark);
					path->AddLine(ptMark, pt1);
					Gdiplus::Region * reg = new Gdiplus::Region(path);
					g->FillRegion(m_brushWound, reg);
					delete reg;
					delete path;
				}
				else
				{
					g->DrawLine(m_penWound, ptMark, pt1);
					g->DrawLine(m_penWound, pt1, pt2);
					g->DrawLine(m_penWound, pt2, ptMark);
				}
			}
		}
		++p;
	}

	if (m_iMode == 1 && m_iCurrentWound >= 0)
	{
		g->DrawLine(m_penMark, (float)tx, 0.0f, (float)tx, (float)rc.bottom);
	}


	//米块分界线
	int dStep = 0;
	int delt = rc.Width() / 6 - 20;
	for (int i = 0; i < m_vBlocksToShow.size(); ++i)
	{
		dStep += m_vBlocksToShow[i].BlockHead.row;
		tx = rc.right - dx * dStep;
		CString str;
		str.Format(_T("%d"), m_vBlocksToShow[i].Index + 1);
		DrawText(g, str, PointF((float)(tx + delt), (float)(rc.bottom - 20)));
		g->DrawLine(m_penBlockGrid, (float)tx, (float)(rc.bottom - 18), (float)tx, (float)rc.bottom);
	}

	//轨颚线
	int y1 = dy * 13 + rc.top;
	g->DrawLine(m_penBlockGrid, rc.left, y1, rc.right, y1);

	iBeginStep = m_vSteps[0].Step;
	iEndStep = iBeginStep + iStepsToDrawCount - 1;
	for (int i = 0; i < m_iBackCount; ++i)
	{
		if (m_vBA[i].Pos0.Step2 >= iEndStep || m_vBA[i].Pos2.Step2 <= iBeginStep)
		{
			continue;
		}
		int step0 = m_vBA[i].Pos0.Step2;
		if (step0 < iBeginStep)
		{
			step0 = iBeginStep;
		}
		int step1 = m_vBA[i].Pos1.Step2;
		int step2 = m_vBA[i].Pos2.Step2;
		float x1 = rc.right - dx * (step0 - iBeginStep);
		float x2 = rc.right - dx * (step1 - iBeginStep);
		float x3 = rc.right - dx * (step2 - iBeginStep);

		g->DrawLine(m_penBackBlue, x2, (float)(rc.bottom - 6), x1, (float)(rc.bottom - 6));
		g->DrawLine(m_penBackGreen, x3, (float)(rc.bottom - 5), x2, (float)(rc.bottom - 5));
	}

	//伤损框
	for (int i = 0; i < m_iWoundCountDetect; ++i)
	{
		if (m_vWounds[i].Step2 > iEndStep || m_vWounds[i].StepLeft < iBeginStep || m_vWounds[i].Manual != 0)
		{
			continue;
		}
		if (m_vWounds[i].Step2 > iEndStep)

		{
			break;
		}
		RectF rect(rc.right - dx * (m_vWounds[i].StepLeft - iBeginStep + 2), rc.top + (m_vWounds[i].Row1 - 1)*dy, dx * (m_vWounds[i].StepLeft - m_vWounds[i].Step2 + 4), dy * (m_vWounds[i].Row2 - m_vWounds[i].Row1 + 3));
		g->DrawRectangle(m_penWound, rect);
	}
}

void CPosJudgeDlg::DrawData2()
{
	CRect rc;
	m_oPlot2.GetClientRect(&rc);
	int iStepsToDrawCount = m_vSteps.size();

	int iBeginStepIndex = 0, iEndStepIndex = iStepsToDrawCount - 1;
	if (m_iStepIndex > 50)
	{
		iBeginStepIndex = m_iStepIndex - 50;
	}

	if (m_iStepIndex + 50 < iStepsToDrawCount)
	{
		iEndStepIndex = m_iStepIndex + 50;
	}
	iStepsToDrawCount = (iEndStepIndex - iBeginStepIndex);

	Gdiplus::Graphics* g = Gdiplus::Graphics::FromHWND(m_oPlot2.GetSafeHwnd());
	g->FillRectangle(m_brushBK, rc.left, rc.top, rc.Width(), rc.Height());

	double dx = 1.0 * (rc.right - rc.left) / (iStepsToDrawCount - 1);
	double dy = 1.0 * (rc.bottom - rc.top - 10) / (65);

	double tx = rc.right - dx * (m_iStepIndex - iBeginStepIndex);

	int order[16] = { CH_A1, CH_A2, CH_a1, CH_a2, CH_B1, CH_B2, CH_b1, CH_b2, CH_G, CH_D, CH_E, CH_d, CH_e, CH_C, CH_c, CH_F };
	int p = 0;
	for (int j = iBeginStepIndex; j < iEndStepIndex; ++j)
	{
		B_Step & s = m_vSteps[j];
		int x = rc.right - dx * p;
		for (int k = 0; k < s.vRowDatas.size(); ++k)
		{
			int y = rc.top + dy * s.vRowDatas[k].Row;
			RectF rcF((float)x, (float)y, (float)dx, (float)dy);

			for (int m = 0; m < 16; ++m)
			{
				if (s.vRowDatas[k].Point.Draw1 & bits[order[m]])
				{
					g->FillRectangle(brushes[order[m]], rcF);
					//pDC->FillRect(&rc2, m_vBbrush[m]);
				}
			}
		}
		++p;
	}

	g->DrawLine(m_penBlockGrid, PointF(0.0f, (float)(13.0*dy + rc.top)), PointF((float)rc.right, (float)(13.0*dy + rc.top)));

	g->DrawLine(m_penBlockGrid, PointF((float)(rc.left + dx * (m_iStepIndex - iBeginStepIndex)), 0.0f), PointF((float)(rc.left + dx * (m_iStepIndex - iBeginStepIndex)), (float)rc.bottom));


	int iBeginStep = 0, iEndStep = 0;
	iBeginStep = m_vSteps[m_iStepIndex].Step - 50;
	iEndStep = m_vSteps[m_iStepIndex].Step + 50;
	//伤损框
	if (m_iCurrentWound >= 0 && m_iCurrentWound < m_iWoundCountDetect)
	{
		if (m_vWounds[m_iCurrentWound].Step2 > iEndStep || m_vWounds[m_iCurrentWound].StepLeft < iBeginStep)
		{

		}
		else
		{
			RectF rect(rc.right - dx * (m_vWounds[m_iCurrentWound].StepLeft - iBeginStep + 2), rc.top + (m_vWounds[m_iCurrentWound].Row1 - 1)*dy, dx * (m_vWounds[m_iCurrentWound].StepLeft - m_vWounds[m_iCurrentWound].Step2 + 4), dy * (m_vWounds[m_iCurrentWound].Row2 - m_vWounds[m_iCurrentWound].Row1 + 3));
			g->DrawRectangle(m_penWound, rect);
		}
	}
}

void CPosJudgeDlg::OnBnClickedExport()
{


}

void CPosJudgeDlg::OnBnClickedLocate()
{
	// TODO: Add your control notification handler code here
	CDlgLocate dlg;
	if (dlg.DoModal() == IDOK)
	{
		if (m_iMode == 1 && m_vWounds != NULL)
		{
			for (int i = 0; i < m_iWoundCountDetect; ++i)
			{
				if (m_vWounds[i].Block + 1 >= dlg.m_iBlockToLocate)
				{
					m_iCurrentWound = i;
					break;
				}
			}
			ShowWound(m_iCurrentWound);
		}
	}
	m_oPlot.SetFocus();
}

bool CPosJudgeDlg::IsFileExist(CString strFilePath)
{
	int nRet = _access((char*)(LPCTSTR)strFilePath, 0);
	bool bExist = (0 == nRet || EACCES == nRet);
	return bExist;
}

BOOL CPosJudgeDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYUP)
	{
		if (m_bReadingB)
		{
			AfxMessageBox(_T("正在读取上一个位置的数据，请稍侯！"), MB_OK | MB_ICONWARNING);
			return TRUE;
		}
		else
		{
			if (m_bLoaded == FALSE)
			{
				return CDialogEx::PreTranslateMessage(pMsg);
			}
			if (pMsg->wParam == VK_SPACE || pMsg->wParam == VK_LEFT)
			{

				for (int i = 0; i < m_oList.GetItemCount(); ++i)
				{
					uint16_t wt = (uint16_t)m_oList.GetItemData(i);
					int bShow = m_oList.GetCheck(i);
					mapIsObserve[wt] = bShow;
				}

				if (m_iMode == 1)//wound
				{
					++m_iCurrentWound;
					while (m_iCurrentWound < m_iWoundCountDetect && mapIsObserve[m_vWounds[m_iCurrentWound].Type] == 0)
					{
						++m_iCurrentWound;
					}
					if (m_iCurrentWound == m_iWoundCountDetect)
					{
						AfxMessageBox(_T("标定完成！"), MB_OK | MB_ICONINFORMATION);
						--m_iCurrentWound;
						//CloseFileAB();
						GetDlgItem(IDC_BTN_BROWSE)->EnableWindow();
						GetDlgItem(IDB_Judge)->EnableWindow();
					}
					else
					{
						ShowWound(m_iCurrentWound);
					}
					m_strProgress2.Format(_T("%d/%d"), m_iCurrentWound + 1, m_iWoundCountDetect);
				}
				UpdateData(FALSE);
				m_oPlot.SetFocus();
				return TRUE;
			}
			else if (pMsg->wParam == VK_RIGHT)
			{
				for (int i = 0; i < m_oList.GetItemCount(); ++i)
				{
					mapIsObserve[(uint16_t)m_oList.GetItemData(i)] = m_oList.GetCheck(i);
				}
				if (m_iMode == 1)
				{
					--m_iCurrentWound;
					while (m_iCurrentWound >= 0 && mapIsObserve[m_vWounds[m_iCurrentWound].Type] == 0)
					{
						--m_iCurrentWound;
					}
					if (m_iCurrentWound < 0)
					{
						m_iCurrentWound = 0;
					}
					ShowWound(m_iCurrentWound);
					m_strProgress2.Format(_T("%d/%d"), m_iCurrentWound + 1, m_iWoundCountDetect);
					UpdateData(FALSE);
				}
				return TRUE;
			}
		}
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


HBRUSH CPosJudgeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	if (pWnd->GetDlgCtrlID() == IDC_WAVE || pWnd->GetDlgCtrlID() == IDC_WAVE2)
	{
		return  (HBRUSH)::GetStockObject(BLACK_BRUSH);
	}
	else if (pWnd->GetDlgCtrlID() == IDC_R2)
	{
		pDC->SetTextColor(RGB(0xFF, 0, 0));
	}
	return hbr;
}

void CPosJudgeDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == TIMER_SOLVINGDATA)
	{
		if (m_bSolving)
		{
			time_t time2;
			time(&time2);
			int d = difftime(time2, m_tBegin);
			m_strTime.Format(_T("%02d:%02d:%02d"), d / 3600, (d / 60) % 60, d % 60);
			UpdateData(FALSE);
		}
		else
		{

		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CPosJudgeDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}

void CPosJudgeDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	return;
	// TODO: Add your message handler code here
	CPoint pt = point;
	CMenu menu;
	CMenu* PopupMenu = NULL;

	ScreenToClient(&pt);

	//加载菜单
	menu.LoadMenu(IDR_MENU1);

	//子菜单项
	//右键点击 弹出此子菜单项
	PopupMenu = menu.GetSubMenu(0);
	PopupMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, point.x, point.y, this);
	if (m_iMode == 0)
	{
		PopupMenu->CheckMenuItem(ID_WD_CHECK, MF_BYCOMMAND | MF_UNCHECKED);
		PopupMenu->CheckMenuItem(ID_POS_CHECK, MF_BYCOMMAND | MF_CHECKED);
	}
	else if (m_iMode == 1)
	{
		PopupMenu->CheckMenuItem(ID_POS_CHECK, MF_BYCOMMAND | MF_UNCHECKED);
		PopupMenu->CheckMenuItem(ID_WD_CHECK, MF_BYCOMMAND | MF_CHECKED);
	}
}


void CPosJudgeDlg::OnWdCheck()
{
	// TODO: Add your command handler code here
	SetMode(1);
}


void CPosJudgeDlg::OnPosCheck()
{
	// TODO: Add your command handler code here
	SetMode(0);
}




void CPosJudgeDlg::OnUpdateWdCheck(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_iMode == 1);
}


void CPosJudgeDlg::OnUpdatePosCheck(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_iMode == 0);
}


void CPosJudgeDlg::OnBnClickedRevise()
{
	// TODO: Add your control notification handler code here
	/*
	if (m_iCurrentWound >= 0 && m_iCurrentWound < m_vWounds.size())
	{
		m_vWounds[m_iCurrentWound].Type2 = m_vWounds[m_iCurrentWound].Type;
	}
	*/
	VWJ vWounds; VPM vPMs; VBA vBA; VPM vPMs2; VBDB vBlocks; VLC vLC; VER vER; VLD vLD;
	GetResult_C(vWounds, vPMs, vBA, vPMs2, vBlocks, vLC, vER, vLD);
}




void CPosJudgeDlg::OnClickedCheckAll()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	for (int i = 0; i < m_oList.GetItemCount(); ++i)
	{
		m_oList.SetCheck(i, m_bSelectAll);
	}
}


void CPosJudgeDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	wchar_t * pFilePath = new wchar_t[4096];
	::DragQueryFile(hDropInfo, 0, pFilePath, 4096);  // 获取拖放第一个文件的完整文件名
	m_szFileB.Format(_T("%s"), pFilePath);
	m_szFileB.Replace('\\', '/');
	UpdateData(FALSE);
	delete[] pFilePath;

	CDialogEx::OnDropFiles(hDropInfo);
}



void CPosJudgeDlg::OnBnClickedCompare()
{
	// TODO: 在此添加控件通知处理程序代码
	//if (m_vWounds == NULL)
	//{
	//	return;
	//}
	//CString str = _T("");
	//for (int i = 0; i < m_iWoundCount; ++i)
	//{
	//	CString temp;
	//	temp.Format(_T("%d, %d, %d"), m_vWounds[i].Block, m_vWounds[i].Step, m_vWounds[i].Type);
	//	str += temp;
	//}

	//CDlgWoundCompare dlg(str);
	//dlg.DoModal();
}


void TravelFolder(CString strFolder, std::vector<CString>& lstFiles)
{
	CString filename = _T("");
	CString fullname = _T("");

	CFileFind find;

	BOOL IsFind = find.FindFile(strFolder + _T("/*.*"));
	while (IsFind)
	{
		IsFind = find.FindNextFile();
		if (find.IsDots())
		{
			continue;
		}
		else if (find.IsDirectory())
		{
			TravelFolder(find.GetFilePath(), lstFiles);
		}
		else
		{
			filename = find.GetFilePath();
			if (filename.Right(3).MakeLower() == _T("tpb") || filename.Right(3).MakeLower() == _T("ptb"))
			{
				lstFiles.push_back(filename);
			}
		}
	}
}

void CPosJudgeDlg::SolveFile()
{
	if (m_isYoloInit == FALSE)
	{
		char szPath[MAX_PATH] = "";
		CStringA strRetFolderAscii = UnicodeToAscii_CSTR(m_strExeFolder);
		for (int i = 0; i < strRetFolderAscii.GetLength(); ++i)
		{
			szPath[i] = strRetFolderAscii[i];
		}
		AlgInit(szPath, strRetFolderAscii.GetLength());

		this->EnableWindow(FALSE);
		std::string yolocfg = szPath;
		std::string cfgFile, objnames, weightsfile, threshold;
		ifstream ifstream(yolocfg + "/yolo.cfg");
		getline(ifstream, cfgFile);
		getline(ifstream, objnames);
		getline(ifstream, weightsfile);
		getline(ifstream, threshold);
		float thresholdf = atof(threshold.c_str());
		ifstream.close();
		//auto threadFunc = [&]()
		//{
		//int rey = SetYoloFiles((char*)cfgFile.c_str(),
		//	(char*)objnames.c_str(),
		//	(char*)weightsfile.c_str(),
		//	thresholdf);
		//if (rey < 0)
		//{
		//	MessageBoxA(this->m_hWnd, "YOLO初始化失败，请重试！", "系统异常", MB_OK);
		//	//	return FALSE;
		//}
		m_isYoloInit = TRUE;
		this->EnableWindow(TRUE);
	};
	
	if (m_bSolving)
	{
		AfxMessageBox(_T("当前正在解析文件，请稍后再试！"), MB_OK | MB_ICONWARNING);
		return;
	}
	UpdateData(TRUE);
	m_bLoaded = FALSE;
	m_oPlot.SetFocus();
	if (m_szFileB.IsEmpty())
	{
		return;
	}

	m_iBlockCount = 0;
	if (m_vPMs != NULL)
	{
		delete m_vPMs;		m_vPMs = NULL;
	}

	if (m_vWounds != NULL)
	{
		delete m_vWounds;		m_vWounds = NULL;
	}

	if (m_vBA != NULL)
	{
		delete m_vBA;		m_vBA = NULL;
	}
	m_vBlockHeads.clear();
	m_iBlockCount = 0;

	CStringA  strTempB = UnicodeToAscii_CSTR(m_szFileB);
	int32_t ret = SetFile(strTempB.GetBuffer(), strTempB.GetLength(), &m_Head2);
	if (ret < 0)
	{
		CString strErr;
		strErr.Format(_T("%s文件打开失败，请检查文件！"), m_szFileB); 
		if (this->m_bSolveFolder == FALSE)
		{
			AfxMessageBox(strErr, MB_OK | MB_ICONWARNING);
		}
		return;
	}
	m_strFileName = m_Head2.FileName;
	m_iBlockCount = GetBlockCount();

	GetDlgItem(IDC_BTN_BROWSE)->EnableWindow(FALSE);
	GetDlgItem(IDB_Judge)->EnableWindow(FALSE);

	m_bSolving = TRUE;

	time(&m_tBegin);
	SetTimer(TIMER_SOLVINGDATA, 1000, NULL);

	uint32_t addr = 0;
	_beginthreadex(NULL, 0, (unsigned int(__stdcall *)(void *))DoJudge, this, 0, &addr);
}


void CPosJudgeDlg::SolveFolder()
{
	// TODO: 在此添加控件通知处理程序代码
	// TODO: Add your control notification handler code here
	wchar_t szPath[MAX_PATH] = { '0' };     //存放选择的目录路径 
	CString str;

	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));     //作用为给所有参数都设为0，即NULL
	bi.hwndOwner = GetSafeHwnd();   //bi.hwndOwner = m_hWnd;           
	bi.pidlRoot = NULL;    //设置开始搜索位置，为NULL默认从the desktop folder开始
	bi.pszDisplayName = szPath;   //被选中的文件夹缓冲区地址
	bi.lpszTitle = _T("请选择目录：");   // //该浏览文件夹对话框对话框的显示文本，用来提示该浏览文件夹对话框的功能、作用和目的。
	bi.ulFlags = BIF_BROWSEINCLUDEFILES;   //文件也能被选中
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;

	//弹出选择目录对话框
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);  //SHBrowseForFolder用来显示一个让用户可以选择文件夹的对话框，返回值是指向选定的文件夹相对应于所选择根目录地址的路径标识符指针。
	
	if (lp && SHGetPathFromIDList(lp, szPath))
	{
		m_lstFiles.clear();
		TravelFolder(szPath, m_lstFiles);
	}
	

	auto threadFunc = [&]()
	{
		for (int i = 0; i < m_lstFiles.size(); ++i)
		{
			m_szFileB = m_lstFiles[i];
			m_szFileB.Replace('\\', '/');
			UpdateData(FALSE);
			SolveFile();
			while (m_bSolving == TRUE)
			{
				Sleep(5000);
			}
		}
		AfxMessageBox(_T("解析完成！"), MB_OK | MB_ICONINFORMATION);
	};

	std::thread th(threadFunc);
	th.detach();
}


void CPosJudgeDlg::OnBnClickedSolvefolder()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSolveFolder = TRUE;
	SolveFolder();
}

LRESULT CPosJudgeDlg::ONMessageStep(WPARAM wParam, LPARAM lParam)
{
	if (m_iCurrentBlock < m_iBlockCount)
	{
		m_strProgress.Format(_T("%d/%d"), m_iCurrentBlock, m_iBlockCount);
	}
	else
	{
		m_Head.block = m_iBlockCount;
		m_strProgress.Format(_T("%d/%d"), m_iBlockCount, m_iBlockCount);
	}
	UpdateData(FALSE);
	return 0L;
}