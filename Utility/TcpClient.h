#pragma once

#include <winsock2.h>
#include <string>

class ITcpClientCallback
{
public:
	virtual void OnConnected() {}

	virtual void OnDataArrive(const std::string& data) {}
};

class CTcpClient
{
public:
	CTcpClient();
	~CTcpClient();

public:
	void SetHost(const char* host) { m_host = host; }

	void SetPort(int port) { m_port = port; }

	void SetCallback(ITcpClientCallback* callback) { m_callback = callback; }

	void Start();

	void Stop() { m_isStart = false; }

	void SendData(const std::string& data);

	bool IsConnected() { return m_clientSocket != INVALID_SOCKET; }

private:
	void ThreadProc();

	void ParseData(char* dataBuffer, int& dataLength);

private:
	std::string m_host;

	int m_port = 80;

	ITcpClientCallback* m_callback = nullptr;

	bool m_isStart = false;

	SOCKET m_clientSocket = INVALID_SOCKET;
};

