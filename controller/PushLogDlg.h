#pragma once
#include "afxdialogex.h"


// CPushLogDlg 对话框

class CPushLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPushLogDlg)

public:
	CPushLogDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPushLogDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PUSH_LOG };
#endif

public:
	CString m_logContent;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_logContentEdit;
	afx_msg void OnBnClickedOk();
};
