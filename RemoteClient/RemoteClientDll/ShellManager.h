#pragma once
#include "Manager.h"
class CShellManager :
	public CManager
{
public:
	CShellManager(IOCPClient* ClientObject);
	virtual ~CShellManager();
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);
	static DWORD WINAPI ReadPipeThread(LPVOID lParam);

public:
	HANDLE m_ReadPipeHandle = NULL;
	HANDLE m_WritePipeHandle = NULL;
	HANDLE m_ReadPipeShellHandle = NULL;
	HANDLE m_WritePipeShellHandle = NULL;
	HANDLE m_ThreadHandle = NULL;
	HANDLE m_ShellProcessHandle = NULL;   //cmd主进程句柄
	HANDLE m_ShellThreadHandle = NULL;    //cmd主线程句柄
};

