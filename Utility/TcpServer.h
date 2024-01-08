#pragma once

#include <winsock2.h>
#include <string>

class ITcpServerCallback
{
public:
	virtual void OnConnected(SOCKET clientSocket) {}

	virtual void OnDisconnected(SOCKET clientSocket) {}

	virtual void OnDataArrive(SOCKET clientSocket, const std::string& data) {}
};

class CTcpServer
{
public:
	CTcpServer();
	~CTcpServer();

public:
	void SetHost(const char* host) { m_host = host; }

	void SetServerPort(int port) { m_port = port; }

	void SetCallback(ITcpServerCallback* callback) { m_callback = callback; }

	void Start();

	void Stop() { m_isStart = false; }

	void SendData(SOCKET clientSocket, const std::string& data);

private:
	void ThreadProc();

private:
	std::string m_host;

	int m_port = 80;

	ITcpServerCallback* m_callback = nullptr;

	bool m_isStart = false;
};

