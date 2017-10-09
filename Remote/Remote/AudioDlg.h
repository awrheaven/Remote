#pragma once
#include "Audio.h"
#include "IOCPServer.h"

// CAudioDlg 对话框

class CAudioDlg : public CDialog
{
	DECLARE_DYNAMIC(CAudioDlg)

public:
	CAudioDlg(CWnd* pParent = NULL, IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // 标准构造函数
	virtual ~CAudioDlg();
	static DWORD WINAPI WorkThread(LPVOID lParam);
	VOID OnReceiveComplete();

public:
	CONTEXT_OBJECT* m_ContextObject;
	IOCPServer* m_IocpServer;
	HICON m_hIcon;
	CString m_strIpAddress;
	DWORD m_dwTotalRecvBytes;   //接收到的数据
	BOOL  m_bIsWorking;
	HANDLE m_WorkThreadHandle;   //工作线程是负责录音

	CAudio* m_AudioObject;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_AUDIO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bSend;
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
};
