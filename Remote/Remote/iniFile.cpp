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
/* 函数BOOL iniFile::ContructIniFile()                                   */
/* 作用：初始化ini文件                                                   */
/************************************************************************/
BOOL iniFile::ContructIniFile()
{
	char szFilePath[MAX_PATH] = { 0 };
	char* FindPoint = NULL;
	//INT Len;
	//获得模块地址
	::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	//去除.exe
	FindPoint = strrchr(szFilePath, '.');

	if (FindPoint != NULL)
	{
		*FindPoint = '\0';
		//连接.ini
		strcat(szFilePath, ".ini");

	}
	m_IniFilePath = szFilePath;
	//创建配置文件
	HANDLE FileHandle = CreateFileA(m_IniFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//获得文件大小
	UINT32 ulLow = GetFileSize(FileHandle, NULL);


	if (ulLow>0)
	{
		CloseHandle(FileHandle);
		return FALSE;
	}
	CloseHandle(FileHandle);
	//写ini文件
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