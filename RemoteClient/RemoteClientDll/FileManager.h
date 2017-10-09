#pragma once
#include "Manager.h"
#include"Common.h"

typedef struct
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILE_SIZE;
class CFileManager :
	public CManager
{
public:
	CFileManager(IOCPClient* ClientObject);
	virtual ~CFileManager();
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);
	ULONG SendDiskDriverList();
	ULONG SendFileList(char* szDirectoryFullPath);
	VOID CreateClientRecvFile(LPBYTE szBuffer);
	BOOL MakeSureDirectoryPathExists(char* szDirectoryFullPath);
	VOID GetFileData();
	VOID WriteClientRecvFile(LPBYTE szBuffer, ULONG ulLength);
	VOID SetTransferMode(LPBYTE szBuffer);
	VOID Rename(char* szExisting, char* szNewFileFullPath);
public:
	char m_szOperatingFileName[MAX_PATH];
	INT64 m_OperatingFileLength;
	ULONG m_ulTransferMode;  //文件传输方法
};

