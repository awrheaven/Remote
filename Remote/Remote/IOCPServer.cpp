#include "stdafx.h"
#include "IOCPServer.h"
#include <mstcpip.h>

CRITICAL_SECTION IOCPServer::m_cs = { 0 };
IOCPServer::IOCPServer()
{

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return;
	}
	m_CompletionPortHandle = NULL;
	m_sListenSocket = INVALID_SOCKET;
	m_ListenEventHandle = WSA_INVALID_EVENT;
	m_ListenThreadHandle= INVALID_HANDLE_VALUE;

	m_uMaxConnections = ((CRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection");


	if (m_uMaxConnections == 0)
	{
		m_uMaxConnections = 100;
	}

	InitializeCriticalSection(&m_cs);

	m_ulWorkThreadCount = 0;


	m_bTimeToKill = FALSE;


	m_ulThreadPoolMin = 0;
	m_ulThreadPoolMax = 0;
	m_ulCPULowThreadsHold = 0;
	m_ulCPUHighThreadsHold = 0;
	m_ulCurrentThread = 0;
	m_ulBusyThread = 0;


	m_ulKeepLiveTime = 0;

	//m_hKillEvent = NULL;

	memcpy(m_szPacketFlag, "malin", FLAG_LENGTH);

	m_NotifyProc = NULL;
}


IOCPServer::~IOCPServer()
{
	DeleteCriticalSection(&m_cs);
}

BOOL IOCPServer::StartServer(pfnNotifyProc NotifyProc, USHORT uPort)
{

	m_NotifyProc = NotifyProc;


	m_KillEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_KillEventHandle == NULL)
	{
		return FALSE;
	}
	//创建监听套接字
	m_sListenSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (m_sListenSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	m_ListenEventHandle = WSACreateEvent();
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		closesocket(m_sListenSocket);
		m_sListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	//将事件与套接字进行关联
	INT iReturn = WSAEventSelect(m_sListenSocket, m_ListenEventHandle, FD_ACCEPT);

	if (iReturn == SOCKET_ERROR)
	{
		closesocket(m_sListenSocket);
		m_sListenSocket = INVALID_SOCKET;

		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
		return FALSE;

	}
	//初始化本地
	SOCKADDR_IN	ServerAddr;
	ServerAddr.sin_port = htons(uPort);
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = INADDR_ANY;

	//绑定
	iReturn = bind(m_sListenSocket,
		(sockaddr*)&ServerAddr,
		sizeof(ServerAddr));

	if (iReturn == SOCKET_ERROR)
	{
		int a = GetLastError();
		closesocket(m_sListenSocket);

		m_sListenSocket = INVALID_SOCKET;
		WSACloseEvent(m_ListenEventHandle);

		m_ListenEventHandle = WSA_INVALID_EVENT;


		return FALSE;
	}


	//监听
	iReturn = listen(m_sListenSocket, SOMAXCONN);

	if (iReturn == SOCKET_ERROR)
	{
		closesocket(m_sListenSocket);
		m_sListenSocket = INVALID_SOCKET;
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
		return FALSE;
	}

	//创建监听线程
	m_ListenThreadHandle =
		(HANDLE)CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)ListenThreadProc,
			(void*)this,	      //向Thread回调函数传入this 方便我们的线程回调访问类中的成员    
			0,
			NULL);

	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)
	{
		closesocket(m_sListenSocket);

		m_sListenSocket = INVALID_SOCKET;
		WSACloseEvent(m_ListenEventHandle);

		m_ListenEventHandle = WSA_INVALID_EVENT;
		return FALSE;
	}



	InitialzeIOCP();
}

