#include "pch.h"
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
}