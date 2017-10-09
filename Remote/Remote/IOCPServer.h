#pragma once

#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include "Buffer.h"
#define PACKET_LENGTH   0x2000
#include "Remote.h"
#include "CpuUsage.h"
#include "zconf.h"
#include"zlib.h"
#define HDR_LENGTH   13
#define FLAG_LENGTH   5
enum IOType
{
	IOInitialize,
	IORead,
	IOWrite,
	IOIdle
};
typedef struct _CONTEXT_OBJECT
{
	SOCKET   ClientSocket;
	WSABUF   wsaInBuf;
	WSABUF	 wsaOutBuffer;
	char     szBuffer[PACKET_LENGTH];
	CBuffer				InCompressedBuffer;	            // 接收到的压缩的数据
	CBuffer				InDeCompressedBuffer;	        // 解压后的数据
	CBuffer             OutCompressedBuffer;
	int				    v1;
	HANDLE              hDlg;

	VOID InitMember()
	{
		memset(szBuffer, 0, sizeof(char)*PACKET_LENGTH);
		v1 = 0;
		hDlg = NULL;
		ClientSocket = INVALID_SOCKET;
		memset(&wsaInBuf, 0, sizeof(WSABUF));
		memset(&wsaOutBuffer, 0, sizeof(WSABUF));
	}


}CONTEXT_OBJECT, *PCONTEXT_OBJECT;



typedef CList<PCONTEXT_OBJECT> ContextOnjectList;
class IOCPServer
{
public:

	typedef void (CALLBACK *pfnNotifyProc)(CONTEXT_OBJECT* ContextObject);
	IOCPServer();
	~IOCPServer();
	BOOL StartServer(pfnNotifyProc NotifyProc, USHORT uPort);
	BOOL InitialzeIOCP();
	//////////////////////////////////////////////////////////////////////////
	PCONTEXT_OBJECT AllocateContext();
	VOID RemoveStaleContext(CONTEXT_OBJECT* ContextObject);
	VOID MoveContextToFreePoolList(CONTEXT_OBJECT* ContextObject);

	//////////////////////////////////////////////////////////////////////////
	VOID OnAccept();
	BOOL HandleIO(IOType PacketFlags, PCONTEXT_OBJECT ContextObject, DWORD dwTrans);
	BOOL OnClientInitialzing(PCONTEXT_OBJECT ContextObject, DWORD dwTrans);
	BOOL OnClientReceiving(PCONTEXT_OBJECT ContextObject, DWORD dwTrans);
	VOID OnClientPreSending(PCONTEXT_OBJECT ContextObject, PBYTE szBuffer, ULONG ulOriginalLength);
	BOOL OnClientPostSending(PCONTEXT_OBJECT ContextObject, ULONG ulCompressedLength);
	VOID PostRecv(CONTEXT_OBJECT* ContextObject);
	//////////////////////////////////////////////////////////////////////////
	//监听线程
	static DWORD WINAPI ListenThreadProc(LPVOID lParam);
	static DWORD WINAPI WorkThreadProc(LPVOID lParam);
	//消息处理
	
	static CRITICAL_SECTION m_cs;
private:

	
	SOCKET m_sListenSocket;           //监听套接字
	HANDLE m_CompletionPortHandle;
	UINT   m_uMaxConnections;         //最大连接数
	HANDLE    m_ListenEventHandle;    //与监听套接字关联的事件
	HANDLE    m_ListenThreadHandle;   //监听线程句柄
	BOOL      m_bTimeToKill;
	HANDLE    m_KillEventHandle;


	ULONG m_ulThreadPoolMin;
	ULONG m_ulThreadPoolMax;
	ULONG m_ulCPULowThreadsHold;
	ULONG m_ulCPUHighThreadsHold;
	ULONG m_ulCurrentThread;
	ULONG m_ulBusyThread;
	ULONG m_ulWorkThreadCount;

	ULONG m_ulKeepLiveTime;
	  

	ContextOnjectList m_ContextFreePoolList;
	ContextOnjectList m_ContextConnectionList;

	pfnNotifyProc m_NotifyProc;      //回调函数指针

	CCpuUsage m_Cpu;

	char m_szPacketFlag[FLAG_LENGTH];
};


class CLock
{
public:
	CLock(CRITICAL_SECTION& cs)
	{
		m_cs = &cs;
		Lock();

	}
	~CLock()
	{
		UnLock();
	}
	VOID Lock()
	{
		EnterCriticalSection(m_cs);
	}
	VOID UnLock()
	{
		LeaveCriticalSection(m_cs);
	}

protected:
	CRITICAL_SECTION* m_cs;
};

class OVERLAPPEDPLUS
{
public:

	OVERLAPPED			m_ol;
	IOType				m_ioType;

	OVERLAPPEDPLUS(IOType ioType)
	{
		ZeroMemory(this, sizeof(OVERLAPPEDPLUS));
		m_ioType = ioType;
	}
};