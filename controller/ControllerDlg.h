
// ControllerDlg.h: 头文件
//

#pragma once

#include <vector>
#include "TcpClient.h"

// CControllerDlg 对话框
class CControllerDlg : public CDialogEx, public ITcpClientCallback
{
// 构造
public:
	CControllerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONTROLLER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	virtual void OnConnected() override;

	virtual void OnDataArrive(const std::string& data) override;

private:
	void DataArrive(const std::string& data);	

	void SendIdentifier();

	std::wstring GenerateString(int length);

	CString GetMachineCode();

private:
	CTcpClient m_tcpClient;

	CString m_clientToControl;

	std::wstring m_id;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:	
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);	
	afx_msg void OnBnClickedButtonAccountError();
	afx_msg void OnBnClickedButtonAddFan();
	CEdit m_accountCountEdit;
	CEdit m_fanMinEdit;
	CEdit m_fanMaxEdit;
	CEdit m_machineCodeEdit;
};
