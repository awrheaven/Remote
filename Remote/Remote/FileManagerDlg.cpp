// FileManagerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "FileManagerDlg.h"
#include "afxdialogex.h"
#include "EditDialog.h"
#include"Common.h"
#include "FileTransferModeDlg.h"
#include "FileCompress.h"
#include < Strsafe.h >

// CFileManagerDlg �Ի���

IMPLEMENT_DYNAMIC(CFileManagerDlg, CDialog)

CFileManagerDlg::CFileManagerDlg(CWnd* pParent /*=NULL*/, IOCPServer* IOCPServer, CONTEXT_OBJECT *ContextObject)
	: CDialog(IDD_DIALOG_FILE_MANAGER, pParent)
{
	m_ContextObject = ContextObject;
	m_IocpServer = IOCPServer;

	sockaddr_in ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));

	int iClientLength = sizeof(sockaddr_in);
	BOOL bOk = getpeername(ContextObject->ClientSocket, (SOCKADDR*)&ClientAddr, &iClientLength);


	m_strClientIp = bOk != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "";
	memset(m_szClientDiskDriverList, 0, sizeof(m_szClientDiskDriverList));
	memcpy(m_szClientDiskDriverList, ContextObject->InDeCompressedBuffer.GetBuffer(1), ContextObject->InDeCompressedBuffer.GetBufferLength() - 1);

	SHFILEINFO sfi;
	HIMAGELIST hImageList;
	hImageList = (HIMAGELIST)SHGetFileInfo(NULL,
		0, &sfi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_SYSICONINDEX);
	m_ImageList_Large = CImageList::FromHandle(hImageList);
	// ����ϵͳͼ���б�
	hImageList = (HIMAGELIST)SHGetFileInfo
	(
		NULL,
		0,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SMALLICON | SHGFI_SYSICONINDEX
	);
	m_ImageList_Small = CImageList::FromHandle(hImageList);


	m_bDragging = FALSE;
	m_bIsStop = FALSE;

}

CFileManagerDlg::~CFileManagerDlg()
{
}

VOID CFileManagerDlg::FixedServerDiskDriverList()
{
	char* Travel = NULL;
	m_ControlList_Server.DeleteAllItems();
	while (m_ControlList_Server.DeleteColumn(0) != 0);

	m_ControlList_Server.InsertColumn(0, "����", LVCFMT_LEFT, 70);
	m_ControlList_Server.InsertColumn(1, "����", LVCFMT_RIGHT, 85);
	m_ControlList_Server.InsertColumn(2, "�ܴ�С", LVCFMT_RIGHT, 80);
	m_ControlList_Server.InsertColumn(3, "���ÿռ�", LVCFMT_RIGHT, 90);
	m_ControlList_Server.SetExtendedStyle(LVS_EX_FULLROWSELECT);


	GetLogicalDriveStrings(sizeof(m_szServerDiskDriverList), (LPSTR)m_szServerDiskDriverList);
	Travel = m_szServerDiskDriverList;

	char szFileSystem[MAX_PATH] = { 0 };
	unsigned __int64	ulHardDiskAmount = 0;   //HardDisk
	unsigned __int64	ulHardDiskFreeSpace = 0;
	unsigned long		ulHardDiskAmountMB = 0; // �ܴ�С
	unsigned long		ulHardDiskFreeMB = 0;   // ʣ��ռ�
	for (int i = 0; *Travel != '\0'; i++, Travel += lstrlen(Travel) + 1)
	{
		// �õ����������Ϣ
		memset(szFileSystem, 0, sizeof(szFileSystem));
		// �õ��ļ�ϵͳ��Ϣ����С
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, szFileSystem, MAX_PATH);

		ULONG	ulFileSystemLength = lstrlen(szFileSystem) + 1;
		if (GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&ulHardDiskFreeSpace, (PULARGE_INTEGER)&ulHardDiskAmount, NULL))
		{
			ulHardDiskAmountMB = ulHardDiskAmount / 1024 / 1024;
			ulHardDiskFreeMB = ulHardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			ulHardDiskAmountMB = 0;
			ulHardDiskFreeMB = 0;
		}


		int	iItem = m_ControlList_Server.InsertItem(i, Travel, GetServerIconIndex(Travel, GetFileAttributes(Travel)));    //���ϵͳ��ͼ��


		m_ControlList_Server.SetItemData(iItem, 1);


		SHFILEINFO	sfi;
		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		m_ControlList_Server.SetItemText(iItem, 1, sfi.szTypeName);


		CString	strCount;
		strCount.Format("%10.1f GB", (float)ulHardDiskAmountMB / 1024);
		m_ControlList_Server.SetItemText(iItem, 2, strCount);
		strCount.Format("%10.1f GB", (float)ulHardDiskFreeMB / 1024);
		m_ControlList_Server.SetItemText(iItem, 3, strCount);
	}
}

