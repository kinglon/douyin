#include "stdafx.h"
#include "ImPath.h"
#include <shlobj.h>
#include <string>
#include <shlwapi.h>
#include <sys\stat.h>

using namespace std;

std::wstring CImPath::GetDataPath()
{
	static std::wstring wstrPath;
	if (!wstrPath.empty())
	{
		return wstrPath;
	}

	wstrPath = GetAppDataRoamingPath() + L"labelprint\\";
	if (!PathFileExists(wstrPath.c_str()))
	{		
		CreateDirectory(wstrPath.c_str(), NULL);
	}
	
	return wstrPath;
}

std::wstring CImPath::GetCachePath()
{
	static std::wstring wstrPath;
	if (!wstrPath.empty())
	{
		return wstrPath;
	}

	wstrPath = GetSystemTempPath();

	return wstrPath;
}

std::wstring CImPath::GetSoftInstallPath()
{
	static std::wstring wstrPath;
	if (!wstrPath.empty())
	{
		return wstrPath;
	}

	wchar_t wszPath[MAX_PATH];
	memset(wszPath, 0, sizeof(wszPath));
	GetModuleFileName(NULL, wszPath, MAX_PATH);
	wstrPath = std::wstring(wszPath); 
    std::wstring::size_type pos = wstrPath.find_last_of(L"\\");
    wstrPath = wstrPath.substr(0, pos)+L"\\";

	return wstrPath;
}

std::wstring CImPath::GetLocalAppDataPath()
{
	TCHAR szPath[MAX_PATH];

	if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)!=S_OK)
	{
		return L"";
	}

	return std::wstring(szPath) + L"\\";
}

std::wstring CImPath::GetAppDataRoamingPath()
{
	TCHAR szPath[MAX_PATH];

	if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath) != S_OK)
	{
		return L"";
	}

	return std::wstring(szPath) + L"\\";
}

std::wstring CImPath::GetSystemTempPath()
{
	TCHAR szPath[MAX_PATH];

	if (GetTempPath(MAX_PATH, szPath) == 0)
	{
		return L"";	
	}
	return std::wstring(szPath);// 默认路径已经带"\"
}

std::wstring CImPath::GetResourcePath()
{
    static std::wstring strResourcePath = L"";
    if (!strResourcePath.empty())
    {
        return strResourcePath;
    }

    strResourcePath = GetSoftInstallPath() + L"resource\\";
    if (PathFileExists(strResourcePath.c_str()))
    {
        return strResourcePath;
    }

    strResourcePath = GetSoftInstallPath() + L"..\\resource\\";
    if (PathFileExists(strResourcePath.c_str()))
    {
        return strResourcePath;
    }

    strResourcePath = L"";
    return strResourcePath;
}

std::wstring CImPath::GetConfPath()
{
    static std::wstring strConfPath = L"";
    if (!strConfPath.empty())
    {
        return strConfPath;
    }

    strConfPath = GetSoftInstallPath() + L"conf\\";
    if (PathFileExists(strConfPath.c_str()))
    {
        return strConfPath;
    }

    strConfPath = GetSoftInstallPath() + L"..\\conf\\";
    if (PathFileExists(strConfPath.c_str()))
    {
        return strConfPath;
    }

    strConfPath = L"";
    return strConfPath;
}

std::wstring CImPath::GetDumpPath()
{
    static std::wstring strDumpPath = L"";
    if (!strDumpPath.empty())
    {
        return strDumpPath;
    }

    strDumpPath = GetSoftInstallPath() + L"dump\\";
    if (PathFileExists(strDumpPath.c_str()))
    {
        return strDumpPath;
    }
    else
    {
        if (CreateDirectory(strDumpPath.c_str(), nullptr))
        {
            return strDumpPath;
        }
        else
        {
            strDumpPath = L"";
            return strDumpPath;
        }
    }
}

std::wstring CImPath::GetSkinRootPath()
{
    static std::wstring strSkinRootPath = L"";
    if (!strSkinRootPath.empty())
    {
        return strSkinRootPath;
    }

    if (PathFileExists((GetSoftInstallPath() + L"resource\\").c_str()))
    {
        strSkinRootPath = L"resource\\";
        return strSkinRootPath;
    }
    else if (PathFileExists((GetSoftInstallPath() + L"..\\resource\\").c_str()))
    {
        strSkinRootPath = L"..\\resource\\";
        return strSkinRootPath;
    }
    else
    {
        return L"";
    }
}