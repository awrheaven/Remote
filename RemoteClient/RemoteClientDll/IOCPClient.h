#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include "Buffer.h"
#include "Manager.h"

#pragma comment(lib,"ws2_32.lib")


#define FLAG_LENGTH    5
#define MAX_SEND_BUFFER  1024*8 
class IOCPClient
{
public:
	IOCPClient();
	~IOCPClient();
	VOID setManagerCallBack(class CManager* Manager);
	BOOL ConnectServer(char* szServerIp, unsigned short uPort);
	VOID DisConnect();
	VOID OnServerReceiving(char* szBuffer, ULONG ulLength);
	INT OnServerSending(char* szBuffer, ULONG ulOriginalLength);
	BOOL SendWithSplit(char* szBuffer, ULONG ulLength, ULONG ulSplitLength);
	inline BOOL IsRunning()
	{
		return m_bIsRunning;
	}
	VOID RunEventLoop();
	static DWORD WINAPI WorkThreadProc(LPVOID lParam);

public:

	SOCKET m_ClientSocket = INVALID_SOCKET;
	HANDLE m_WorkThreadHandle = NULL;
	HANDLE m_EventHandle = NULL;
	char m_szPacketFlag[FLAG_LENGTH] = { 0 };

	BOOL m_bIsRunning = TRUE;

	CBuffer m_CompressedBuffer;  //Ñ¹ËõÊý¾Ý
	CBuffer m_WriteBuffer;
	CBuffer m_DeCompressedBuffer;

public:
	class CManager* m_Manager;

};