VOID CFileManagerDlg::FixedClientDiskDriverList()
{
	m_ControlList_Client.DeleteAllItems();

	m_ControlList_Client.InsertColumn(0, "����", LVCFMT_LEFT, 70);
	m_ControlList_Client.InsertColumn(1, "����", LVCFMT_RIGHT, 85);
	m_ControlList_Client.InsertColumn(2, "�ܴ�С", LVCFMT_RIGHT, 80);
	m_ControlList_Client.InsertColumn(3, "���ÿռ�", LVCFMT_RIGHT, 90);

	m_ControlList_Client.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	char* Travel = NULL;
	Travel = (char*)m_szClientDiskDriverList;
	/*0x04628964  43 03 f2 b4 01 00 78 68 00 00 b1 be b5 d8 b4 c5 c5 cc 00 4e 54 46 53 00 44 03 04 bc  C.??..xh..????????.NTFS.D..?
	0x04628980  00 00 37 4b 00 00 b1 be b5 d8 b4 c5 c5 cc 00 4e 54 46 53 00 47 03 0d 78 03 00 a7 14  ..7K..????????.NTFS.G..x..?.
	0x0462899C  02 00 b1 be b5 d8 b4 c5 c5 cc 00 4e 54 46 53 00 48 03 0a 74 03 00 86 5d 03 00 b1 be  ..????????.NTFS.H..t..?]..??
	0x046289B8  b5 d8 b4 c5 c5 cc 00 4e 54 46 53 00 49 03 0a 74 03 00 14 87 02 00 b1 be b5 d8 b4 c5  ??????.NTFS.I..t...?..??????
	0x046289D4  c5 cc 00 4e 54 46 53 00 4a 03 e3 71 03 00 57 65 03 00 b1 be b5 d8 b4 c5 c5 cc 00 4e  ??.NTFS.J.?q..We..????????.N
	0x046289F0  54 46 53 00 4b 03 02 28 00 00 e0 27 00 00 b1 be b5 d8 b4 c5 c5 cc 00 4e 54 46 53 00  TFS.K..(..?'..????????.NTFS.
	0x04628A0C  4c 05 00 00 00 00 00 00 00 00 43 44 20 c7 fd b6 af c6 f7 00 00 00 00 00 00 00 00 00  L.........CD ??????*/
	int i = 0;
	ULONG ulIconIndex = 0;
	for (i = 0;Travel[i] != '\0';)
	{
		if (Travel[i] == 'A' || Travel[i] == 'B')
		{
			ulIconIndex = 6;
		}
		else
		{
			switch (Travel[i+1])
			{
				case DRIVE_REMOVABLE:
				{
					ulIconIndex = 2 + 5;
					break;
				}
					
				case DRIVE_FIXED:
				{
					ulIconIndex = 3 + 5;
					break;
				}
					
				case DRIVE_REMOTE:
				{
					ulIconIndex = 4 + 5;
					break;
				}
					
				case DRIVE_CDROM:
				{
					ulIconIndex = 9;	//Win7Ϊ10
					break;
				}
					
				default:
				{
					ulIconIndex = 0;
					break;
				}
					
			}
		}

		CString strVolume;
		strVolume.Format("%c:\\", Travel[i]);
		int iItem = m_ControlList_Client.InsertItem(i, strVolume, ulIconIndex);
		m_ControlList_Client.SetItemData(iItem, 1);

		unsigned long		ulHardDiskAmountMB = 0; // �ܴ�С
		unsigned long		ulHardDiskFreeMB = 0;   // ʣ��ռ�
		memcpy(&ulHardDiskAmountMB, Travel + i + 2, 4);
		memcpy(&ulHardDiskFreeMB, Travel + i + 6, 4);
		CString  strCount;
		strCount.Format("%10.1f GB", (float)ulHardDiskAmountMB / 1024);
		m_ControlList_Client.SetItemText(iItem, 2, strCount);
		strCount.Format("%10.1f GB", (float)ulHardDiskFreeMB / 1024);
		m_ControlList_Client.SetItemText(iItem, 3, strCount);

		i += 10;   //10 


		CString  strTypeName;
		strTypeName = Travel + i;
		m_ControlList_Client.SetItemText(iItem, 1, strTypeName);



		i += strlen(Travel + i) + 1;
		i += strlen(Travel + i) + 1;
	}
}

