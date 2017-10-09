#pragma once

#include "IOCPServer.h"
#include "afxcmn.h"
#include "afxwin.h"
// CServicesDlg �Ի���

class CServicesDlg : public CDialog
{
	DECLARE_DYNAMIC(CServicesDlg)

public:
	CServicesDlg(CWnd* pParent = NULL, IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	virtual ~CServicesDlg();
	int ShowServicesList();
	VOID ServicesConfig(BYTE bCmd);
	VOID OnReceiveComplete();

public:
	CONTEXT_OBJECT* m_ContextObject;
	IOCPServer* m_IocpServer;

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SERVICES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_ControlList;
	CStatic m_ServicesCount;
	afx_msg void OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnServicesAuto();
	afx_msg void OnServicesManual();
	afx_msg void OnServicesStop();
	afx_msg void OnServicesStart();
	afx_msg void OnServicesReflash();
	afx_msg void OnClose();
};
