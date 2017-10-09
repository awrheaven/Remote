// TalkDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "TalkDlg.h"
#include "afxdialogex.h"


// CTalkDlg �Ի���
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


// CTalkDlg ��Ϣ�������


BOOL CTalkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	BYTE bToken = COMMAND_NEXT;
	m_IopcServer->OnClientPreSending(m_ContextObject, &bToken, sizeof(BYTE));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CTalkDlg::OnBnClickedButtonTalk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_ContextObject->v1 = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
	CDialog::OnClose();
}


BOOL CTalkDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		// ����VK_ESCAPE��VK_DELETE
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
			
		//����ǿɱ༭��Ļس���
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_EditTalk.m_hWnd)
		{
			OnBnClickedButtonTalk();

			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