int CFileManagerDlg::GetServerIconIndex(LPCTSTR szVolume, DWORD dwFileAttributes)
{
	SHFILEINFO	sfi;
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
	{
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	}
		
	else
	{
		dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
	}
		

	SHGetFileInfo
	(
		szVolume,
		dwFileAttributes,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
	);

	return sfi.iIcon;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
VOID CFileManagerDlg::FixedServerFileList(CString strDirectory)
{
	if (strDirectory.GetLength() == 0)
	{
		int iItem = m_ControlList_Server.GetSelectionMark();

		if (iItem != -1)
		{
			//�����ǲ��Ǵ���
			if (m_ControlList_Server.GetItemData(iItem) == 1)
			{
				//����̷�
				strDirectory = m_ControlList_Server.GetItemText(iItem, 0);
			}
		}
		else
		{
			m_ComboBox_Server.GetWindowText(m_Server_File_Path);
		}
	}
	//
	if (strDirectory == "..")
	{
		m_Server_File_Path = GetParentDirectory(m_Server_File_Path);
	}
	else if (strDirectory != ".")
	{
		m_Server_File_Path += strDirectory;
		if (m_Server_File_Path.Right(1) != "\\")
		{
			m_Server_File_Path += "\\";
		}
	}

	if (m_Server_File_Path.GetLength() == 0)
	{
		FixedServerFileList();
		return;
	}

	m_ComboBox_Server.InsertString(0, m_Server_File_Path);
	m_ComboBox_Server.SetCurSel(0);
	//ɾ��������
	m_ControlList_Server.DeleteAllItems();
	while (m_ControlList_Server.DeleteColumn(0) != 0);  //ɾ��
	m_ControlList_Server.InsertColumn(0, "����", LVCFMT_LEFT, 200);
	m_ControlList_Server.InsertColumn(1, "��С", LVCFMT_LEFT, 100);
	m_ControlList_Server.InsertColumn(2, "����", LVCFMT_LEFT, 100);
	m_ControlList_Server.InsertColumn(3, "�޸�����", LVCFMT_LEFT, 115);

	int  iItemIndex = 0;
	//����0������
	m_ControlList_Server.SetItemData(m_ControlList_Server.InsertItem(iItemIndex++, "..",
		GetServerIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);

	for (int i = 0;i < 2;i++)
	{
		CFileFind FindFile;
		BOOL bContinue;
		bContinue = FindFile.FindFile(m_Server_File_Path + "*.*");
		while (bContinue)
		{
			bContinue = FindFile.FindNextFileA();
			if (FindFile.IsDots())
			{
				continue;
			}
			BOOL bIsInsert = !FindFile.IsDirectory() == i;
			if (!bIsInsert)
			{
				continue;
			}

			int iItem = m_ControlList_Server.InsertItem(iItemIndex++, FindFile.GetFileName(),
				GetServerIconIndex(FindFile.GetFileName(), GetFileAttributes(FindFile.GetFilePath())));

			m_ControlList_Server.SetItemData(iItem, FindFile.IsDirectory());

			SHFILEINFO	sfi;
			SHGetFileInfo(FindFile.GetFileName(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO),
				SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);

			if (FindFile.IsDirectory())
			{
				m_ControlList_Server.SetItemText(iItem, 2, "�ļ���");
			}

			else
			{
				m_ControlList_Server.SetItemText(iItem, 2, sfi.szTypeName);
			}

			CString strFileLength;
			strFileLength.Format("%10d KB", FindFile.GetLength() / 1024 + (FindFile.GetLength() % 1024 ? 1 : 0));
			m_ControlList_Server.SetItemText(iItem, 1, strFileLength);
			CTime Time;
			FindFile.GetLastWriteTime(Time);
			m_ControlList_Server.SetItemText(iItem, 3, Time.Format("%Y-%m-%d %H:%M"));
		}
	}
}
//��ÿͻ��˵�ָ�����ļ��б�
VOID CFileManagerDlg::GetClientFileList(CString strDirectory)
{
	if (strDirectory.GetLength() == 0)
	{
		int iItem = m_ControlList_Client.GetSelectionMark();

		if (iItem != -1)
		{
			if (m_ControlList_Client.GetItemData(iItem) == 1)
			{
				strDirectory = m_ControlList_Client.GetItemText(iItem, 0);
			}
		}
		else
		{

		}

		if (strDirectory == "..")
		{
		}
		else if(strDirectory != ".")
		{

			m_Client_File_Path += strDirectory;
			if (m_Client_File_Path.Right(1) != "\\")
			{
				m_Client_File_Path += "\\";
			}
		}
	}

	if (m_Client_File_Path .GetLength() == 0)
	{
		return;
	}

	ULONG ulLength = m_Client_File_Path.GetLength() + 2;
	BYTE* szBuffer = (BYTE*)new BYTE[ulLength];
	szBuffer[0] = COMMAND_LIST_FILES;

	memcpy(szBuffer + 1, m_Client_File_Path.GetBuffer(0), ulLength - 1);

	m_IocpServer->OnClientPreSending(m_ContextObject, szBuffer, ulLength);

	delete[] szBuffer;
	szBuffer = NULL;
	m_ControlList_Client.EnableWindow(FALSE);
	//���ý������ĳ�ʼλ��
	m_ProgressCtrl->SetPos(0);
}

CString CFileManagerDlg::GetParentDirectory(CString strPath)
{
	CString strCurrentPath = strPath;
	//�������һ��ƥ����
	int iIndex = strCurrentPath.ReverseFind('\\');
	if (iIndex == -1)
	{
		return strCurrentPath;
	}
	CString strCurrentSubPath = strCurrentPath.Left(iIndex);
	iIndex = strCurrentPath.ReverseFind('\\');
	if (iIndex == -1)
	{
		strCurrentPath = "";
		return strCurrentPath;
	}
	strCurrentPath = strCurrentSubPath.Left(iIndex);
	if (strCurrentPath.Right(1) != "\\")
	{
		strCurrentPath += "\\";
	}
	return strCurrentPath;
}

BOOL CFileManagerDlg::MakeSureDirectoryPathExists(char * szDirtoryFullPath)
{
	char* szTravel = NULL;
	char* szBuffer = NULL;

	DWORD dwAttributes = 0;
	__try
	{
		//
		szBuffer = (char*)malloc(sizeof(char)*(strlen(szDirtoryFullPath) + 1));
		if (szBuffer == NULL)
		{
			return FALSE;
		}
		strcpy(szBuffer, szDirtoryFullPath);
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
				//��ǰһ��Ŀ¼�õ�
				*szTravel = '\0';
				dwAttributes = GetFileAttributes(szBuffer);
				if (dwAttributes == 0xffffffff)
				{
					if (!CreateDirectory(szBuffer,NULL))
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
			//��ָ������ƶ�
			szTravel = CharNext(szTravel);
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
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

VOID CFileManagerDlg::EnableControl(BOOL bEnable)
{
	m_ControlList_Client.EnableWindow(bEnable);
	m_ControlList_Server.EnableWindow(bEnable);
	m_ComboBox_Server.EnableWindow(bEnable);
	m_ComboBox_Client.EnableWindow(bEnable);
}

BOOL CFileManagerDlg::DeleteDirectory(LPCTSTR strDirectoryFullPath)
{
	WIN32_FIND_DATA wfd;
	char szBuffer[MAX_PATH] = { 0 };
	wsprintf(szBuffer, "%s\\*.*", strDirectoryFullPath);

	HANDLE FileHandle = FindFirstFile(szBuffer, &wfd);

	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	do 
	{
		if (wfd.cFileName[0] != '.')
		{
			//���ļ���
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char szV1[MAX_PATH] = { 0 };
				wsprintf(szV1, "%s\\%s", strDirectoryFullPath, wfd.cFileName);
				DeleteDirectory(szV1);

			}
			//�ļ�
			else
			{
				char szv2[MAX_PATH];
				wsprintf(szv2, "%s\\%s", strDirectoryFullPath, wfd.cFileName);
				DeleteFile(szv2);
			}
		}
	} while (FindNextFileA(FileHandle, &wfd));

	FindClose(FileHandle);
	if (!RemoveDirectory(strDirectoryFullPath))
	{
		return FALSE;
	}
	return TRUE;
}

VOID CFileManagerDlg::OnReceiveComplete()
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	
	switch (m_ContextObject->InDeCompressedBuffer.GetBuffer()[0])
	{
		case TOKEN_FILE_LIST:
		{
			FixedClientFileList(m_ContextObject->InDeCompressedBuffer.GetBuffer(),
				m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1);
			break;
		}
		case TOKEN_GET_TRANSFER_MODE:
		{
			SendTransferMode();
			break;
		}
		case TOKEN_DATA_CONTINUE:
		{
			SendFileData();
			break;
		}
		default:
			break;
	}
}

VOID CFileManagerDlg::FixedClientFileList(BYTE * szBuffer, ULONG ulLength)
{
	m_ControlList_Client.DeleteAllItems();
	while (m_ControlList_Client.DeleteColumn(0) != 0);
	m_ControlList_Client.InsertColumn(0, "����", LVCFMT_LEFT, 200);
	m_ControlList_Client.InsertColumn(1, "��С", LVCFMT_LEFT, 100);
	m_ControlList_Client.InsertColumn(2, "����", LVCFMT_LEFT, 100);
	m_ControlList_Client.InsertColumn(3, "�޸�����", LVCFMT_LEFT, 115);

	int	iItemIndex = 0;
	m_ControlList_Client.SetItemData(m_ControlList_Client.InsertItem(iItemIndex++, 
		"..", GetServerIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);


	if (ulLength != 0)
	{
		for (int i = 0;i < 2;i++)
		{
			char* szTravel = (char*)(szBuffer + 1);
			for (char* szBase = szTravel; szTravel - szBase < ulLength - 1;)
			{
				char* szFileName = NULL;
				DWORD dwFileSizeHigh = 0;
				DWORD dwFileSizeLow = 0;
				int iItem = 0;
				BOOL bIsInsert = FALSE;
				FILETIME FileTime;

				int iType = *szTravel ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

				// i Ϊ 0 ʱ����Ŀ¼��iΪ1ʱ���ļ�
				bIsInsert = !(iType == FILE_ATTRIBUTE_DIRECTORY) == i;

				szFileName = ++szTravel;

				if (bIsInsert)
				{
					iItem = m_ControlList_Client.InsertItem(iItemIndex++, szFileName, GetServerIconIndex(szFileName, iType));
					m_ControlList_Client.SetItemData(iItem, iType == FILE_ATTRIBUTE_DIRECTORY);   //��������
					SHFILEINFO	sfi;
					SHGetFileInfo(szFileName, FILE_ATTRIBUTE_NORMAL | iType, &sfi, sizeof(SHFILEINFO),
						SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					m_ControlList_Client.SetItemText(iItem, 2, sfi.szTypeName);
				}

				szTravel += strlen(szFileName) + 1;
				if (bIsInsert)
				{
					memcpy(&dwFileSizeHigh, szTravel, 4);
					memcpy(&dwFileSizeLow, szTravel + 4, 4);
					CString strFileSize;
					strFileSize.Format("%10d KB", (dwFileSizeHigh * (MAXDWORD + 1)) / 1024 + dwFileSizeLow / 1024 + (dwFileSizeLow % 1024 ? 1 : 0));
					m_ControlList_Client.SetItemText(iItem, 1, strFileSize);
					memcpy(&FileTime, szTravel + 8, sizeof(FILETIME));
					CTime	Time(FileTime);
					m_ControlList_Client.SetItemText(iItem, 3, Time.Format("%Y-%m-%d %H:%M"));
				}
				szTravel += 16;
			}
		}
	}
	m_ControlList_Client.EnableWindow(TRUE);
}

VOID CFileManagerDlg::DropItemOnList()
{
	if (m_DragControlList == m_DropControlList)
	{
		return;
	}
	if ((CWnd *)m_DropControlList == &m_ControlList_Client)
	{
		OnCopyServerToClient();
		
	}
	else
	{
		return;
	}
}

VOID CFileManagerDlg::OnCopyServerToClient()
{
	m_Remote_Upload_Jop.RemoveAll();

	POSITION Pos = m_ControlList_Server.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = m_ControlList_Server.GetNextSelectedItem(Pos);
		CString strFileFullPath = "";

		strFileFullPath = m_Server_File_Path + 
			m_ControlList_Server.GetItemText(iItem, 0);
	
		// �����Ŀ¼
		if (m_ControlList_Server.GetItemData(iItem))
		{
			strFileFullPath += '\\';
			//����Ŀ¼
			FixedServerToClientDirectory(strFileFullPath.GetBuffer(0));
		}
		else
		{
			HANDLE FileHandle = CreateFile(strFileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (FileHandle == INVALID_HANDLE_VALUE)
			{
				continue;
			}
			m_Remote_Upload_Jop.AddTail(strFileFullPath);
			CloseHandle(FileHandle);
		}

	}
	if (m_Remote_Upload_Jop.IsEmpty())
	{
		::MessageBox(m_hWnd, "�ļ���Ϊ��", "����", MB_OK | MB_ICONWARNING);
		return;
	}
	EnableControl(FALSE);
	SendToClientJop();
}

BOOL CFileManagerDlg::FixedServerToClientDirectory(LPCTSTR szDircetoryFullPath)
{
	char szBuffer[MAX_PATH] = { 0 };
	char* szSlash = NULL;
	if (szDircetoryFullPath[strlen(szDircetoryFullPath) - 1] != '\\')
		szSlash = "\\";
	else
		szSlash = "";
	sprintf(szBuffer, "%s%s*.*", szDircetoryFullPath, szSlash);

	WIN32_FIND_DATA wfd;
	HANDLE FileHandle = FindFirstFile(szBuffer, &wfd);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	do 
	{
		if (wfd.cFileName[0] == '.')
		{
			continue;
		}
		//��Ŀ¼
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			char szV1[MAX_PATH] = { 0 };
			sprintf(szV1, "%s%s%s", szDircetoryFullPath, szSlash, wfd.cFileName);
			FixedServerToClientDirectory(szV1); // ����ҵ�����Ŀ¼��������Ŀ¼���еݹ� 
		}
		else
		{
			CString strFileFullPath;
			strFileFullPath.Format("%s%s%s", szDircetoryFullPath, szSlash, wfd.cFileName);
			m_Remote_Upload_Jop.AddTail(strFileFullPath);
		}

	} while (FindNextFileA(FileHandle,&wfd));

	FindClose(FileHandle);
	return TRUE;
}

BOOL CFileManagerDlg::SendToClientJop()
{
	if (m_Remote_Upload_Jop.IsEmpty())
	{
		return FALSE;
	}

	CString strDestDirectory = m_Client_File_Path;
	//��õ�һ�����������

	m_strSourFileFullPath = m_Remote_Upload_Jop.GetHead();

	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = 0;


	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	// /Զ���ļ�
	CString strString = m_strSourFileFullPath;
	//����Ŀ��Ŀ¼
	strString.Replace(m_Server_File_Path, m_Client_File_Path);
	m_strDestFileFullPath = strString;

	FileHandle = CreateFile(m_strSourFileFullPath.GetBuffer(0),
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);   //���Ҫ�����ļ��Ĵ�С)
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	dwSizeLow = GetFileSize(FileHandle, &dwSizeHigh);

	m_OperatingFileLength = (dwSizeHigh * (MAXDWORD + 1)) + dwSizeLow;
	CloseHandle(FileHandle);


	ULONG ulLength = strString.GetLength() + 10;


	BYTE* szBuffer = (BYTE*)LocalAlloc(LPTR, ulLength);
	memset(szBuffer, 0, ulLength);
	szBuffer[0] = COMMAND_FILE_SIZE;

	memcpy(szBuffer + 1, &dwSizeHigh, sizeof(DWORD));
	memcpy(szBuffer + 5, &dwSizeLow, sizeof(DWORD));

	memcpy(szBuffer + 9, strString.GetBuffer(0), strString.GetLength() + 1);

	m_IocpServer->OnClientPreSending(m_ContextObject, szBuffer, ulLength);
	LocalFree(szBuffer);
	m_Remote_Upload_Jop.RemoveHead();
	return TRUE;
}

VOID CFileManagerDlg::SendTransferMode()
{
	CFileTransferModeDlg Dlg(this);
	Dlg.m_strFileName = m_strDestFileFullPath;
	switch (Dlg.DoModal())
	{
	case IDC_OVERWRITE:
		m_ulTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case IDC_OVERWRITE_ALL:
		m_ulTransferMode = TRANSFER_MODE_OVERWRITE_ALL;
		break;
	case IDC_JUMP:
		m_ulTransferMode = TRANSFER_MODE_JUMP;
		break;
	case IDC_JUMP_ALL:
		m_ulTransferMode = TRANSFER_MODE_JUMP_ALL;
		break;
	case IDCANCEL:
		m_ulTransferMode = TRANSFER_MODE_CANCEL;
		break;
	}
	if (m_ulTransferMode == TRANSFER_MODE_CANCEL)
	{
		EndCopyServerToClient();
		return;
	}
	BYTE bToken[5] = { 0 };

	bToken[0] = COMMAND_SET_TRANSFER_MODE;
	memcpy(bToken + 1, &m_ulTransferMode, sizeof(m_ulTransferMode));
	m_IocpServer->OnClientPreSending(m_ContextObject, (BYTE*)&bToken, sizeof(bToken));

}

VOID CFileManagerDlg::EndCopyServerToClient()
{
	m_ulCounter = 0;
	m_OperatingFileLength = 0;

	ShowProgress();
	if (m_Remote_Upload_Jop.IsEmpty() || m_bIsStop)
	{
		m_Remote_Upload_Jop.RemoveAll();
		m_bIsStop = FALSE;
		EnableControl(TRUE);
		m_ulTransferMode = TRANSFER_MODE_NORMAL;
		GetClientFileList(".");
	}
	else
	{
		Sleep(5);
		SendToClientJop();
	}
}

VOID CFileManagerDlg::ShowProgress()
{
	if ((int)m_ulCounter == -1)
	{
		m_ulCounter = m_OperatingFileLength;

	}
	int iProgress = (float)(m_ulCounter * 100) / m_OperatingFileLength;

	m_ProgressCtrl->SetPos(iProgress);
	if (m_ulCounter == m_OperatingFileLength)
	{
		m_ulCounter = m_OperatingFileLength;
	}
}

VOID CFileManagerDlg::SendFileData()
{
	FILE_SIZE* FileSize = (FILE_SIZE*)(m_ContextObject->InDeCompressedBuffer.GetBuffer(1));

	LONG dwOffsetHigh = FileSize->dwSizeHigh;
	LONG dwOffsetLow = FileSize->dwSizeLow;
	m_ulCounter = MAKEINT64(FileSize->dwSizeLow, FileSize->dwSizeHigh);   //0

	ShowProgress();

	//����ļ������Ѿ�����m_ulCounter ��ֹͣ
	if (m_ulCounter == m_OperatingFileLength || m_bIsStop)
	{
		EndCopyServerToClient();
		return;
	}

	//���ļ����
	HANDLE FileHandle = CreateFile(m_strSourFileFullPath.GetBuffer(0),
		GENERIC_READ, FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 0);

	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	SetFilePointer(FileHandle, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int iHeadLength = 0;
	DWORD dwNumberOfBytesToRead = MAX_SEND_BUFFER - iHeadLength;

	DWORD dwNumberOfBytesRead = 0;
	BYTE* szBuffer = (BYTE*)LocalAlloc(LPTR, MAX_SEND_BUFFER);

	if (szBuffer == NULL)
	{
		CloseHandle(FileHandle);
		return;
	}
	szBuffer[0] = COMMAND_FILE_DATA;

	memcpy(szBuffer + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(szBuffer + 5, &dwOffsetLow, sizeof(dwOffsetLow));
	ReadFile(FileHandle, szBuffer + iHeadLength, dwNumberOfBytesToRead, &dwNumberOfBytesRead,
		NULL);
	CloseHandle(FileHandle);
	if (dwNumberOfBytesRead > 0 )
	{
		ULONG ulLength = dwNumberOfBytesRead + iHeadLength;
		m_IocpServer->OnClientPreSending(m_ContextObject, szBuffer, ulLength);
	}
	LocalFree(szBuffer);
}
//ѹ���ļ�
VOID CFileManagerDlg::ServerCompress(ULONG ulType)
{
	//��ȡѡ���ĵ�һ��
	POSITION Pos = m_ControlList_Server.GetFirstSelectedItemPosition();
	CString strString;
	while (Pos)
	{
		int iItem = m_ControlList_Server.GetNextSelectedItem(Pos);

		strString += m_Server_File_Path + m_ControlList_Server.GetItemText(iItem, 0);
		strString += " ";
	}

	if (!strString.IsEmpty())
	{
		CString strRARFileFullPath;
		strRARFileFullPath += m_Server_File_Path;
		CFileCompress Dlg(this, ulType);

		if (Dlg.DoModal() == IDOK)
		{
			if (Dlg.m_EditRarName.IsEmpty())
			{
				MessageBox("ERROR");
				return;

			}

			strRARFileFullPath += Dlg.m_EditRarName;
			strRARFileFullPath += ".rar";
			CompressFiles(strRARFileFullPath.GetBuffer(strRARFileFullPath.GetLength()),
				strString.GetBuffer(strString.GetLength()), ulType);
		}
	}
}
//ѹ���ļ�
//Դ�ļ���strString
BOOL CFileManagerDlg::CompressFiles(PCSTR strRarFileFullPath, PSTR strSourceFileFullPath, ULONG ulType)
{
	PSTR szExePath = "/c c:\\progra~1\\winrar\\winrar.exe a -ad -ep1 -ibck ";
	ULONG ulLength = strlen(szExePath) + strlen(strRarFileFullPath) + strlen(strSourceFileFullPath) + 2;

	PSTR szBuffer = (PSTR)malloc(sizeof(char)*ulLength);
	//����
	StringCchCopyN(szBuffer, ulLength, szExePath, strlen(szExePath));
	StringCchCatN(szBuffer, ulLength, strRarFileFullPath, strlen(strRarFileFullPath));
	StringCchCatN(szBuffer, ulLength, " ", 1);
	StringCchCatN(szBuffer, ulLength, strSourceFileFullPath, strlen(strSourceFileFullPath));

	if (ulType == 1)
	{
		SHELLEXECUTEINFO sei = { 0 };
		sei.cbSize = sizeof sei;
		sei.lpVerb = "open";
		sei.lpFile = "c:\\windows\\system32\\cmd.exe";
		sei.lpParameters = szBuffer;
		sei.nShow = SW_HIDE;
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		BOOL fReturn = ShellExecuteEx(&sei);

		CloseHandle(sei.hProcess);
		return (fReturn);
	}
	return TRUE;
}

void CFileManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FILE_SERVER, m_FileServerBarPos);
	DDX_Control(pDX, IDC_STATIC_FILE_CLIENT, m_FileClientBarPos);
	DDX_Control(pDX, IDC_LIST_SERVER, m_ControlList_Server);
	DDX_Control(pDX, IDC_LIST_CLIENT, m_ControlList_Client);
	DDX_Control(pDX, IDC_COMBO_SERVER, m_ComboBox_Server);
	DDX_Control(pDX, IDC_COMBO_CLIENT, m_ComboBox_Client);


}


BEGIN_MESSAGE_MAP(CFileManagerDlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_SERVER, &CFileManagerDlg::OnNMDblclkListServer)
	ON_CBN_SELCHANGE(IDC_COMBO_SERVER, &CFileManagerDlg::OnSelchangeComboServer)
	ON_COMMAND(IDT_SERVER_FILE_PREV, &CFileManagerDlg::OnIdtServerFilePrev)
	ON_COMMAND(IDT_SERVER_FILE_NEW_FOLDER, &CFileManagerDlg::OnIdtServerFileNewFolder)
	ON_COMMAND(IDT_SERVER_FILE_DELETE, &CFileManagerDlg::OnIdtServerFileDelete)
	ON_COMMAND(IDT_SERVER_FILE_STOP, &CFileManagerDlg::OnIdtServerFileStop)
	ON_COMMAND(ID_VIEW_BIG_ICON, &CFileManagerDlg::OnViewBigIcon)
	ON_COMMAND(ID_VIEW_SMALL_ICON, &CFileManagerDlg::OnViewSmallIcon)
	ON_COMMAND(ID_VIEW_DETAIL, &CFileManagerDlg::OnViewDetail)
	ON_COMMAND(ID_VIEW_LIST, &CFileManagerDlg::OnViewList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CLIENT, &CFileManagerDlg::OnNMDblclkListClient)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_SERVER, &CFileManagerDlg::OnLvnBegindragListServer)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SERVER, &CFileManagerDlg::OnNMRClickListServer)
	ON_COMMAND(ID_OPERATION_SERVER_RUN, &CFileManagerDlg::OnOperationServerRun)
	ON_COMMAND(ID_OPERATION_RENAME, &CFileManagerDlg::OnOperationRename)
	ON_COMMAND(ID_OPERATION_CLIENT_SHOW_RUN, &CFileManagerDlg::OnOperationClientShowRun)
	ON_COMMAND(ID_OPERATION_CLIENT_HIDE_RUN, &CFileManagerDlg::OnOperationClientHideRun)
	ON_COMMAND(ID_OPERATION_COMPRESS, &CFileManagerDlg::OnOperationCompress)
	ON_COMMAND(ID_OPERATION_DECOMPRESS, &CFileManagerDlg::OnOperationDecompress)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CLIENT, &CFileManagerDlg::OnNMRClickListClient)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_SERVER, &CFileManagerDlg::OnLvnEndlabeleditListServer)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_CLIENT, &CFileManagerDlg::OnLvnEndlabeleditListClient)
