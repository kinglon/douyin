// PushLogDlg.cpp: 实现文件
//

#include "pch.h"
#include "controller.h"
#include "afxdialogex.h"
#include "PushLogDlg.h"


// CPushLogDlg 对话框

IMPLEMENT_DYNAMIC(CPushLogDlg, CDialogEx)

CPushLogDlg::CPushLogDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_PUSH_LOG, pParent)
{

}

CPushLogDlg::~CPushLogDlg()
{
}

void CPushLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG_CONTENT, m_logContentEdit);
}


BEGIN_MESSAGE_MAP(CPushLogDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPushLogDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CPushLogDlg 消息处理程序


void CPushLogDlg::OnBnClickedOk()
{
	m_logContentEdit.GetWindowText(m_logContent);
	if (m_logContent.IsEmpty())
	{
		MessageBox(L"日志内容不能为空", L"提示", MB_OK);
		return;
	}

	CDialogEx::OnOK();
}
