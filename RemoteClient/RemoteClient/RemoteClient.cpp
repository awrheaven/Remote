// ClientEXE.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>



typedef void(*pFnTextRun)(char* szServerIp, unsigned short uPort);

VOID Start();
VOID Start_1();
BOOL CreateExeFilePre(char* szFileFullPath, INT iResourceID, char* szResourceName);
BOOL CreateExeFilePost(char * szFileFullPath, LPBYTE szBuffer, DWORD dwBufferSize);
int main()
{
	//Start();
	Start_1();
	return 0;
}
VOID Start()
{
	char szDllFullPath[MAX_PATH] = "ClientDll.dll";
	//CreateExeFilePre(szDllFullPath, IDR_DLL, "RES");
}

BOOL CreateExeFilePre(char* szFileFullPath, INT iResourceID, char* szResourceName)
{
	HRSRC hRescInfor;
	HGLOBAL hRescData;
	DWORD  dwRescSize;
	LPBYTE szBuffer;

	//查找所属的资源得资源句柄
	hRescInfor = FindResource(NULL, MAKEINTRESOURCE(iResourceID), szResourceName);
	if (hRescInfor == NULL)
	{
		return FALSE;
	}

	//获得资源大小
	dwRescSize = SizeofResource(NULL, hRescInfor);
	//装在资源
	hRescData = LoadResource(NULL, hRescInfor);

	if (hRescData == NULL)
	{
		return FALSE;
	}

	//申请内存
	szBuffer = (LPBYTE)GlobalAlloc(GPTR, dwRescSize);

	if (szBuffer == NULL)
	{
		return FALSE;

	}
	CopyMemory((LPVOID)szBuffer, (LPVOID)LockResource(hRescData), dwRescSize);

	BOOL bOk = CreateExeFilePost(szFileFullPath, szBuffer, dwRescSize);
	if (!bOk)
	{
		GlobalFree((HGLOBAL)szBuffer);
		return FALSE;
	}

	GlobalFree((HGLOBAL)szBuffer);
	return TRUE;
}

BOOL CreateExeFilePost(char* szFileFullPath, LPBYTE szBuffer, DWORD dwBufferSize)
{
	DWORD dwReturn = 0;

	HANDLE FileHandle = CreateFile(szFileFullPath, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, 0, NULL);
	if (FileHandle != NULL)
	{
		WriteFile(FileHandle, (LPVOID)szBuffer, dwBufferSize, &dwReturn, NULL);
	}
	else
	{
		return FALSE;
	}
	CloseHandle(FileHandle);
	return TRUE;
}


VOID Start_1()
{
	char szDllFullPath[MAX_PATH] = "RemoteClientDll.dll";

	HMODULE DLlHandle = LoadLibrary(szDllFullPath);

	char szServeIp[] = "127.0.0.1";

	unsigned short uPort = 2356;
	if (DLlHandle != NULL)
	{
		pFnTextRun TestRunAddress = (pFnTextRun)GetProcAddress(DLlHandle, "TestRun");

		if (TestRunAddress != NULL)
		{
			TestRunAddress(szServeIp, uPort);
		}
	}

	printf("Input AnyKey To Exit\r\n");

	getchar();
	getchar();

	if (DLlHandle != NULL)
	{
		FreeLibrary(DLlHandle);
		DLlHandle = NULL;
	}

}