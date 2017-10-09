#pragma once
#include "IOCPClient.h"

class IOCPClient;

class CManager
{
public:
	
	CManager(IOCPClient* ClientObject);
	virtual ~CManager();
	virtual VOID OnReceive(PBYTE szBuffer, ULONG ulLength)
	{
	}
	VOID WaitForDialogOpen();
	VOID NotifyDialogOpen();


public:
	HANDLE m_EventDlgOpenHandle;
	IOCPClient* m_ClientObject;
};

