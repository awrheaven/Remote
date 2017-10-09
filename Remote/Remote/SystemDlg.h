#pragma once
#include "IOCPServer.h"
#include "afxcmn.h"

// CSystemDlg 对话框

class CSystemDlg : public CDialog
{
	DECLARE_DYNAMIC(CSystemDlg)

public:
	CSystemDlg(CWnd* pParent = NULL,IOCPServer* IOCPServer = NULL,CONTEXT_OBJECT* ContextObject = NULL);   // 标准构造函数
	virtual ~CSystemDlg();
	VOID ShowProcessList();
	VOID ShowWindowsList();
	VOID GetProcessList();
	VOID GetWindowsList();
	VOID OnReceiveComplete();

public:
	CONTEXT_OBJECT* m_ContextOBject;
	IOCPServer* m_IOCPServer;
	BOOL m_bHow;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SYSTEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_ControlList;
	//afx_msg void OnItemchangedListSystem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListSystem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPlistKill();
	afx_msg void OnPlistRefresh();
	afx_msg void OnClose();
	afx_msg void OnWlistRefresh();
	afx_msg void OnWlistClose();
	afx_msg void OnWlistHide();
	afx_msg void OnWlistRecover();
	afx_msg void OnWlistMax();
	afx_msg void OnWlistMin();
};