END_MESSAGE_MAP()


// CFileManagerDlg ��Ϣ�������


BOOL CFileManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	CString strString;
	strString.Format("\\\\%s Զ���ļ�����", m_strClientIp);
	SetWindowText(strString);

	if (!m_ToolBar_File_Server.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR_FILE)
		|| !m_ToolBar_File_Server.LoadToolBar(IDR_TOOLBAR_FILE))
	{

		return -1;
	}
	//m_ToolBar_File_Server.ModifyStyle(0, TBSTYLE_FLAT);
	m_ToolBar_File_Server.LoadTrueColorToolBar
	(
		24,    //������ʹ����� 
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE    //û����
	);

	m_ToolBar_File_Server.AddDropDownButton(this, IDT_SERVER_FILE_VIEW, IDR_MENU_FILE_SERVER);

	m_ToolBar_File_Server.SetButtonText(0, "����");     //��λͼ����������ļ�
	m_ToolBar_File_Server.SetButtonText(1, "�鿴");
	m_ToolBar_File_Server.SetButtonText(2, "ɾ��");
	m_ToolBar_File_Server.SetButtonText(3, "�½�");
	m_ToolBar_File_Server.SetButtonText(4, "����");
	m_ToolBar_File_Server.SetButtonText(5, "ֹͣ");

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //��ʾ

	RECT RectServer;
	m_FileServerBarPos.GetWindowRect(&RectServer);
	

	m_FileServerBarPos.ShowWindow(SW_HIDE);
	m_ToolBar_File_Server.MoveWindow(&RectServer);   //��ʾToolbar


	
	m_ControlList_Server.SetImageList(m_ImageList_Large, LVSIL_NORMAL);
	m_ControlList_Server.SetImageList(m_ImageList_Small, LVSIL_SMALL);


	m_ControlList_Client.SetImageList(m_ImageList_Large, LVSIL_NORMAL);
	m_ControlList_Client.SetImageList(m_ImageList_Small, LVSIL_SMALL);

	//����״̬��
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(Indicators1,
			sizeof(Indicators1) / sizeof(UINT)))
	{
		return -1;
	}

	m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_StatusBar.SetPaneInfo(1, m_StatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_StatusBar.SetPaneInfo(2, m_StatusBar.GetItemID(2), SBPS_NORMAL, 50);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //��ʾ״̬��



	RECT	ClientRect;
	GetClientRect(&ClientRect);
	CRect Rect;
	Rect.top = ClientRect.bottom - 25;
	Rect.left = 0;
	Rect.right = ClientRect.right;
	Rect.bottom = ClientRect.bottom;

	m_StatusBar.MoveWindow(Rect);

	m_StatusBar.GetItemRect(1, &ClientRect);

	ClientRect.bottom -= 1;

	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, ClientRect, &m_StatusBar, 1);
	m_ProgressCtrl->SetRange(0, 100);           //���ý�������Χ
	m_ProgressCtrl->SetPos(0);                  //���ý�������ǰλ��


	FixedServerDiskDriverList();
	FixedClientDiskDriverList();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


