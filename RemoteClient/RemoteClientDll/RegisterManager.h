#pragma once
#include "Manager.h"
#include"RegisterOperation.h"
class CRegisterManager :
	public CManager
{
public:
	CRegisterManager(IOCPClient* ClientObject);
	virtual ~CRegisterManager();
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);
	VOID Find(char bToken, char* szPath);
};

