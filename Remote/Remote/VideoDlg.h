#pragma once

#include "IOCPServer.h"
#include "CVideoCodec.h"
// CVideoDlg 对话框



class CVideoDlg : public CDialog
{
	DECLARE_DYNAMIC(CVideoDlg)

public:
	CVideoDlg(CWnd* pParent = NULL ,IOCPServer* IOCPServer = NULL, CONTEXT_OBJECT *ContextObject = NULL);   // 标准构造函数
	virtual ~CVideoDlg();
	VOID ResetScreen();

public:
	CONTEXT_OBJECT* m_ContextObject;
	IOCPServer* m_IocpServer;
	CString m_strIpAddress;
	LPBITMAPINFO m_BitmapInfor_Full;
	BYTE*        m_BitmapData_Full;
	BYTE*        m_BitmapCompressedData_Full;
	HDC			m_hDC;
	HDRAWDIB	m_hDD;
	CVideoCodec* m_pVideoCodec;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_VIDEO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void OnReceiveComplete();
	void DrawDIB();
	void InitCodec(DWORD fccHandler);
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
