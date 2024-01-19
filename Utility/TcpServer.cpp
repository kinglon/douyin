#include "stdafx.h"
#include "TcpServer.h"
#include <future>
#include "ImCharset.h"
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

CTcpServer::CTcpServer()
{
}


CTcpServer::~CTcpServer()
{
}


void CTcpServer::Start()
{
	if (m_isStart)
	{
		return;
	}

	m_isStart = true;
	std::async(std::launch::async, &CTcpServer::ThreadProc, this);
}

void CTcpServer::ThreadProc()
{
	LOG_INFO(L"tcp server thread begins to run");

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
	{
		LOG_ERROR(L"failed to call WSAStartup, error is %d", WSAGetLastError());
		return;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) 
	{
		LOG_ERROR(L"failed to create tcp socket, error is %d", WSAGetLastError());
		return;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	if (m_host.empty())
	{
		serverAddr.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		serverAddr.sin_addr.s_addr = inet_addr(m_host.c_str());
	}
	serverAddr.sin_port = htons(m_port);

	if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
	{
		LOG_ERROR(L"failed to bind tcp socket, error is %d", WSAGetLastError());
		closesocket(serverSocket);
		return;
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) 
	{
		LOG_ERROR(L"failed to listen tcp socket, error is %d", WSAGetLastError());
		closesocket(serverSocket);
		return;
	}

	LOG_INFO(L"Server started, waiting for incoming connections...");

	std::vector<SOCKET> clientSockets;
	char recvBuffer[64000];
	while (m_isStart)
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(serverSocket, &readSet);

		for (SOCKET clientSocket : clientSockets) {
			FD_SET(clientSocket, &readSet);
		}

		int totalSockets = static_cast<int>(clientSockets.size()) + 1;

		int selectResult = select(0, &readSet, NULL, NULL, NULL);
		if (selectResult == SOCKET_ERROR)
		{
			LOG_ERROR(L"failed to call select, error is %d", WSAGetLastError());
			break;
		}

		if (FD_ISSET(serverSocket, &readSet))
		{
			SOCKET clientSocket = accept(serverSocket, NULL, NULL);
			if (clientSocket == INVALID_SOCKET) 
			{
				LOG_ERROR(L"failed to call accept, error is %d", WSAGetLastError());
			}
			else 
			{
				SOCKADDR_IN clientAddr;
				int clientAddrLen = sizeof(clientAddr);
				getpeername(clientSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
				char clientIP[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
				LOG_INFO(L"client 0x%x connected, ip is %s", clientSocket, CImCharset::UTF8ToUnicode(clientIP).c_str());
				clientSockets.push_back(clientSocket);
				if (m_callback)
				{
					m_callback->OnConnected(clientSocket);
				}
			}
		}

		for (auto it = clientSockets.begin(); it != clientSockets.end();) 
		{
			if (FD_ISSET(*it, &readSet))
			{				
				int recvResult = recv(*it, recvBuffer, sizeof(recvBuffer), 0);
				if (recvResult > 0) 
				{
					if (m_callback)
					{
						m_callback->OnDataArrive(*it, std::string(recvBuffer, recvResult));
					}
				}
				else
				{
					if (recvResult < 0)
					{
						LOG_ERROR(L"client 0x%x recv error: %d", *it, WSAGetLastError());
					}

					LOG_INFO(L"client 0x%x disconnected", *it);
					if (m_callback)
					{
						m_callback->OnDisconnected(*it);
					}
					closesocket(*it);
					it = clientSockets.erase(it);
					continue;
				}				
			}

			++it;
		}
	}

	for (SOCKET clientSocket : clientSockets) 
	{
		closesocket(clientSocket);
	}
	closesocket(serverSocket);
}

void CTcpServer::SendData(SOCKET clientSocket, const std::string& data)
{	
	LOG_INFO(L"send data: %s", CImCharset::UTF8ToUnicode(data.c_str()).c_str());

	char* dataBuffer = new char[data.length() + 2];
	dataBuffer[0] = static_cast<char>((data.length() >> 8) & 0xFF);
	dataBuffer[1] = static_cast<char>(data.length() & 0xFF);
	memcpy(&dataBuffer[2], data.c_str(), data.length());
	if (send(clientSocket, dataBuffer, data.length() + 2, 0) == SOCKET_ERROR)
	{
		LOG_ERROR(L"failed to send data, error: %d, data: %s", WSAGetLastError(), CImCharset::UTF8ToUnicode(data.c_str()).c_str());
	}
	delete[] dataBuffer;
}