#include "stdafx.h"
#include "SettingManager.h"
#include "ImPath.h"


CSettingManager::CSettingManager()
{
	Load();
}


CSettingManager::~CSettingManager()
{
}

CSettingManager* CSettingManager::Get()
{
	static CSettingManager* instance = new CSettingManager();
	return instance;
}

void CSettingManager::Load()
{
	std::wstring confFilePath = CImPath::GetConfPath() + L"configs.ini";
	wchar_t serverAddr[MAX_PATH];
	GetPrivateProfileString(L"setting", L"server_addr", L"127.0.0.1", serverAddr, MAX_PATH-1, confFilePath.c_str());
	m_serverAddr = serverAddr;
	m_accountCount = GetPrivateProfileInt(L"setting", L"account_count", 200, confFilePath.c_str());
	m_addFanIntervalHour = GetPrivateProfileInt(L"setting", L"add_fan_interval_hour", 1, confFilePath.c_str());
	m_addFanCount = GetPrivateProfileInt(L"setting", L"add_fan_count", 6, confFilePath.c_str());
}