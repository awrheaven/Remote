#include "stdafx.h"
#include "IOCPClient.h"
#include <stdlib.h>
#include <mstcpip.h>
#include"zconf.h"
#include "zlib.h"

#define MAX_RECV_BUFFER  1024*8
#define HDR_LENGTH     13
IOCPClient::IOCPClient()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	memcpy(m_szPacketFlag, "malin", FLAG_LENGTH);
	m_EventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_Manager = NULL;
}


IOCPClient::~IOCPClient()
{
	if (m_EventHandle != NULL)
	{
		CloseHandle(m_EventHandle);
		m_EventHandle = NULL;
	}
	if (m_ClientSocket != NULL)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = NULL;

	}
	if (m_WorkThreadHandle != NULL)
	{
		CloseHandle(m_WorkThreadHandle);
		m_WorkThreadHandle = NULL;
	}
}

VOID IOCPClient::setManagerCallBack(CManager * Manager)
{
	m_Manager = Manager;
}


BOOL IOCPClient::ConnectServer(char * szServerIp, unsigned short uPort)
{
	m_ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_ClientSocket == SOCKET_ERROR)
	{
		return FALSE;
	}
	sockaddr_in ServerAddr;
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(uPort);
	ServerAddr.sin_addr.S_un.S_addr = inet_addr(szServerIp);

	if (connect(m_ClientSocket,(SOCKADDR*)&ServerAddr,sizeof(sockaddr_in))== SOCKET_ERROR)
	{
		

		//MessageBoxA(NULL, v1, "2", MB_OK);//测试
		if (m_ClientSocket != INVALID_SOCKET)
		{
			closesocket(m_ClientSocket);
			m_ClientSocket = NULL;
		}
		return FALSE;
	}

	const char chOpt = 1; // True
						  // Set KeepAlive 开启保活机制, 防止服务端产生死连接
	if (setsockopt(m_ClientSocket, SOL_SOCKET, SO_KEEPALIVE,
		(char *)&chOpt, sizeof(chOpt)) == 0)
	{
		// 设置超时详细信息
		tcp_keepalive	klive;
		klive.onoff = 1; // 启用保活
		klive.keepalivetime = 1000 * 60 * 3; // 3分钟超时 Keep Alive
		klive.keepaliveinterval = 1000 * 5;  // 重试间隔为5秒 Resend if No-Reply
		WSAIoctl
		(
			m_ClientSocket,
			SIO_KEEPALIVE_VALS,
			&klive,
			sizeof(tcp_keepalive),
			NULL,
			0,
			(unsigned long *)&chOpt,
			0,
			NULL
		);
	}

	m_WorkThreadHandle = (HANDLE)CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThreadProc, (LPVOID)this, 0, NULL);

	return TRUE;

}

VOID IOCPClient::DisConnect()
{
	CancelIo((HANDLE)m_ClientSocket);
	InterlockedExchange((LPLONG)&m_bIsRunning, FALSE);
	closesocket(m_ClientSocket);

	SetEvent(m_EventHandle);
	m_ClientSocket = INVALID_SOCKET;
}

VOID IOCPClient::OnServerReceiving(char * szBuffer, ULONG ulLength)
{
	try
	{
		if (ulLength == 0)
		{
			DisConnect();
			return;
		}

		//将接到的数据进行压缩
		m_CompressedBuffer.WriteBuffer((LPBYTE)szBuffer, ulLength);
		// 检测数据是否大于数据头 
		while (m_CompressedBuffer.GetBufferLength() > HDR_LENGTH)
		{
			char szPacketFlag[FLAG_LENGTH] = { 0 };
			CopyMemory(szPacketFlag, m_CompressedBuffer.GetBuffer(), FLAG_LENGTH);
			if (memcmp(m_szPacketFlag,szPacketFlag, FLAG_LENGTH) != 0)
			{
				throw "Bab Buffer";
			}
			ULONG ulPackTotalLength = 0;

			CopyMemory(&ulPackTotalLength, m_CompressedBuffer.GetBuffer(FLAG_LENGTH),
				sizeof(ULONG));
			if (ulPackTotalLength && 
				(m_CompressedBuffer.GetBufferLength())>= ulPackTotalLength)
			{
				//读取数据头
				m_CompressedBuffer.ReadBuffer((PBYTE)szPacketFlag, FLAG_LENGTH);
				//读取数据的总长度
				m_CompressedBuffer.ReadBuffer((PBYTE)&ulPackTotalLength, sizeof(ULONG));
				//读取数据原长度
				ULONG ulOriginalLength = 0;
				m_CompressedBuffer.ReadBuffer((PBYTE)&ulOriginalLength, sizeof(ULONG));

				ULONG ulCompressedLength = ulPackTotalLength - HDR_LENGTH;
				//申请解压后存储buffer的缓冲区
				PBYTE CompressedBuffer = new BYTE[ulCompressedLength];
				PBYTE DeCompressedBuffer = new BYTE[ulOriginalLength];
				if (CompressedBuffer == NULL || DeCompressedBuffer == NULL)
				{
					throw"bad Allocate";
				}
				m_CompressedBuffer.ReadBuffer(CompressedBuffer, ulCompressedLength);
				int iReturn = uncompress(DeCompressedBuffer, &ulOriginalLength,
					CompressedBuffer, ulCompressedLength);
				//解压成功
				if (iReturn == Z_OK)
				{
					m_DeCompressedBuffer.ClearBuffer();
					m_DeCompressedBuffer.WriteBuffer(DeCompressedBuffer, ulOriginalLength);


					m_Manager->OnReceive((PBYTE)m_DeCompressedBuffer.GetBuffer(0),
						m_DeCompressedBuffer.GetBufferLength());
				}
				else
				{
					throw "Bad Buffer";
				}
				delete[] CompressedBuffer;
				delete[] DeCompressedBuffer;
			}
			else
			{
				break;
			}
		}
	}
	catch (...)
	{
		m_CompressedBuffer.ClearBuffer();
	}
}

