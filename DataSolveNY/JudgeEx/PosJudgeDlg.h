
// TLTSDlg.h : 头文件
//

#pragma once
#include <afxcmn.h>
#include "afxwin.h"

#include "ExcelHPE.h"
#include "DataEntities.h"
#include "DataEntitiesForNode.h"
using namespace std;

#include <cstringt.h>

//#include "MySqlHPE.h"


// CPosJudgeDlg 对话框
class CPosJudgeDlg : public CDialogEx
{
	// 构造
public:
	CPosJudgeDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CPosJudgeDlg();

	// 对话框数据
	enum { IDD = IDD_POSJUDGE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL	OnInitDialog();
	afx_msg void	OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void	OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT ONMessageStep(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT ONMessageFinish(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void	OnBnClickedJudge();

	bool			m_bShowingDialog;
	afx_msg void	OnBnClickedBrowse();

	CListCtrl		m_oList;
	CString			m_strBeginTime;
	CString			m_strEndTime;
	CString			m_strProgress;

	CStatic			m_oPlot;
	CStatic			m_oPlot2;

public:
	bool			IsFileExist(CString strFilePath);

	//解析文件
public:
	BOOL			m_bLoaded;
	BOOL			m_bOpen;
	BOOL			m_bSolving;
	int				m_iCurrentBlock;

	F_HEAD			m_Head;
	CString			m_strNewFileName;
	File4Nodejs		m_Head2;
	VBDB			m_vBlockHeads;
	uint32_t		m_iBlockCount;
	bool			m_direction;

	CString			m_strFileFolder;
	CString			m_strFileName;

	FILE*			m_pFileA;
	uint32_t		m_iSizeA;

	CString			m_szFileB;
	FILE*			m_pFileB;
	uint32_t		m_iSizeB;
	VBDB			m_vBlocksToShow;
	vector<B_Step>		m_vSteps;

	double			m_minWalk;
	double			m_maxWalk;

	BOOL			m_bExported;
	void			ExportPMs();
	//void			ExportPMsToExcel(vector<Position_Mark>& vPMs, int count, CString strPath);

	void			ExportWounds();
	//void			ExportWoundsToExcel(vector<Wound_Judged>& vWounds, int count, CString strPath);

	void			ExportFileHead();

	void			ExportBlockHead(BLOCK_B4Nodejs* pBlocks);

	void			ExportBackAction();

	time_t			m_tBegin;
	CString			m_strTime;
	afx_msg void	OnTimer(UINT_PTR nIDEvent);


public:
	int				m_iStepIndex;

	CString			m_strRetOld;
	CString			m_strRetNew;

	BOOL			m_bReadingB;
	void			DrawData();
	void			DrawData2();//画放大波形
	void			DrawText(Gdiplus::Graphics* g, CString strText, PointF point);
	CString			m_strProgress2;

	std::string		m_strNewFileNameBFull;
	uint32_t		ReadABData(int iBeginBlock, int halfScreenStep = 500);

	uint32_t		ReadABData2(int centerStep, std::vector<B_Step>& vStep, int halfScreenStep = 500);
	/*
	0 : 位置标
	1 : 伤损
	2 : 米块
	*/
	BOOL			m_iMode;
	int				m_iBlock;
	int				m_iStep;
public:	
	Position_Mark_RAW*				m_vPMs;
	uint32_t		m_iPMCount;

	Position_Mark_RAW*				m_vPMs2;
	uint32_t		m_iPMCount2;
public:
	Wound4Nodejs*	m_vWounds;
	uint32_t		m_iWoundCountDetect;
	int				m_iWDBlock;
	int				m_iWDStep;
	int32_t			m_iCurrentWound;
	void			ShowWound(int iWound);	

public:
	BackAction*		m_vBA;
	uint32_t		m_iBackCount;

public:
	void			ShowBlock(int iBlock);
	afx_msg void	OnBnClickedLocate();

public:
	CExcelHPE		m_oExcel;
	afx_msg void	OnBnClickedExport();

public:
	/*
	0：不输出
	1：二进制输出
	2：文本输出
	*/
	int				m_iExport;
	void			LoadConfig();
	CString			m_strRetFolder;
	CString			m_strExeFolder;
	CStringA		m_strExeFolderAscii;

	
	CStatic			m_oTips;
	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg void	OnClose();
	virtual void	OnOK();

public:
	void			SetMode(int iMode);
public:

	//CSplitButton m_oSplit;
	afx_msg void	OnWdCheck();
	afx_msg void	OnPosCheck();
	afx_msg void	OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void	OnUpdateWdCheck(CCmdUI *pCmdUI);
	afx_msg void	OnUpdatePosCheck(CCmdUI *pCmdUI);
	CListCtrl		m_oList2;
	CString			m_strWalk;
	int				m_iAlarmCount;
	int				m_iWoundCount;


	afx_msg void	OnBnClickedRevise();
	CString			m_strSection;
	CString			m_strTotalWork;
	//GdiPlus 


public:
	CBrush*			m_BrushBK;
	CPen			m_pen;//画虚线用的
	CPen			m_pen2;
	
	CFont			m_fontTip;
	CFont			m_fontTitle;
	COLORREF		m_clrBK;
	int				m_R[16];
	int				m_G[16];
	int				m_B[16];
	float			m_fWidth[16];

	Gdiplus::Brush*	m_brushBK;
	Gdiplus::Font*	m_FontText;
	Gdiplus::Brush*	m_brushText;
	Gdiplus::Pen*	m_penBlockGrid;
	Gdiplus::Pen*	m_penMark;
	Gdiplus::Image*	m_pImage;

	Gdiplus::Pen*	m_penBackGreen;
	Gdiplus::Pen*	m_penBackBlue;

	Gdiplus::Pen*	m_penWound;
	Gdiplus::Brush* m_brushWound;

	Gdiplus::Pen*	m_penRect;
	BOOL m_bSelectAll;
	afx_msg void	OnClickedCheckAll();
	CString			m_strText;
	afx_msg void	OnDropFiles(HDROP hDropInfo);
	// 是否试块数据
	CString			m_strFileType;
	afx_msg void	OnBnClickedCompare();
	

	SW_FileInfo		m_fileInfo;

public: 
	std::string		m_strDataPathNoVersion;
	//CMySqlHPE*		m_oSql;
	//WoundMySQL		m_woundMySql;
	afx_msg void	OnBnClickedBtnRemovefromDb();
	afx_msg void	OnBnClickedBtnInsertToDb();

public:
	BOOL			m_isYoloInit;
	// 开始解析米块索引
	int		m_iStartMeterIndex;
	void	SolveFile();
	void	SolveFolder();
	afx_msg void OnBnClickedSolvefolder();
	std::vector<CString> m_lstFiles;
	BOOL	m_bSolveFolder;
};


