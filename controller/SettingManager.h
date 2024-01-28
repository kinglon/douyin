#pragma once

class CSettingManager
{
private:
	CSettingManager();
	~CSettingManager();

public:
	static CSettingManager* Get();

private:
	void Load();

public:
	std::wstring m_serverAddr = L"127.0.0.1";
};