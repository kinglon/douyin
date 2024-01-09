#include "stdafx.h"
#include "MainWindow.h"
#include "ImPath.h"
#include "ImCharset.h"
#include "Resource.h"
#include <Commdlg.h>
#include <stdlib.h>
#include <time.h>
#include <shellapi.h>
#include <sstream>
#include <shlwapi.h>
#include "SettingManager.h"

#define WM_TCPCLIENT_CONNECT WM_USER+1
#define WM_TCPCLIENT_DATA_ARRIVE WM_USER+2

#define TIMERID_ADD_FAN 1000

CMainWindow::CMainWindow()
{
	srand((unsigned)time(0));
	m_clientId = GenerateString(10);
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::InitWindow()
{
	__super::InitWindow();

	SetIcon(IDI_DOUYIN);

	m_currentPage = m_PaintManager.FindControl(L"accountPage");

	m_tcpClient.SetCallback(this);
	m_tcpClient.SetHost(CImCharset::UnicodeToUTF8(CSettingManager::Get()->m_serverAddr.c_str()).c_str());
	m_tcpClient.SetPort(80);
	m_tcpClient.Start();
}

void CMainWindow::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	m_tcpClient.Stop();
}

CDuiString CMainWindow::GetSkinFolder()
{
	static std::wstring strSkinRootPath = L"";
	if (!strSkinRootPath.empty())
	{
		return strSkinRootPath.c_str();
	}

	if (PathFileExists((CImPath::GetSoftInstallPath() + L"resource\\").c_str()))
	{
		strSkinRootPath = L"resource\\";
		return strSkinRootPath.c_str();
	}
	else if (PathFileExists((CImPath::GetSoftInstallPath() + L"..\\resource\\").c_str()))
	{
		strSkinRootPath = L"..\\resource\\";
		return strSkinRootPath.c_str();
	}
	else
	{
		return L"";
	}
}

CDuiString CMainWindow::GetSkinFile()
{
	return L"main.xml";
}

LPCTSTR CMainWindow::GetWindowClassName() const
{
	return L"DouYinMain";
}

LRESULT CMainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCLBUTTONDOWN)
	{
		POINT pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		ScreenToClient(GetHWND(), &pt);

		RECT closeBtnRect{ 1061, 0, 1096, 40 };
		if (PtInRect(&closeBtnRect, pt))
		{
			Close();
			return 0L;
		}

		RECT minBtnRect{ 1026, 0, 1061, 40 };
		if (PtInRect(&minBtnRect, pt))
		{
			::ShowWindow(GetHWND(), SW_MINIMIZE);
			return 0L;
		}
	}
	else if (uMsg == WM_NCLBUTTONDBLCLK)
	{
		// 禁止双击放大窗口
		return 0L;
	}
	else if (uMsg == WM_TIMER)
	{
		if (wParam == TIMERID_ADD_FAN)
		{
			AddFan(CSettingManager::Get()->m_addFanCount);
			return 0L;
		}
	}
	else if (uMsg == WM_TCPCLIENT_CONNECT)
	{
		SendIdentifier(m_clientId);
		GetPublicIp();
		return 0L;
	}
	else if (uMsg == WM_TCPCLIENT_DATA_ARRIVE)
	{
		std::string* data = (std::string*)lParam;
		DataArrive(*data);
		delete data;
		return 0L;
	}
		
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CMainWindow::OnConnected()
{
	PostMessage(WM_TCPCLIENT_CONNECT, 0, 0);
}

void CMainWindow::OnDataArrive(const std::string& data)
{
	std::string* lparam = new std::string(data);
	PostMessage(WM_TCPCLIENT_DATA_ARRIVE, 0, (LPARAM)lparam);
}

void CMainWindow::SendIdentifier(const std::wstring& identifier)
{
	if (!m_tcpClient.IsConnected())
	{
		return;
	}

	std::vector<std::wstring> identifiers;
	if (!identifier.empty())
	{
		identifiers.push_back(identifier);
	}
	else
	{
		for (auto& account : m_accountList)
		{
			identifiers.push_back(account.m_userId);
		}
	}

	for (auto& identifier : identifiers)
	{
		std::wstringstream formattedTime;
		formattedTime << L"cmd=identify,ctype=douyin,id=" << identifier;
		std::wstring data = formattedTime.str();
		m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data.c_str()));
	}
}

void CMainWindow::GetPublicIp()
{
	if (!m_tcpClient.IsConnected())
	{
		return;
	}
	m_tcpClient.SendData("cmd=get_public_ip");
}

void CMainWindow::DataArrive(const std::string& data)
{
	std::map<std::string, std::string> result;
	std::istringstream ss(data);
	std::string token;

	while (std::getline(ss, token, ',')) 
	{
		size_t pos = token.find('=');
		if (pos != std::string::npos) 
		{
			std::string key = token.substr(0, pos);
			std::string value = token.substr(pos + 1);
			result[key] = value;
		}
	}

	if (result.find("cmd") == result.end())
	{
		return;
	}

	std::string cmd = result["cmd"];
	if (cmd == "push")
	{
		if (result.find("id") == result.end() || result.find("type") == result.end())
		{
			return;
		}

		std::wstring id = CImCharset::UTF8ToUnicode(result["id"].c_str());
		std::string type = result["type"];
		if (type == "log" && result.find("message") != result.end())
		{
			Log(CImCharset::UTF8ToUnicode(result["message"].c_str()));
		}
		else if (type == "addfan" && result.find("count") != result.end())
		{
			int count = atoi(result["count"].c_str());
			AddFan(count);			
		}
		else if (type == "changestatus" && result.find("status") != result.end())
		{
			for (auto& account : m_accountList)
			{
				if (result["status"] == "0")
				{
					account.m_status = L"正常";
				}
				else
				{
					account.m_status = L"异常";
				}
			}
			UpdateAccountListUI();
		}
	}
	else if (cmd == "get_public_ip")
	{
		if (result.find("ip") == result.end())
		{
			return;
		}
		m_PaintManager.FindControl(L"publicIpLabel")->SetText(CImCharset::UTF8ToUnicode(result["ip"].c_str()).c_str());
	}
}

