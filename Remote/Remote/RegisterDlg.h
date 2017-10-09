#pragma once
#include "IOCPServer.h"
#include "afxcmn.h"

// CRegisterDlg 对话框

class CRegisterDlg : public CDialog
{
	DECLARE_DYNAMIC(CRegisterDlg)

public:
	CRegisterDlg(CWnd* pParent = NULL, IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // 标准构造函数
	virtual ~CRegisterDlg();

	CString GetFullPath(HTREEITEM hCurrent);
	char GetFatherPath(CString& strFullPath);
	void OnReceiveComplete();
	VOID AddPath(char * szBuffer);
	VOID AddKey(char* szBuffer);
public:
	CONTEXT_OBJECT* m_ContextObject;
	IOCPServer* m_IOCPServer;


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REGISTER };
#endif

protected:
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	
	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_Tree;
	CListCtrl m_ControlList;
	CImageList m_ImageListTree;  //树控件上的图标	
	CImageList m_ImageListControlList;
	HICON m_hIcon;
	HTREEITEM	m_hRoot;
	HTREEITEM	HKLM;
	HTREEITEM	HKCR;
	HTREEITEM	HKCU;
	HTREEITEM	HKUS;
	HTREEITEM	HKCC;
	HTREEITEM   m_hSelectedItem;
	BOOL m_isEnable;
	virtual BOOL OnInitDialog();
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
};
