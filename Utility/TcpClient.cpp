#include "stdafx.h"
#include "TcpClient.h"
#include <future>
#include "ImCharset.h"

#pragma comment(lib, "ws2_32.lib")

CTcpClient::CTcpClient()
{
}


CTcpClient::~CTcpClient()
{
}


void CTcpClient::Start()
{
	if (m_isStart)
	{
		return;
	}

	m_isStart = true;
	std::async(std::launch::async, &CTcpClient::ThreadProc, this);
}

void CTcpClient::ThreadProc()
{
	LOG_INFO(L"tcp client thread begins to run");

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
	{
		LOG_ERROR(L"failed to call WSAStartup, error is %d", WSAGetLastError());
		return;
	}

	while (m_isStart)
	{
		// Create a socket
		m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_clientSocket == INVALID_SOCKET) {
			LOG_ERROR(L"failed to create tcp socket, error is %d", WSAGetLastError());
			return;
		}

		// Connect to the server
		LOG_INFO(L"begin to connect to %s:%d", CImCharset::UTF8ToUnicode(m_host.c_str()).c_str(), m_port);
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr(m_host.c_str());
		serverAddr.sin_port = htons(m_port);
		if (connect(m_clientSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) 
		{
			LOG_ERROR(L"failed to connect to server, error is %d", WSAGetLastError());
			closesocket(m_clientSocket);
			m_clientSocket = INVALID_SOCKET;
			Sleep(5000);
			continue;
		}

		if (m_callback)
		{
			m_callback->OnConnected();
		}
		
		char dataBuffer[64000];
		int dataLength = 0;
		while (m_isStart) {			
			int bytesReceived = recv(m_clientSocket, &dataBuffer[dataLength], sizeof(dataBuffer)-dataLength, 0);
			if (bytesReceived > 0) 
			{
				dataLength += bytesReceived;
				ParseData(dataBuffer, dataLength);
			}
			else if (bytesReceived == 0) 
			{
				LOG_INFO(L"Connection closed by server");
				break;
			}
			else
			{
				LOG_ERROR(L"Error in receiving data, error is %d", WSAGetLastError());
				break;
			}
		}
		
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
	}
}

void CTcpClient::ParseData(char* dataBuffer, int& dataLength)
{
	int offset = 0;
	while (true)
	{
		if (offset + 2 > dataLength)
		{
			break;
		}

		int length = (static_cast<unsigned char>(dataBuffer[offset]) << 8) | static_cast<unsigned char>(dataBuffer[offset+1]);
		if (offset + 2 + length > dataLength)
		{
			break;
		}

		std::string value(&dataBuffer[offset + 2], length);
		LOG_INFO(L"data arrive: %s", CImCharset::UTF8ToUnicode(value.c_str()).c_str());
		if (m_callback)
		{			
			m_callback->OnDataArrive(value);
		}

		offset += 2 + length;
	}

	if (offset > 0)
	{
		memcpy(&dataBuffer, &dataBuffer[offset], dataLength - offset);
		dataLength = dataLength - offset;
	}
}

void CTcpClient::SendData(const std::string& data)
{
	LOG_INFO(L"send data: %s", CImCharset::UTF8ToUnicode(data.c_str()).c_str());

	if (m_clientSocket == INVALID_SOCKET)
	{
		LOG_ERROR(L"failed to send data, not connected");
		return;
	}

	char* dataBuffer = new char[data.length() + 2];
	dataBuffer[0] = static_cast<char>((data.length() >> 8) & 0xFF);
	dataBuffer[1] = static_cast<char>(data.length() & 0xFF);
	memcpy(&dataBuffer[2], data.c_str(), data.length());	
	if (send(m_clientSocket, dataBuffer, data.length()+2, 0) == SOCKET_ERROR)
	{
		LOG_ERROR(L"failed to send data, error: %d, data: %s", WSAGetLastError(), CImCharset::UTF8ToUnicode(data.c_str()).c_str());	
		closesocket(m_clientSocket);
	}
	delete[] dataBuffer;
}