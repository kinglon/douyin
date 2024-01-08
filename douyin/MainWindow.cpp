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

#define WM_TCPCLIENT_CONNECT WM_USER+1
#define WM_TCPCLIENT_DATA_ARRIVE WM_USER+2

CMainWindow::CMainWindow()
{
	srand((unsigned)time(0));
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
	m_tcpClient.SetHost("127.0.0.1");
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
	else if (uMsg == WM_TCPCLIENT_CONNECT)
	{
		SendIdentifier(L"");
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
	if (identifier.empty())
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
		formattedTime << L"cmd=identifier,ctype=main,id=" << identifier;
		std::wstring data = formattedTime.str();
		m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data.c_str()));
	}
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

	if (result.find("cmd") == result.end() 
		|| result["cmd"] != "push"
		|| result.find("id") == result.end()
		|| result.find("type") == result.end())
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
		m_fanCount += count;
		m_PaintManager.FindControl(L"fanCountLabel")->SetText(std::to_wstring(m_fanCount).c_str());
	}
	else if (type == "changestatus" && result.find("status") != result.end())
	{
		for (auto& account : m_accountList)
		{
			if (account.m_userId == id)
			{
				if (result["status"] == "0")
				{
					account.m_status = L"正常";
				}
				else
				{
					account.m_status = L"异常";
				}
				break;
			}
		}
		UpdateAccountListUI();
	}
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

	CAccountItem accountItem;
	accountItem.m_userId = GenerateString(10);
	accountItem.m_userPassword = GenerateString(8);
	accountItem.m_status = L"未连接";
	accountItem.m_cook = GenerateString(10);
	m_accountList.push_back(accountItem);
	UpdateAccountListUI();

	SendIdentifier(accountItem.m_userId);
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
	std::vector<CControlUI*> rows;
	rows.push_back(m_PaintManager.FindControl(L"line1"));
	rows.push_back(m_PaintManager.FindControl(L"line2"));
	rows.push_back(m_PaintManager.FindControl(L"line3"));

	std::vector<CControlUI*> seperators;
	seperators.push_back(m_PaintManager.FindControl(L"seperator1"));
	seperators.push_back(m_PaintManager.FindControl(L"seperator2"));
	seperators.push_back(m_PaintManager.FindControl(L"seperator3"));

	for (unsigned i = 0; i < rows.size(); i++)
	{
		if (i >= m_accountList.size())
		{
			rows[i]->SetVisible(false);
			seperators[i]->SetVisible(false);
		}
		else
		{
			m_PaintManager.FindSubControlByName(rows[i], L"accountLabel")->SetText(m_accountList[i].m_userId.c_str());
			m_PaintManager.FindSubControlByName(rows[i], L"passwordLabel")->SetText(m_accountList[i].m_userPassword.c_str());
			m_PaintManager.FindSubControlByName(rows[i], L"statusLabel")->SetText(m_accountList[i].m_status.c_str());
			m_PaintManager.FindSubControlByName(rows[i], L"cookLabel")->SetText(m_accountList[i].m_cook.c_str());
			rows[i]->SetVisible(true);
			seperators[i]->SetVisible(true);
		}
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