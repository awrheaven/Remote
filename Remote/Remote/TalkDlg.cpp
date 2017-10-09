// TalkDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Remote.h"
#include "TalkDlg.h"
#include "afxdialogex.h"


// CTalkDlg 对话框
CONTEXT_OBJECT* g_ContextObject;
IMPLEMENT_DYNAMIC(CTalkDlg, CDialog)

CTalkDlg::CTalkDlg(CWnd* pParent /*=NULL*/, IOCPServer* IOCPServer , CONTEXT_OBJECT *ContextObject)
	: CDialog(IDD_DIALOG_TALK, pParent)
{
	m_IopcServer = IOCPServer;
	m_ContextObject = ContextObject;
	g_ContextObject = ContextObject;
}

CTalkDlg::~CTalkDlg()
{
	int a = 0;
}

void CTalkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TALK, m_EditTalk);
}


BEGIN_MESSAGE_MAP(CTalkDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_TALK, &CTalkDlg::OnBnClickedButtonTalk)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CTalkDlg 消息处理程序


BOOL CTalkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	BYTE bToken = COMMAND_NEXT;
	m_IopcServer->OnClientPreSending(m_ContextObject, &bToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CTalkDlg::OnBnClickedButtonTalk()
{
	// TODO: 在此添加控件通知处理程序代码
	int iLength = m_EditTalk.GetWindowTextLength();
	if (!iLength)
	{
		return;
	}
	CString strData;
	m_EditTalk.GetWindowText(strData);
	char* szBuffer = new char[iLength];
	memset(szBuffer, 0, sizeof(szBuffer));
	strcpy(szBuffer, strData.GetBuffer(0));
	m_EditTalk.SetWindowText(NULL);
	m_IopcServer->OnClientPreSending(m_ContextObject, (LPBYTE)szBuffer, strlen(szBuffer));
}


void CTalkDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->v1 = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
	CDialog::OnClose();
}


BOOL CTalkDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		// 屏蔽VK_ESCAPE、VK_DELETE
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
			
		//如果是可编辑框的回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_EditTalk.m_hWnd)
		{
			OnBnClickedButtonTalk();

			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
