// AddFanDlg.cpp: 实现文件
//

#include "pch.h"
#include "controller.h"
#include "afxdialogex.h"
#include "AddFanDlg.h"


// CAddFanDlg 对话框

IMPLEMENT_DYNAMIC(CAddFanDlg, CDialogEx)

CAddFanDlg::CAddFanDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ADD_FAN, pParent)
{

}

CAddFanDlg::~CAddFanDlg()
{
}

void CAddFanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FAN_COUNT, m_fanCountEdit);
}


BEGIN_MESSAGE_MAP(CAddFanDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddFanDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddFanDlg 消息处理程序


void CAddFanDlg::OnBnClickedOk()
{
	CString fanCount;
	m_fanCountEdit.GetWindowText(fanCount);
	if (fanCount.IsEmpty())
	{
		MessageBox(L"增加粉丝数不能为空", L"提示", MB_OK);
		return;
	}

	m_fanCount = _ttoi(fanCount);
	CDialogEx::OnOK();
}
