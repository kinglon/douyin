#pragma once

#include <string>
#include <shtypes.h>

class CImPath
{
public:
    /**
    @name 获取应用数据路径，以斜杠结尾
    */
	static std::wstring GetDataPath();

    /**
    @name 获取缓存数据路径，以斜杠结尾
    */
	static std::wstring GetCachePath();

    /**
    @name 获取软件安装路径，以斜杠结尾
    */
	static std::wstring GetSoftInstallPath(); 

    /**
    @name 获取系统本地临时文件夹路径%AppData%\\Local
    */
	static std::wstring GetLocalAppDataPath();

	/**
	@name 获取系统本地临时文件夹路径%AppData%\\Roaming
	*/
	static std::wstring GetAppDataRoamingPath();

    /**
    @name 获取资源路径，以斜杠结尾
    */
    static std::wstring GetResourcePath();

    /**
    @name 获取配置路径，以斜杠结尾
    */
    static std::wstring GetConfPath();

    /**
    @name 获取dump路径，以斜杠结尾
    */
    static std::wstring GetDumpPath();

    /**
    @name 获取皮肤文件根目录，以斜杠结尾
    */
    static std::wstring GetSkinRootPath();

private:	
	static std::wstring GetSystemTempPath();
};