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

	int m_accountCount = 500;

	int m_addFanIntervalHour = 2;

	int m_addFanCount = 12;
};