BOOL CFileManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���

	return CDialog::PreTranslateMessage(pMsg);
}


void CFileManagerDlg::OnNMDblclkListServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//int a = m_ControlList_Server.GetSelectedCount();   //ѡ�е�����
	//int b = m_ControlList_Server.GetSelectionMark();   //ѡ���Ǹ�
	if (m_ControlList_Server.GetSelectedCount() == 0 || 
		m_ControlList_Server.GetItemData(m_ControlList_Server.GetSelectionMark()) != 1)
	{
		return;
	}
	//�о��ļ�
	FixedServerFileList();
	*pResult = 0;
}


void CFileManagerDlg::OnSelchangeComboServer()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	int iIndex = m_ComboBox_Server.GetCurSel();
	CString strString;
	m_ComboBox_Server.GetLBText(iIndex, strString);

	m_ComboBox_Server.SetWindowTextA(strString);
	FixedServerFileList();

}


//����
void CFileManagerDlg::OnIdtServerFilePrev()
{
	// TODO: �ڴ���������������
	FixedServerFileList("..");
}

//�½�
void CFileManagerDlg::OnIdtServerFileNewFolder()
{
	// TODO: �ڴ���������������
	if (m_Server_File_Path == "")
	{
		return;
	}

	CEditDialog Dlg(this);
	if (Dlg.DoModal() == IDOK && Dlg.m_EditString.GetLength())
	{

		CString strString;
		strString = m_Server_File_Path + Dlg.m_EditString + "\\";

		MakeSureDirectoryPathExists(strString.GetBuffer());
		FixedServerFileList(".");
	}
}


