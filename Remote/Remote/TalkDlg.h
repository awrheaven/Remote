#pragma once

#include "IOCPServer.h"
#include "Common.h"
#include "afxwin.h"
// CTalkDlg �Ի���

class CTalkDlg : public CDialog
{
	DECLARE_DYNAMIC(CTalkDlg)

public:
	CTalkDlg(CWnd* pParent = NULL, IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	 ~CTalkDlg();


public:
	CONTEXT_OBJECT* m_ContextObject;
	IOCPServer* m_IopcServer;

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_TALK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonTalk();
	CEdit m_EditTalk;
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
