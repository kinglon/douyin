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
	m_accountCount = GetPrivateProfileInt(L"setting", L"account_count", 200, confFilePath.c_str());
	m_addFanIntervalHour = GetPrivateProfileInt(L"setting", L"add_fan_interval_hour", 1, confFilePath.c_str());
	m_addFanCount = GetPrivateProfileInt(L"setting", L"add_fan_count", 6, confFilePath.c_str());
}