void CFileManagerDlg::OnIdtServerFileDelete()
{
	// TODO: �ڴ���������������

	CString strString;
	if (m_ControlList_Server.GetSelectedCount()>1)
	{
		strString.Format("ȷ��Ҫ���� %d ��ɾ����?", 
			m_ControlList_Server.GetSelectedCount());
	}
	else
	{
		CString strFileName = m_ControlList_Server.GetItemText(m_ControlList_Server.GetSelectionMark(), 0);

		int iItem = m_ControlList_Server.GetSelectionMark();
		if (iItem == -1)
		{
			return;
		}

		if (m_ControlList_Server.GetItemData(iItem) == 1)
		{
			strString.Format("ȷʵҪɾ���ļ��С�%s��������������ɾ����?", 
				strFileName);
		}
		else
		{
			strString.Format("ȷʵҪ�ѡ�%s��ɾ����?", strFileName);
		}
		
	}
	if (::MessageBox(m_hWnd, strString, "ȷ��ɾ��",
		MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	EnableControl(FALSE);

	POSITION Pos = m_ControlList_Server.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = m_ControlList_Server.GetNextSelectedItem(Pos);
		CString strFileFullPath = m_Server_File_Path + m_ControlList_Server.GetItemText(iItem, 0);

		if (m_ControlList_Server.GetItemData(iItem))
		{
			strFileFullPath += '\\';
			DeleteDirectory(strFileFullPath);
		}
		else
		{
			DeleteFile(strFileFullPath);
		}

	}
	EnableControl(TRUE);

	FixedServerFileList(".");
}


void CFileManagerDlg::OnIdtServerFileStop()
{
	// TODO: �ڴ���������������
	m_bIsStop = TRUE;
}


void CFileManagerDlg::OnViewBigIcon()
{
	// TODO: �ڴ���������������
	m_ControlList_Server.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


void CFileManagerDlg::OnViewSmallIcon()
{
	// TODO: �ڴ���������������
	m_ControlList_Server.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);
}


void CFileManagerDlg::OnViewDetail()
{
	// TODO: �ڴ���������������
	m_ControlList_Server.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
}


void CFileManagerDlg::OnViewList()
{
	// TODO: �ڴ���������������
	m_ControlList_Server.ModifyStyle(LVS_TYPEMASK, LVS_LIST);
}


void CFileManagerDlg::OnNMDblclkListClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if (m_ControlList_Client.GetSelectedCount() == 0 ||
		m_ControlList_Client.GetItemData(m_ControlList_Client.GetSelectionMark()) != 1)
	{
		return;
	}

	GetClientFileList();
	*pResult = 0;
}


