// CDlgWoundCompare.cpp: 实现文件
//

#include "stdafx.h"
#include "CDlgWoundCompare.h"
#include "afxdialogex.h"
#include "PosJudge.h"


// CDlgWoundCompare 对话框

IMPLEMENT_DYNAMIC(CDlgWoundCompare, CDialogEx)

CDlgWoundCompare::CDlgWoundCompare(CString strWoundList, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_WOUND, pParent)
	, m_strWoundList(strWoundList)
{

}

CDlgWoundCompare::~CDlgWoundCompare()
{
}

void CDlgWoundCompare::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strWoundList);
}


BEGIN_MESSAGE_MAP(CDlgWoundCompare, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgWoundCompare::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgWoundCompare 消息处理程序


void CDlgWoundCompare::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}
