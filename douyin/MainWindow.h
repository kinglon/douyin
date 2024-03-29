#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include "TcpClient.h"

class CAccountItem
{
public:
	std::wstring m_userId;
	
	std::wstring m_userPassword;

	std::wstring m_status;

	std::wstring m_cook;
};

// 加粉
class CFanItem
{
public:
	ULONGLONG m_addFanTimeSec = 0;  // 加粉触发时间，相对于系统启动时间

	std::wstring m_action;  // 加粉行为：关注/回复/私聊/评论成功
};

class CMainWindow : public WindowImplBase, public ITcpClientCallback
{
public:
	CMainWindow();
	~CMainWindow();

public:
	UIBEGIN_MSG_MAP		
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("importAccountBtn"), OnImportAccountBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("activateBtn"), OnActivateBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("mapBtn"), OnMapBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("saveSettingBtn"), OnSaveSettingBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("homePageBtn"), OnHomePageBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("accountPageBtn"), OnAccountPageBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("usagePageBtn"), OnUsagePageBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("settingPageBtn"), OnSettingPageBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("workPageBtn"), OnWorkPageBtn)
		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK, _T("startBtn"), OnStartBtn)
		UIEND_MSG_MAP

protected: //override base
	virtual void InitWindow() override;
	virtual void OnFinalMessage(HWND hWnd) override;
	virtual CDuiString GetSkinFolder() override;
	virtual CDuiString GetSkinFile() override;
	virtual LPCTSTR GetWindowClassName(void) const override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

public: //implement ITcpClientCallback
	virtual void OnConnected() override;
	virtual void OnDataArrive(const std::string& data) override;

private:
	void OnImportAccountBtn(TNotifyUI& msg);
	void OnActivateBtn(TNotifyUI& msg);
	void OnMapBtn(TNotifyUI& msg);
	void OnSaveSettingBtn(TNotifyUI& msg);
	void OnHomePageBtn(TNotifyUI& msg) { ShowPage(L"homePage"); }
	void OnAccountPageBtn(TNotifyUI& msg) { ShowPage(L"accountPage"); }
	void OnUsagePageBtn(TNotifyUI& msg) { ShowPage(L"usagePage"); }
	void OnSettingPageBtn(TNotifyUI& msg) { ShowPage(L"settingPage"); }
	void OnWorkPageBtn(TNotifyUI& msg) { ShowPage(L"workPage"); }
	void OnStartBtn(TNotifyUI& msg);

private:
	void ShowPage(const wchar_t* pageName);

	std::wstring GenerateString(int length);

	void UpdateAccountListUI();

	void Log(const std::wstring& message);

	void OnTcpClientConnected();

	void DataArrive(const std::string& data);

	void SendIdentifier(const std::wstring& identifier);

	void SendKeepAlive();

	void GetPublicIp();

	void AddFan();

	void ChangeAccountStatus(int count);

	std::wstring GetMachineCode();

private:
	std::vector<CAccountItem> m_accountList;

	std::wstring m_logContent;

	int m_fanCount = 0;

	CControlUI* m_currentPage = nullptr;

	bool m_activate = false;

	CTcpClient m_tcpClient;

	std::wstring m_clientId;

	ULONGLONG m_fanPlanTimeSec = 0; // 加粉计划创建时间，单位秒，距离系统启动的时间

	std::vector<CFanItem> m_fanPlan;  // 加粉计划
};