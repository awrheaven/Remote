#pragma once
#include "Manager.h"
#include "Common.h"

class CKernelManager :
	public CManager
{
public:
	
	CKernelManager(IOCPClient* ClientObject);
	virtual ~CKernelManager();

	VOID OnReceive(PBYTE szBuffer, ULONG ilLength);


public:
	HANDLE m_ThreadHandle[0x1000];
	ULONG  m_ulThreadCount;
};


