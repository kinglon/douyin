#pragma once

class CSettingManager
{
private:
	CSettingManager();
	~CSettingManager();

public:
	static CSettingManager* Get();

	void SetAddFanRange(int min, int max);

private:
	void Load();

public:
	std::wstring m_serverAddr = L"127.0.0.1";

	int m_addFanMin = 0;

	int m_addFanMax = 0;
};

