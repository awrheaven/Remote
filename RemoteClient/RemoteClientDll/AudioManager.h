#pragma once
#include "Manager.h"
#include "Audio.h"
class CAudioManager :
	public CManager
{
public:
	CAudioManager(IOCPClient* ClientObject);
	~CAudioManager();
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);
	BOOL Initialize();
	static DWORD WINAPI WorkThread(LPVOID lParam);
	int SendRecordBuffer();

public:
	BOOL m_isWorking;
	HANDLE m_WorkThreadHandle;
	CAudio* m_AudioObject;

};

