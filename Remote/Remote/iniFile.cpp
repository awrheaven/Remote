#include "stdafx.h"
#include "iniFile.h"


iniFile::iniFile()
{
	ContructIniFile();
}


iniFile::~iniFile()
{
}


/************************************************************************/
/* ����BOOL iniFile::ContructIniFile()                                   */
/* ���ã���ʼ��ini�ļ�                                                   */
/************************************************************************/
BOOL iniFile::ContructIniFile()
{
	char szFilePath[MAX_PATH] = { 0 };
	char* FindPoint = NULL;
	//INT Len;
	//���ģ���ַ
	::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	//ȥ��.exe
	FindPoint = strrchr(szFilePath, '.');

	if (FindPoint != NULL)
	{
		*FindPoint = '\0';
		//����.ini
		strcat(szFilePath, ".ini");

	}
	m_IniFilePath = szFilePath;
	//���������ļ�
	HANDLE FileHandle = CreateFileA(m_IniFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//����ļ���С
	UINT32 ulLow = GetFileSize(FileHandle, NULL);


	if (ulLow>0)
	{
		CloseHandle(FileHandle);
		return FALSE;
	}
	CloseHandle(FileHandle);
	//дini�ļ�
	WritePrivateProfileString("Settings", "ListenPort", "2356", m_IniFilePath);
	WritePrivateProfileString("Settings", "MaxConnection", "10000", m_IniFilePath);

	return TRUE;
}

BOOL iniFile::GetInt(CString Mainkey, CString SubKey)
{
	return ::GetPrivateProfileInt(Mainkey, SubKey, 0, m_IniFilePath);
}

BOOL iniFile::SetInt(CString MainKey, CString SubKey, int Data)
{
	CString strData;
	strData.Format("%d", Data);   
	return ::WritePrivateProfileString(MainKey, SubKey,
		strData, m_IniFilePath);
}