void CFileManagerDlg::OnLvnBegindragListServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_Server_File_Path.IsEmpty() || m_Client_File_Path.IsEmpty())
	{
		return;
	}
	if (m_ControlList_Server.GetSelectedCount() > 1)
	{
		m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR_MDRAG);
	}
	{
		m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG);
	}
	

	m_bDragging = TRUE;
	m_DragControlList = &m_ControlList_Server;
	m_DropControlList = &m_ControlList_Server;

	SetCapture();
	*pResult = 0;
}


void CFileManagerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	if (m_bDragging)
	{
		CPoint Point(point); //������λ��
		ClientToScreen(&Point); //������λ��
	//��������ô��ھ��
		CWnd* pDropWnd = WindowFromPoint(Point);    //ֵ����λ�� ��û�пؼ�

		if (pDropWnd->IsKindOf(RUNTIME_CLASS(CListCtrl)))   //�������ǵĴ��ڷ�Χ��
		{
			SetCursor(m_hCursor);

			return;
		}
		else
		{
			SetCursor(LoadCursor(NULL, IDC_NO));   //�������ڻ������ʽ
		}

	}
	CDialog::OnMouseMove(nFlags, point);
}


void CFileManagerDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	if (m_bDragging)
	{
		//�ͷ����Ĳ���
		ReleaseCapture();
		m_bDragging = FALSE;
		//��õ�ǰ����λ�������������Ļ��
		CPoint Point(point);
		//ת��������ڵ�ǰ�û��Ĵ��ڵ�λ��
		ClientToScreen(&Point);
		CWnd* DropWnd = WindowFromPoint(Point);   //��õ�ǰ������·����޿ؼ�

		if (DropWnd->IsKindOf(RUNTIME_CLASS(CListCtrl))) //�����һ��ListControl
		{
			m_DropControlList = (CListCtrl*)DropWnd;       //���浱ǰ�Ĵ��ھ��

			DropItemOnList(); //������
		}
	}

	CDialog::OnLButtonUp(nFlags, point);
}


