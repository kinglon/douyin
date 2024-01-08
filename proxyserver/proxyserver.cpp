// proxyserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "LogUtil.h"
#include "DumpUtil.h"
#include "ImPath.h"
#include "Server.h"

CLogUtil* g_dllLog = nullptr;

int _tmain(int argc, _TCHAR* argv[])
{
	g_dllLog = CLogUtil::GetLog(L"main");
	CServer server;
	server.Run();
	return 0;
}

