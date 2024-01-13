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
	m_addFanMin = GetPrivateProfileInt(L"setting", L"add_fan_min", 0, confFilePath.c_str());
	m_addFanMax = GetPrivateProfileInt(L"setting", L"add_fan_max", 0, confFilePath.c_str());
}

void CSettingManager::SetAddFanRange(int min, int max)
{
	m_addFanMin = min;
	m_addFanMax = max;

	std::wstring confFilePath = CImPath::GetConfPath() + L"configs.ini";
	WritePrivateProfileString(L"setting", L"add_fan_min", std::to_wstring(m_addFanMin).c_str(), confFilePath.c_str());
	WritePrivateProfileString(L"setting", L"add_fan_max", std::to_wstring(m_addFanMax).c_str(), confFilePath.c_str());
}