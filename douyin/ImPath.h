#pragma once

#include <string>
#include <shtypes.h>

class CImPath
{
public:
    /**
    @name ��ȡӦ������·������б�ܽ�β
    */
	static std::wstring GetDataPath();

    /**
    @name ��ȡ��������·������б�ܽ�β
    */
	static std::wstring GetCachePath();

    /**
    @name ��ȡ�����װ·������б�ܽ�β
    */
	static std::wstring GetSoftInstallPath(); 

    /**
    @name ��ȡϵͳ������ʱ�ļ���·��%AppData%\\Local
    */
	static std::wstring GetLocalAppDataPath();

	/**
	@name ��ȡϵͳ������ʱ�ļ���·��%AppData%\\Roaming
	*/
	static std::wstring GetAppDataRoamingPath();

    /**
    @name ��ȡ��Դ·������б�ܽ�β
    */
    static std::wstring GetResourcePath();

    /**
    @name ��ȡ����·������б�ܽ�β
    */
    static std::wstring GetConfPath();

    /**
    @name ��ȡdump·������б�ܽ�β
    */
    static std::wstring GetDumpPath();

    /**
    @name ��ȡƤ���ļ���Ŀ¼����б�ܽ�β
    */
    static std::wstring GetSkinRootPath();

private:	
	static std::wstring GetSystemTempPath();
};