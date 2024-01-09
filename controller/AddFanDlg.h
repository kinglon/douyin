#pragma once
#include "afxdialogex.h"


// CAddFanDlg 对话框

class CAddFanDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddFanDlg)

public:
	CAddFanDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAddFanDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ADD_FAN };
#endif

public:
	int m_fanCount = 0;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_fanCountEdit;
	afx_msg void OnBnClickedOk();
};
