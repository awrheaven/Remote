#pragma once
class iniFile
{
public:
	iniFile();
	~iniFile();
	BOOL ContructIniFile();      //初始化ini文件
	BOOL GetInt(CString Mainkey, CString SubKey);   //
	BOOL SetInt(CString MainKey, CString SubKey, int Data);

	CString m_IniFilePath;
};