BOOL IOCPServer::InitialzeIOCP()
{
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
		NULL, 0, 0);
	if (m_CompletionPortHandle == NULL)
	{

		return FALSE;
	}

	if (m_CompletionPortHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);  //获得PC中有几核





	m_ulThreadPoolMin = 1;
	m_ulThreadPoolMax = 16;
	m_ulCPULowThreadsHold = 10;
	m_ulCPUHighThreadsHold = 75;
	m_Cpu.Init();

	ULONG ulWorkThreadCount = m_ulThreadPoolMax;
	HANDLE WorkThreadHandle = NULL;
	//创建工作线程
	for (int i = 0; i < ulWorkThreadCount;i++)
	{
		WorkThreadHandle = (HANDLE)CreateThread(NULL,	             //创建工作线程目的是处理投递到完成端口中的任务			
			0,
			(LPTHREAD_START_ROUTINE)WorkThreadProc,
			(void*)this,
			0,
			NULL);
		if (WorkThreadHandle == NULL)
		{
			CloseHandle(m_CompletionPortHandle);
			return FALSE;
		}

		m_ulWorkThreadCount++;

		CloseHandle(WorkThreadHandle);
	}
	return TRUE;
}
PCONTEXT_OBJECT IOCPServer::AllocateContext()
{
	PCONTEXT_OBJECT ContextObject = NULL;

	CLock cs(m_cs);

	if (m_ContextFreePoolList.IsEmpty() == FALSE)
	{
		ContextObject = m_ContextFreePoolList.RemoveHead();
	}
	else
	{
		ContextObject = new CONTEXT_OBJECT;
	}
	if (ContextObject != NULL)
	{
		ContextObject->InitMember();
	}
	return ContextObject;



}
VOID IOCPServer::RemoveStaleContext(CONTEXT_OBJECT * ContextObject)
{
	CLock cs(m_cs); 

	//在内存中查找
	if (m_ContextConnectionList.Find(ContextObject))
	{
		CancelIo((HANDLE)ContextObject->ClientSocket);
		closesocket(ContextObject->ClientSocket);

		ContextObject->ClientSocket = NULL;
		//判断还有没有异步IO请求在当前套接字上
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject))
		{
			Sleep(0);

		}
		//将该内存结构回收至内存池
		MoveContextToFreePoolList(ContextObject);
	}
}
VOID IOCPServer::MoveContextToFreePoolList(CONTEXT_OBJECT * ContextObject)
{
	CLock cs(m_cs);
	POSITION Pos = m_ContextConnectionList.Find(ContextObject);
	if (Pos)
	{
		ContextObject->InCompressedBuffer.ClearBuffer();
		ContextObject->InDeCompressedBuffer.ClearBuffer();
		ContextObject->OutCompressedBuffer.ClearBuffer();
		memset(ContextObject->szBuffer, 0, sizeof(ContextObject->szBuffer));
		m_ContextFreePoolList.AddTail(ContextObject);
		m_ContextConnectionList.RemoveAt(Pos);
	}
}
BOOL IOCPServer::HandleIO(IOType PacketFlags, PCONTEXT_OBJECT ContextObject, DWORD dwTrans)
{
	BOOL bReturn = FALSE;
	if (PacketFlags == IOInitialize)
	{
		bReturn = OnClientInitialzing(ContextObject, dwTrans);
	}
	if (IORead == PacketFlags)   //WsaResv
	{
		
		bReturn = OnClientReceiving(ContextObject, dwTrans);  //
	}

	if (IOWrite == PacketFlags)  //WsaSend
	{
		
		bReturn = OnClientPostSending(ContextObject, dwTrans);
	}
	return bReturn;
}
BOOL IOCPServer::OnClientInitialzing(PCONTEXT_OBJECT ContextObject,DWORD dwTrans)
{
	//测试
	//send(ContextObject->ClientSocket, "Hello", strlen("Hello"), 0);
	return TRUE;
}
BOOL IOCPServer::OnClientReceiving(PCONTEXT_OBJECT ContextObject, DWORD dwTrans)
{
	CLock cs(m_cs);
	try
	{
		if (dwTrans == 0)  //对方关闭了套接字
		{
			RemoveStaleContext(ContextObject);
			return FALSE;
		}
		//将接收到的数据拷贝到我们自己的内存中wsabuff    8192
		ContextObject->InCompressedBuffer.WriteBuffer((PBYTE)ContextObject->szBuffer, dwTrans);

		while (ContextObject->InCompressedBuffer.GetBufferLength() > HDR_LENGTH)
		{
			char* szPacketFlag[FLAG_LENGTH] = { 0 };
			CopyMemory(szPacketFlag, ContextObject->InCompressedBuffer.GetBuffer(), FLAG_LENGTH);
			//校验包头
			if (memcmp(m_szPacketFlag,szPacketFlag, FLAG_LENGTH) != 0)
			{
				throw "Bad Buffer";
			}
			//获得数据包的总长度
			ULONG ulPackTotalLength = 0;
			CopyMemory(&ulPackTotalLength, ContextObject->InCompressedBuffer.GetBuffer(FLAG_LENGTH), sizeof(ULONG));

			if (ulPackTotalLength && (ContextObject->InCompressedBuffer.GetBufferLength()) >= ulPackTotalLength)
			{
				int a = sizeof(ULONG);
				ULONG ulOriginalLength = 0;
				ContextObject->InCompressedBuffer.ReadBuffer((PBYTE)szPacketFlag, FLAG_LENGTH);

				ContextObject->InCompressedBuffer.ReadBuffer((PBYTE)&ulPackTotalLength, sizeof(ULONG));
				ContextObject->InCompressedBuffer.ReadBuffer((PBYTE)&ulOriginalLength, sizeof(ULONG));
				ULONG ulCompressedLength = ulPackTotalLength - HDR_LENGTH;
				PBYTE CompressedBuffer = new BYTE[ulCompressedLength];
				//解压过后的数据
				PBYTE DeCompressedBuffer = new BYTE[ulOriginalLength];

				if (CompressedBuffer == NULL || DeCompressedBuffer == NULL)
				{
					throw "Bad Allocate";
				}
				//从数据包当前将源数据没有解压读取到pData
				ContextObject->InCompressedBuffer.ReadBuffer(CompressedBuffer, ulCompressedLength);
				//解压数据
				int iReturn = uncompress(DeCompressedBuffer, &ulOriginalLength, CompressedBuffer, ulCompressedLength);
				int b = GetLastError();
				if (iReturn == Z_OK)
				{
					ContextObject->InDeCompressedBuffer.ClearBuffer();
					ContextObject->InCompressedBuffer.ClearBuffer();
					ContextObject->InDeCompressedBuffer.WriteBuffer(DeCompressedBuffer, ulOriginalLength);

					m_NotifyProc(ContextObject);
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
		PostRecv(ContextObject);
	}
	catch (...)
	{

		ContextObject->InCompressedBuffer.ClearBuffer();
		ContextObject->InDeCompressedBuffer.ClearBuffer();

		PostRecv(ContextObject);
	}
	
	return TRUE;
}
VOID IOCPServer::OnClientPreSending(PCONTEXT_OBJECT ContextObject, PBYTE szBuffer, ULONG ulOriginalLength)
{
	if (ContextObject == NULL)
	{
		return;
	}
	try
	{
		if (ulOriginalLength > 0)
		{
			unsigned long ulCompressLength = (double)ulOriginalLength* 1.001 + 12;
			LPBYTE CompressBuffer = new BYTE[ulCompressLength];
			int iReturn = compress(CompressBuffer, &ulCompressLength, (LPBYTE)szBuffer, ulOriginalLength);
			if (iReturn != Z_OK)
			{
				delete[] CompressBuffer;
				return;
			}

			ULONG ulPackTotalLength = ulCompressLength + HDR_LENGTH;
			ContextObject->OutCompressedBuffer.WriteBuffer((LPBYTE)m_szPacketFlag, FLAG_LENGTH);
			ContextObject->OutCompressedBuffer.WriteBuffer((LPBYTE)&ulPackTotalLength, sizeof(ULONG));
			ContextObject->OutCompressedBuffer.WriteBuffer((LPBYTE)&ulOriginalLength, sizeof(ULONG));
			ContextObject->OutCompressedBuffer.WriteBuffer(CompressBuffer, ulCompressLength);
			delete[] CompressBuffer;
		}
		OVERLAPPEDPLUS* OverlappedPlus = new OVERLAPPEDPLUS(IOWrite);
		PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (DWORD)ContextObject, &OverlappedPlus->m_ol);
		
	}
	catch (...){}
	
	
}
BOOL IOCPServer::OnClientPostSending(PCONTEXT_OBJECT ContextObject, ULONG ulCompletedLength)
{
	try
	{
		DWORD ulFlags = MSG_PARTIAL;
		//移除已经完成后的数据
		ContextObject->OutCompressedBuffer.RemoveComletedBuffer(ulCompletedLength);
		
		if (ContextObject->OutCompressedBuffer.GetBufferLength() == 0)
		{
			ContextObject->OutCompressedBuffer.ClearBuffer();
			return FALSE;
		}
		else
		{
			OVERLAPPEDPLUS* OverlappedPlus = new OVERLAPPEDPLUS(IOWrite);
			ContextObject->wsaOutBuffer.buf = (char*)ContextObject->OutCompressedBuffer.GetBuffer();
			ContextObject->wsaOutBuffer.len = ContextObject->OutCompressedBuffer.GetBufferLength();

			int iOk = WSASend(ContextObject->ClientSocket,
				&ContextObject->wsaOutBuffer,
				1,
				&ContextObject->wsaOutBuffer.len,
				ulFlags,
				&OverlappedPlus->m_ol,
				NULL);
			int a = WSAGetLastError();
			if (iOk == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				RemoveStaleContext(ContextObject);
			}
		}
	}
	catch (...) {}

	return TRUE;

}
VOID IOCPServer::PostRecv(CONTEXT_OBJECT * ContextObject)
{
	OVERLAPPEDPLUS* OverlappedPlus = new OVERLAPPEDPLUS(IORead);

	DWORD dwReturn = 0;
	ULONG ulFlags = MSG_PARTIAL;
	int iReturn = WSARecv(ContextObject->ClientSocket,
		&ContextObject->wsaInBuf,
		1,
		&dwReturn,
		&ulFlags,
		&OverlappedPlus->m_ol,
		NULL);

	if (iReturn == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		RemoveStaleContext(ContextObject);
	}
}
/************************************************************************/
/* 函数 ListenThreadProc()监听线程                                      */
/************************************************************************/
DWORD IOCPServer::ListenThreadProc(LPVOID lParam)
{
	IOCPServer* This = (IOCPServer*)lParam;
	WSANETWORKEVENTS NetWorkEvents;
	while (TRUE)
	{
		///WAIT_OBJECT_0 0x00000000 ：指定的对象出有有信号状态
		if (WaitForSingleObject(This->m_KillEventHandle,100) == WAIT_OBJECT_0)
		{
			break;
		}
		UINT32 ReturnLength = 0;

		ReturnLength = WSAWaitForMultipleEvents(1,
			&This->m_ListenEventHandle,
			FALSE,
			100,
			FALSE);
		if (ReturnLength == WSA_WAIT_TIMEOUT)
		{
			continue;
		}

		///如果事件授信 我们就将该事件转换成一个网络事件 进行 判断
		ReturnLength = WSAEnumNetworkEvents(This->m_sListenSocket,
			This->m_ListenEventHandle,
			&NetWorkEvents);

		if (ReturnLength == SOCKET_ERROR)
		{
			break;
		}
		if (NetWorkEvents.lNetworkEvents & FD_ACCEPT)
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				///进行消息处理
				This->OnAccept();

			}
			else
			{
				break;
			}
		}
	}
	return 0;
}

