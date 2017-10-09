#pragma once
#include "Audio.h"
#include "IOCPServer.h"

// CAudioDlg �Ի���

class CAudioDlg : public CDialog
{
	DECLARE_DYNAMIC(CAudioDlg)

public:
	CAudioDlg(CWnd* pParent = NULL, IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	virtual ~CAudioDlg();
	static DWORD WINAPI WorkThread(LPVOID lParam);
	VOID OnReceiveComplete();

public:
	CONTEXT_OBJECT* m_ContextObject;
	IOCPServer* m_IocpServer;
	HICON m_hIcon;
	CString m_strIpAddress;
	DWORD m_dwTotalRecvBytes;   //���յ�������
	BOOL  m_bIsWorking;
	HANDLE m_WorkThreadHandle;   //�����߳��Ǹ���¼��

	CAudio* m_AudioObject;

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_AUDIO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bSend;
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
};