void CMainWindow::AddFan(int count)
{
	m_fanCount += count;
	m_PaintManager.FindControl(L"fanCountLabel")->SetText(std::to_wstring(m_fanCount).c_str());
	Log(std::wstring(L"加粉") + std::to_wstring(count) + L"个");
}

void CMainWindow::ShowPage(const wchar_t* pageName)
{
	if (m_currentPage)
	{
		m_currentPage->SetVisible(false);
	}

	m_currentPage = m_PaintManager.FindControl(pageName);
	m_currentPage->SetVisible(true);
}

void CMainWindow::OnActivateBtn(TNotifyUI& msg)
{
	CDuiString activateCode = m_PaintManager.FindControl(L"activateCodeEdit")->GetText();
	if (activateCode.IsEmpty())
	{
		::MessageBox(GetHWND(), L"激活码不能为空", L"提示", MB_OK);
	}
	else
	{
		m_activate = true;
		::MessageBox(GetHWND(), L"激活成功", L"提示", MB_OK);
	}
}

void CMainWindow::OnImportAccountBtn(TNotifyUI& msg)
{
	if (!m_activate)
	{
		::MessageBox(GetHWND(), L"请先激活", L"提示", MB_OK);
		return;
	}

	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetHWND();
	ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if (!GetOpenFileName(&ofn))
	{
		return;
	}

	m_accountList.clear();
	int count = CSettingManager::Get()->m_accountCount;
	for (int i = 0; i < count; i++)
	{
		CAccountItem accountItem;
		accountItem.m_userId = GenerateString(10);
		accountItem.m_userPassword = GenerateString(8);
		accountItem.m_status = L"正常";
		accountItem.m_cook = GenerateString(10);
		m_accountList.push_back(accountItem);		
	}
	UpdateAccountListUI();
}

std::wstring CMainWindow::GenerateString(int length)
{
	std::wstring randomString;
	for (int i = 0; i < 10; i++) {
		wchar_t randomChar = rand() % 36; // Generate a random number between 0 and 35
		if (randomChar < 10) {
			randomChar += L'0'; // Convert the number to a digit
		}
		else {
			randomChar = L'a' + (randomChar - 10); // Convert the number to a letter
		}
		randomString += randomChar;
	}
	return randomString;
}

void CMainWindow::UpdateAccountListUI()
{
	CListUI* accountList = (CListUI*)m_PaintManager.FindControl(L"accountList");
	accountList->RemoveAll();
	for (unsigned i = 0; i < m_accountList.size(); ++i)
	{
		CDialogBuilder builder;
		CControlUI* pControl = builder.Create(L"account_item.xml");
		m_PaintManager.FindSubControlByName(pControl, L"indexLabel")->SetText(std::to_wstring(i+1).c_str());
		m_PaintManager.FindSubControlByName(pControl, L"accountLabel")->SetText(m_accountList[i].m_userId.c_str());
		m_PaintManager.FindSubControlByName(pControl, L"passwordLabel")->SetText(m_accountList[i].m_userPassword.c_str());
		m_PaintManager.FindSubControlByName(pControl, L"statusLabel")->SetText(m_accountList[i].m_status.c_str());
		m_PaintManager.FindSubControlByName(pControl, L"cookLabel")->SetText(m_accountList[i].m_cook.c_str());		
		accountList->Add(pControl);
	}
}

void CMainWindow::OnMapBtn(TNotifyUI& msg)
{
	::ShellExecute(NULL, L"open", L"https://api.map.baidu.com/lbsapi/getpoint/index.html", NULL, NULL, SW_SHOWNORMAL);
}

void CMainWindow::OnSaveSettingBtn(TNotifyUI& msg)
{
	::MessageBox(GetHWND(), L"保存成功", L"提示", MB_OK);
}

void CMainWindow::OnStartBtn(TNotifyUI& msg)
{
	if (m_accountList.size() == 0)
	{
		::MessageBox(GetHWND(), L"请先导入账号数据", L"提示", MB_OK);
		return;
	}

	::MessageBox(GetHWND(), L"程序已启动", L"提示", MB_OK);
	Log(L"程序已启动");
	::SetTimer(GetHWND(), TIMERID_ADD_FAN, CSettingManager::Get()->m_addFanIntervalHour * 3600000, nullptr);
}

void CMainWindow::Log(const std::wstring& message)
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	std::wstringstream formattedTime;
	formattedTime << L"[" << time.wYear << L"-"
		<< (time.wMonth < 10 ? L"0" : L"") << time.wMonth << L"-"
		<< (time.wDay < 10 ? L"0" : L"") << time.wDay << L" "
		<< (time.wHour < 10 ? L"0" : L"") << time.wHour << L":"
		<< (time.wMinute < 10 ? L"0" : L"") << time.wMinute << L":"
		<< (time.wSecond < 10 ? L"0" : L"") << time.wSecond << L"]";

	if (!m_logContent.empty())
	{
		m_logContent += L"\r\n";
	}
	m_logContent += formattedTime.str() + L" " + message;
	m_PaintManager.FindControl(L"logEdit")->SetText(m_logContent.c_str());
}