DWORD IOCPServer::WorkThreadProc(LPVOID lParam)
{

	IOCPServer* This = (IOCPServer*)lParam;
	DWORD    dwTrans = 0;

	HANDLE CompletionPortHandle = This->m_CompletionPortHandle;
	PCONTEXT_OBJECT  ContextObject = NULL;
	LPOVERLAPPED     Overlapped = NULL;
	OVERLAPPEDPLUS*  OverlappedPlus = NULL;
	ULONG            ulBusyThread = 0;
	BOOL             bError = FALSE;


	InterlockedIncrement(&This->m_ulCurrentThread);
	InterlockedIncrement(&This->m_ulBusyThread);

	while (This->m_bTimeToKill == FALSE)
	{
		InterlockedDecrement(&This->m_ulBusyThread);
		BOOL bOk = GetQueuedCompletionStatus(CompletionPortHandle,
			&dwTrans,
			(LPDWORD)&ContextObject,
			&Overlapped, INFINITE);

		DWORD dwIoError = GetLastError();

		OverlappedPlus = CONTAINING_RECORD(Overlapped, OVERLAPPEDPLUS, m_ol);
		ulBusyThread = InterlockedIncrement(&This->m_ulBusyThread);

		if (!bOk && dwIoError != WAIT_TIMEOUT)   //当对方的套机制发生了关闭                    
		{
			if (ContextObject && This->m_bTimeToKill == FALSE &&dwTrans == 0)
			{
				This->RemoveStaleContext(ContextObject);
			}
			continue;
		}

		if (!bError)
		{
			if (ulBusyThread == This->m_ulCurrentThread)
			{
				if (ulBusyThread < This->m_ulThreadPoolMax)
				{
					if (ContextObject != NULL)
					{

						HANDLE hThread = (HANDLE)CreateThread(NULL,
							0,
							(LPTHREAD_START_ROUTINE)WorkThreadProc,
							(void*)This,
							0,
							NULL);
						InterlockedIncrement(&This->m_ulWorkThreadCount);

						CloseHandle(hThread);
					}
				}
			}

			if (!bError && dwIoError == WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					if (This->m_ulCurrentThread > This->m_ulThreadPoolMin)
					{
						break;
					}
					bError = TRUE;
				}
			}
		}

		if (!bError)
		{
			if (bOk && OverlappedPlus != NULL && ContextObject !=NULL)
			{
				try
				{
					This->HandleIO(OverlappedPlus->m_ioType, ContextObject, dwTrans);
					ContextObject = NULL;
				}
				catch (...){}
				
			}
		}

		if (OverlappedPlus)
		{
			delete OverlappedPlus;
			OverlappedPlus = NULL;
		}
	}
	InterlockedDecrement(&This->m_ulWorkThreadCount);

	InterlockedDecrement(&This->m_ulCurrentThread);
	InterlockedDecrement(&This->m_ulBusyThread);
	return 0;
	
}

 /************************************************************************/
 /* 函数：VOID IOCPServer::OnAccept()                                     */
