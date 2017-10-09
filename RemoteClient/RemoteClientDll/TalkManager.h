#pragma once
#include "Manager.h"
class CTalkManager :
	public CManager
{
public:
	CTalkManager(IOCPClient* ClientObject);
	virtual ~CTalkManager();
	VOID OnReceive(PBYTE szBuffer, ULONG ulLength);
	static VOID OnInitDialog(HWND hDlg);
	static VOID OnDlgTimer(HWND hDlg);
	static int CALLBACK DialogProc(HWND hDlg, unsigned int uMsg,
		WPARAM wParam, LPARAM lParam);
};

