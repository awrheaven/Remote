#pragma once
class iniFile
{
public:
	iniFile();
	~iniFile();
	BOOL ContructIniFile();      //��ʼ��ini�ļ�
	BOOL GetInt(CString Mainkey, CString SubKey);   //
	BOOL SetInt(CString MainKey, CString SubKey, int Data);

	CString m_IniFilePath;
};