/*   作用：消息处理                                                       */
 /************************************************************************/
VOID IOCPServer::OnAccept()
{
	SOCKADDR_IN ClinetAddr;
	SOCKET ClientSocket = INVALID_SOCKET;

	int iReturn = 0;
	int iLength = 0;

	iLength = sizeof(SOCKADDR_IN);

	ClientSocket = accept(m_sListenSocket, (sockaddr*)&ClinetAddr,
		&iLength);

	if (ClientSocket == SOCKET_ERROR)
	{
		return;
	}
	//客户端上下背景文对象
	PCONTEXT_OBJECT ContextObject = AllocateContext();


	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		ClientSocket = NULL;
		return;
	}

	ContextObject->ClientSocket = ClientSocket;
	ContextObject->wsaInBuf.buf = (char*)ContextObject->szBuffer;
	ContextObject->wsaInBuf.len = sizeof(ContextObject->szBuffer);

	HANDLE Handle = CreateIoCompletionPort((HANDLE)ClientSocket, m_CompletionPortHandle,
		(DWORD)ContextObject, 0);

	if (Handle != m_CompletionPortHandle)
	{
		delete ContextObject;
		if (ClientSocket != INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
		return;
	}
	//设置套接字的选项卡 Set KeepAlive 开启保活机制 SO_KEEPALIVE 
	//保持连接检测对方主机是否崩溃如果2小时内在此套接口的任一方向都没
	//有数据交换，TCP就自动给对方 发一个保持存活

	m_ulKeepLiveTime = 3;

	const BOOL bKeepAlive = TRUE;
	if (setsockopt(ContextObject->ClientSocket,SOL_SOCKET,SO_KEEPALIVE,
		(char*)&bKeepAlive,sizeof(bKeepAlive)) != 0)
	{
	}

	tcp_keepalive KeepAlive;
	KeepAlive.onoff = 1;// 启用保活
	KeepAlive.keepalivetime = m_ulKeepLiveTime; //超过3分钟没有数据，就发送探测包
	KeepAlive.keepaliveinterval = 1000 * 10;//重试间隔为10秒 Resend if No-Reply

	WSAIoctl(ContextObject->ClientSocket,
		SIO_KEEPALIVE_VALS,
		&KeepAlive,
		sizeof(KeepAlive),
		NULL,
		0,
		(unsigned long*)&KeepAlive,
		0,
		NULL);

	CLock cs(m_cs);
	//插入到我们的内存列表中
	m_ContextConnectionList.AddTail(ContextObject);
	//  IOInitialize用户上线
	OVERLAPPEDPLUS* OverLappedPlus = new OVERLAPPEDPLUS(IOInitialize);

	BOOL bOk = PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (DWORD)ContextObject,
		&OverLappedPlus->m_ol);     //  工作线程

	if (!bOk && GetLastError() != ERROR_IO_PENDING)
	{
		RemoveStaleContext(ContextObject);
		return;
	}
	PostRecv(ContextObject);
}

