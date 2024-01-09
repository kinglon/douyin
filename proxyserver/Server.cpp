#include "stdafx.h"
#include "Server.h"
#include <sstream>
#include "ImCharset.h"
#include <ws2tcpip.h>

CServer::CServer()
{
}


CServer::~CServer()
{
}

void CServer::Run()
{
	m_tcpServer.SetServerPort(80);
	m_tcpServer.SetCallback(this);
	m_tcpServer.Start();

	while (true) 
	{		
		Sleep(INFINITE);
	}
}

void CServer::OnConnected(SOCKET clientSocket)
{
	//
}

void CServer::OnDisconnected(SOCKET clientSocket)
{
	auto it = m_dataBuffers.find(clientSocket);
	if (it != m_dataBuffers.end())
	{		
		m_dataBuffers.erase(it);
	}

	bool isDouyinClient = false;
	for (auto it = m_douyinClients.begin(); it != m_douyinClients.end();)
	{
		if (it->second == clientSocket)
		{
			it = m_douyinClients.erase(it);
			isDouyinClient = true;
			continue;
		}

		it++;
	}
	if (isDouyinClient)
	{
		NotifyDouyinClientChange();
		return;
	}

	for (auto it = m_controllerClients.begin(); it != m_controllerClients.end();)
	{
		if (it->second == clientSocket)
		{
			it = m_controllerClients.erase(it);
			continue;
		}

		it++;
	}
}

void CServer::OnDataArrive(SOCKET clientSocket, const std::string& data)
{
	std::string dataBuffer;
	auto it = m_dataBuffers.find(clientSocket);
	if (it == m_dataBuffers.end())
	{
		dataBuffer = data;
	}
	else
	{
		dataBuffer = it->second + data;
	}

	int dataLength = dataBuffer.length();
	int offset = 0;
	while (true)
	{
		if (offset + 2 > dataLength)
		{
			break;
		}

		int length = (static_cast<unsigned char>(dataBuffer[offset]) << 8) | static_cast<unsigned char>(dataBuffer[offset + 1]);
		if (offset + 2 + length > dataLength)
		{
			break;
		}

		std::string value(&dataBuffer[offset + 2], length);
		LOG_INFO(L"data arrive: %s", CImCharset::UTF8ToUnicode(value.c_str()).c_str());
		HandleData(clientSocket, value);

		offset += 2 + length;
	}

	if (offset < dataLength)
	{
		m_dataBuffers[clientSocket] = dataBuffer.substr(offset);
	}
}

void CServer::HandleData(SOCKET clientSocket, const std::string& data)
{
	std::map<std::string, std::string> result;
	std::istringstream ss(data);
	std::string token;

	while (std::getline(ss, token, ','))
	{
		size_t pos = token.find('=');
		if (pos != std::string::npos)
		{
			std::string key = token.substr(0, pos);
			std::string value = token.substr(pos + 1);
			result[key] = value;
		}
	}

	if (result.find("cmd") == result.end())
	{
		return;
	}

	std::string cmd = result["cmd"];
	if (cmd == "push")
	{
		if (result.find("id") == result.end())
		{
			return;
		}
		
		std::string id = result["id"];
		if (!id.empty())
		{
			auto it = m_douyinClients.find(id);
			if (it == m_douyinClients.end())
			{
				LOG_ERROR(L"failed to push cmd to %s", id.c_str());
				return;
			}

			m_tcpServer.SendData(it->second, data);
		}
		else
		{
			for (auto it = m_douyinClients.begin(); it != m_douyinClients.end(); it++)
			{
				m_tcpServer.SendData(it->second, data);
			}
		}
	}
	else if (cmd == "identify")
	{
		if (result.find("ctype") == result.end()
			|| result.find("id") == result.end())
		{
			return;
		}

		std::string ctype = result["ctype"];
		std::string id = result["id"];
		if (ctype == "douyin")
		{
			m_douyinClients[id] = clientSocket;
			NotifyDouyinClientChange();
		}
		else if (ctype == "controller")
		{
			m_controllerClients[id] = clientSocket;
		}
	}
	else if (cmd == "getdouyinclients")
	{
		std::string clients;
		for (auto& it = m_douyinClients.begin(); it != m_douyinClients.end(); it++)
		{
			if (!clients.empty())
			{
				clients += "_";
			}
			clients += it->first;
		}

		std::string data = "cmd=getdouyinclients,clients=" + clients;
		m_tcpServer.SendData(clientSocket, data);
	}
	else if (cmd == "get_public_ip")
	{
		SOCKADDR_IN clientAddr;
		int clientAddrLen = sizeof(clientAddr);
		getpeername(clientSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);

		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
		std::string data = std::string("cmd=get_public_ip,ip=") + clientIP;
		m_tcpServer.SendData(clientSocket, data);
	}
}

void CServer::NotifyDouyinClientChange()
{
	for (auto it = m_controllerClients.begin(); it != m_controllerClients.end(); it++)
	{
		m_tcpServer.SendData(it->second, "douyinclientchange");
	}
}