INT IOCPClient::OnServerSending(char * szBuffer, ULONG ulOriginalLength)
{
	m_WriteBuffer.ClearBuffer();

	if (ulOriginalLength > 0)
	{
		unsigned long ulCompressedLength = (double)ulOriginalLength * 1.001 + 12;
		LPBYTE CompressedBuffer = new BYTE[ulCompressedLength];

		if (CompressedBuffer == NULL)
		{
			return 0;
		}
		int iReturn = compress(CompressedBuffer, &ulCompressedLength, (PBYTE)szBuffer, ulOriginalLength);

		if (iReturn != Z_OK)
		{
			delete[] CompressedBuffer;
			return 0;

		}
		ULONG ulPackTotalLength = ulCompressedLength + HDR_LENGTH;
		m_WriteBuffer.WriteBuffer((PBYTE)m_szPacketFlag, sizeof(m_szPacketFlag));
		m_WriteBuffer.WriteBuffer((PBYTE)&ulPackTotalLength, sizeof(ULONG));

		m_WriteBuffer.WriteBuffer((PBYTE)&ulOriginalLength, sizeof(ULONG));

		m_WriteBuffer.WriteBuffer(CompressedBuffer, ulCompressedLength);

		delete[] CompressedBuffer;
		CompressedBuffer = NULL;

	}
	
	return SendWithSplit((char*)m_WriteBuffer.GetBuffer(), 
		m_WriteBuffer.GetBufferLength(), MAX_SEND_BUFFER);
}

BOOL IOCPClient::SendWithSplit(char * szBuffer, ULONG ulLength, ULONG ulSplitLength)
{
	int         iReturn     = 0;   //实际发送的数据
	const char* Travel      = (char*)szBuffer;
	int         i           = 0;
	int         j           = 0;
	ULONG       ulSended    = 0;
	ULONG       ulSendRetry = 15;
	

	for (i = ulLength ; i >= ulSplitLength; i -= ulSplitLength)
	{
		for (j = 0; j < ulSendRetry;j++)
		{
			iReturn = send(m_ClientSocket, Travel, ulSplitLength, 0);
			if (iReturn > 0)
			{
				break;
			}
		}
		if (j == ulSendRetry)
		{
			return FALSE;
		}
		ulSended += iReturn;
		Travel += ulSplitLength;
		Sleep(15);
	}
	if (i>0)
	{
		for (int j = 0; j < ulSendRetry;j++)
		{
			iReturn = send(m_ClientSocket, (char*)Travel, i, 0);
			Sleep(15);
			if (iReturn > 0)
			{
				
				break;
			}
			if (j == ulSendRetry)
			{
				return FALSE;
			}
			ulSended += iReturn;
		}
	}
	if (ulSended == ulLength)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID IOCPClient::RunEventLoop()
{
	WaitForSingleObject(m_EventHandle,INFINITE);
}

DWORD WINAPI IOCPClient::WorkThreadProc(LPVOID lParam)
{
	IOCPClient* This = (IOCPClient*)lParam;
	char szBuffer[MAX_RECV_BUFFER] = { 0 };

	fd_set fdOld;
	fd_set fdNew;

	FD_ZERO(&fdOld);
	FD_ZERO(&fdNew);
	FD_SET(This->m_ClientSocket, &fdOld);
	while (This->IsRunning())
	{
		fdNew = fdOld;
		int iReturn = select(NULL, &fdNew, NULL, NULL, NULL);
		int b = GetLastError();
		if (iReturn == SOCKET_ERROR)
		{
			This->DisConnect();
			printf("关闭1\r\n");
			break;
		}
		Sleep(100);
		if (iReturn > 0)
		{
			memset(szBuffer, 0, sizeof(szBuffer));
			int iReceivedLength = recv(This->m_ClientSocket,
				szBuffer, sizeof(szBuffer), 0);

			if (iReceivedLength <= 0)
			{
				int a = GetLastError();
				printf("关闭2\r\n");
				This->DisConnect();//接收错误处理
				break;
			}

			if (iReceivedLength > 0)
			{
				This->OnServerReceiving((char*)szBuffer, iReceivedLength);
			}
		}
	}
	return 0;
}
