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
#include <iphlpapi.h>
#include <set>
#include <fstream>

#pragma comment(lib, "IPHLPAPI.lib")

#define WM_TCPCLIENT_CONNECT WM_USER+1
#define WM_TCPCLIENT_DATA_ARRIVE WM_USER+2

#define TIMERID 1000

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
		if (wParam == TIMERID)
		{
			AddFan();
			return 0L;
		}
	}
	else if (uMsg == WM_TCPCLIENT_CONNECT)
	{
		OnTcpClientConnected();
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

void CMainWindow::OnTcpClientConnected()
{
	m_PaintManager.FindControl(L"connectionStatusLabel")->SetText(L"运行");

	if (m_clientId.empty()) 
	{
		m_clientId = GetMachineCode();
		m_PaintManager.FindControl(L"machineCodeLabel")->SetText(m_clientId.c_str());
	}
	SendIdentifier(m_clientId);

	GetPublicIp();

	::SetTimer(GetHWND(), TIMERID, 1000, nullptr);
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
		if (result.find("type") == result.end())
		{
			return;
		}
		
		std::string type = result["type"];
		if (type == "log" && result.find("message") != result.end())
		{
			Log(CImCharset::UTF8ToUnicode(result["message"].c_str()));
		}
		else if (type == "addfan" && result.find("min") != result.end() && result.find("max") != result.end())
		{
			int min = atoi(result["min"].c_str());
			int max = atoi(result["max"].c_str());
			CSettingManager::Get()->SetAddFanRange(min, max);
			// 清空加粉计划
			m_fanPlanTimeSec = 0;
			m_fanPlan.clear();
		}
		else if (type == "changestatus" && result.find("count") != result.end())
		{
			int count = atoi(result["count"].c_str());
			ChangeAccountStatus(count);
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

void CMainWindow::ChangeAccountStatus(int count)
{
	// 获取正常账号的索引列表
	std::vector<int> accountIndices;
	for (unsigned i = 0; i < m_accountList.size(); i++)
	{
		if (m_accountList[i].m_status == L"正常")
		{
			accountIndices.push_back(i);
		}
	}

	if ((int)accountIndices.size() <= count)
	{
		for (unsigned i = 0; i < accountIndices.size(); i++)
		{
			m_accountList[accountIndices[i]].m_status = L"异常";
		}
		return;
	}

	// 随机选择指定个数
	std::set<int> selectSet;
	int time = 0;
	while (time < 10000)
	{
		time++;
		selectSet.insert(selectSet.end(), rand() % accountIndices.size());
		if ((int)selectSet.size() >= count)
		{
			break;
		}
	}

	for (auto it = selectSet.begin(); it != selectSet.end(); it++)
	{
		m_accountList[accountIndices[*it]].m_status = L"异常";
	}

	UpdateAccountListUI();
}

void CMainWindow::AddFan()
{	
	if (CSettingManager::Get()->m_addFanMax <= 0)
	{
		return;
	}

	// 创建加粉计划
	ULONGLONG now = GetTickCount64() / 1000;
	if (m_fanPlanTimeSec == 0 || now - m_fanPlanTimeSec > 3600)
	{
		m_fanPlanTimeSec = now;
		int min = CSettingManager::Get()->m_addFanMin;
		int count = rand() % (CSettingManager::Get()->m_addFanMax - min + 1) + min;
		std::wstring actions[4] = { L"关注成功", L"回复成功", L"私聊成功", L"评论成功" };
		m_fanPlan.clear();
		for (int i = 0; i < count; i++)
		{
			CFanItem fanItem;
			fanItem.m_addFanTimeSec = m_fanPlanTimeSec + rand() % 3600;
			fanItem.m_action = actions[rand() % ARRAYSIZE(actions)];
			m_fanPlan.push_back(fanItem);
		}

		return;
	}

	// 执行加粉计划
	for (auto it = m_fanPlan.begin(); it != m_fanPlan.end(); it++)
	{
		if (it->m_addFanTimeSec <= now)
		{
			Log(it->m_action);
			m_fanCount++;
			m_PaintManager.FindControl(L"fanCountLabel")->SetText(std::to_wstring(m_fanCount).c_str());
			m_fanPlan.erase(it);
			break;
		}
	}
}

std::wstring CMainWindow::GetMachineCode()
{
	std::wstring machineCode;
	ULONG ulOutBufLen = sizeof (IP_ADAPTER_ADDRESSES);
	PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(ulOutBufLen);

	// Make an initial call to GetAdaptersAddresses to get the necessary size into the ulOutBufLen variable
	if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		free(pAddresses);
		pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(ulOutBufLen);		
	}

	// Make a second call to GetAdaptersAddresses to get the actual data we want
	DWORD dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &ulOutBufLen);
	if (dwRetVal == NO_ERROR) 
	{
		PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
		while (pCurrAddresses) 
		{
			if ((pCurrAddresses->IfType == IF_TYPE_ETHERNET_CSMACD || pCurrAddresses->IfType == IF_TYPE_IEEE80211)
				&& pCurrAddresses->OperStatus == IfOperStatusUp
				&& pCurrAddresses->PhysicalAddressLength > 0)
			{
				for (int i = 0; i < (int)pCurrAddresses->PhysicalAddressLength; i++) 
				{
					wchar_t address[5];
					swprintf(address, L"%.2x", (int)pCurrAddresses->PhysicalAddress[i]);
					machineCode += address;
				}				
				break;
			}
			pCurrAddresses = pCurrAddresses->Next;
		}
	}

	if (pAddresses)
	{
		free(pAddresses);
	}

	return machineCode;
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
	
	std::ifstream file(szFile);
	if (!file.is_open())
	{
		return;
	}

	std::vector<std::wstring> accountList;
	std::string line;
	while (std::getline(file, line)) 
	{
		std::istringstream iss(line);
		std::string token1, token2;
		iss >> token1 >> token2;
		if (token1.empty() || token2.empty())
		{
			LOG_ERROR(L"the account file is invalid.");
			return;
		}
		accountList.push_back(CImCharset::UTF8ToUnicode(token1.c_str()).c_str());
	}
	file.close();

	m_accountList.clear();
	for (unsigned i = 0; i < accountList.size(); i++)
	{
		CAccountItem accountItem;
		accountItem.m_userId = accountList[i];
		accountItem.m_userPassword = L"";
		accountItem.m_status = L"正常";
		accountItem.m_cook = L"";
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

	m_tcpClient.SetCallback(this);
	m_tcpClient.SetHost(CImCharset::UnicodeToUTF8(CSettingManager::Get()->m_serverAddr.c_str()).c_str());
	m_tcpClient.SetPort(80);
	m_tcpClient.Start();

	Log(L"程序已启动");
	::MessageBox(GetHWND(), L"程序已启动", L"提示", MB_OK);
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