void CFileManagerDlg::OnNMRClickListServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_FILE_OPERATION);
	CMenu* SubMenu = Menu.GetSubMenu(0);

	CPoint Point;
	GetCursorPos(&Point);
	//SubMenu->DeleteMenu(2, MF_BYPOSITION);
	
	if (m_ControlList_Server.GetSelectedCount() == 0)
	{
		int iCount = SubMenu->GetMenuItemCount();
		for (int i = 0; i < iCount;i++)
		{
			//������Menu
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
	}
	//��ָ����λ����ʾMneu
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);
	*pResult = 0;
}


void CFileManagerDlg::OnOperationServerRun()
{
	// TODO: �ڴ���������������
	CString strFileFullPath;
	//�ļ�·��
	strFileFullPath = m_Server_File_Path +
		m_ControlList_Server.GetItemText(m_ControlList_Server.GetSelectionMark(), 0);
	//ִ��
	ShellExecute(NULL, "open", strFileFullPath, NULL, NULL, SW_SHOW);
}


void CFileManagerDlg::OnOperationRename()
{
	// TODO: �ڴ���������������
	POINT Point;
	GetCursorPos(&Point);
	//������ǰ�������뽹���ָ�� CWnd��

	if (GetFocus()->m_hWnd == m_ControlList_Server.m_hWnd)
	{
		//�༭
		m_ControlList_Server.EditLabel(m_ControlList_Server.GetSelectionMark());
	}
	else
	{
		m_ControlList_Client.EditLabel(m_ControlList_Client.GetSelectionMark());
	}
}


void CFileManagerDlg::OnOperationClientShowRun()
{
	// TODO: �ڴ���������������
	CString strFileFullPath;
	//���ļ�·��
	strFileFullPath = m_Client_File_Path +
		m_ControlList_Client.GetItemText(m_ControlList_Client.GetSelectionMark(), 0);

	ULONG ulLenght = strFileFullPath.GetLength() + 2;

	BYTE szBuffer[MAX_PATH + 10] = { 0 };

	szBuffer[0] = COMMAND_OPEN_FILE_SHOW;
	memcpy(szBuffer + 1, strFileFullPath.GetBuffer(0), ulLenght - 1);
	m_IocpServer->OnClientPreSending(m_ContextObject, szBuffer, ulLenght);

	
}


void CFileManagerDlg::OnOperationClientHideRun()
{
	// TODO: �ڴ���������������
}


void CFileManagerDlg::OnOperationCompress()
{
	// TODO: �ڴ���������������

	POINT Point;
	GetCursorPos(&Point);
	if (GetFocus()->m_hWnd == m_ControlList_Server.m_hWnd)
	{
		ServerCompress(1);
	}
}


void CFileManagerDlg::OnOperationDecompress()
{
	// TODO: �ڴ���������������
}


void CFileManagerDlg::OnNMRClickListClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CMenu	Menu;
	Menu.LoadMenu(IDR_MENU_FILE_OPERATION);
	CMenu*	SubMenu = Menu.GetSubMenu(0);
	CPoint	Point;
	GetCursorPos(&Point);
	SubMenu->DeleteMenu(1, MF_BYPOSITION);
	if (m_ControlList_Client.GetSelectedCount() == 0)
	{
		int	iCount = SubMenu->GetMenuItemCount();
		for (int i = 0; i < iCount; i++)
		{
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
	}

	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);
	*pResult = 0;
}


void CFileManagerDlg::OnLvnEndlabeleditListServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strNewFileName;
	CString strExistingFileFullPath;
	CString strNewFileFullPath;

	m_ControlList_Server.GetEditControl()->GetWindowTextA(strNewFileName);
	strExistingFileFullPath = m_Server_File_Path + 
		m_ControlList_Server.GetItemText(pDispInfo->item.iItem, 0);

	strNewFileFullPath = m_Server_File_Path + strNewFileName;

	*pResult = ::MoveFile(strExistingFileFullPath.GetBuffer(0),
		strNewFileFullPath.GetBuffer(0));
}


void CFileManagerDlg::OnLvnEndlabeleditListClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strNewFileName, strExistingFileFullPath, strNewFileFullPath;
	m_ControlList_Client.GetEditControl()->GetWindowText(strNewFileName);

	strExistingFileFullPath = m_Client_File_Path + m_ControlList_Client.GetItemText(pDispInfo->item.iItem, 0);
	strNewFileFullPath = m_Client_File_Path + strNewFileName;

	if (strExistingFileFullPath != strNewFileFullPath)
	{
		UINT   ulLength = strExistingFileFullPath.GetLength() + strNewFileFullPath.GetLength() + 3;
		LPBYTE szBuffer = (LPBYTE)LocalAlloc(LPTR, ulLength);
		szBuffer[0] = COMMAND_RENAME_FILE;                                                                   //�򱻿ض˷�����Ϣ
		memcpy(szBuffer + 1, strExistingFileFullPath.GetBuffer(0), strExistingFileFullPath.GetLength() + 1);
		memcpy(szBuffer + 2 + strExistingFileFullPath.GetLength(),
			strNewFileFullPath.GetBuffer(0), strNewFileFullPath.GetLength() + 1);
		m_IocpServer->OnClientPreSending(m_ContextObject, szBuffer, ulLength);
		LocalFree(szBuffer);


		GetClientFileList(".");
	}
	*pResult = 0;
}
