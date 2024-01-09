
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

	void UpdateClientListUI(const std::vector<std::wstring>& clients);

	void SendLog(const std::wstring& content);

	void ChangeStatus(bool ok);

private:
	CTcpClient m_tcpClient;

	CString m_clientToControl;

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
	CButton m_controlAllCheckBox;
	CListCtrl m_clientList;
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMRClickListClient(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAccountOk();
	afx_msg void OnAccountError();
	afx_msg void OnAddFan();
	afx_msg void OnFollow();
	afx_msg void OnReply();
	afx_msg void OnPrivateChat();
	afx_msg void OnComment();
	afx_msg void OnPushLog();
};
