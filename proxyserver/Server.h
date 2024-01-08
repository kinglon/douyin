#pragma once

#include "TcpServer.h"
#include <map>

class CServer : public ITcpServerCallback
{
public:
	CServer();
	~CServer();

public:
	void Run();

public:
	virtual void OnConnected(SOCKET clientSocket) override;

	virtual void OnDisconnected(SOCKET clientSocket) override;

	virtual void OnDataArrive(SOCKET clientSocket, const std::string& data) override;

private:
	void HandleData(SOCKET clientSocket, const std::string& data);

	void NotifyDouyinClientChange();

private:
	CTcpServer m_tcpServer;

	std::map<SOCKET, std::string> m_dataBuffers;

	std::map<std::string, SOCKET> m_douyinClients;

	std::map<std::string, SOCKET> m_controllerClients;
};

