#include "stdafx.h"
#include "Manager.h"


CManager::CManager(IOCPClient* ClientObject)
{
	m_ClientObject = ClientObject;
	m_ClientObject->setManagerCallBack(this);
	m_EventDlgOpenHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
}


CManager::~CManager()
{
	if (m_EventDlgOpenHandle != NULL)
	{
		CloseHandle(m_EventDlgOpenHandle);
		m_EventDlgOpenHandle = NULL;
	}
}

VOID CManager::WaitForDialogOpen()
{
	WaitForSingleObject(m_EventDlgOpenHandle,INFINITE);
	Sleep(150);
}

VOID CManager::NotifyDialogOpen()
{
	SetEvent(m_EventDlgOpenHandle);
}
