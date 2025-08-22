#pragma once


// CDlgWoundCompare 对话框

class CDlgWoundCompare : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgWoundCompare)

public:
	CDlgWoundCompare(CString strWoundList, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDlgWoundCompare();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WOUND };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_strWoundList;
	afx_msg void OnBnClickedOk();
};
