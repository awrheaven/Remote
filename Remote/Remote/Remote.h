
// Remote.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "iniFile.h"

// CRemoteApp: 
// �йش����ʵ�֣������ Remote.cpp
//

class CRemoteApp : public CWinApp
{
public:
	CRemoteApp();
	iniFile m_IniFile;
// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CRemoteApp theApp;