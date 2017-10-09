#pragma once
#include "CursorInfor.h"

#define ALGORITHM_DIFF 1
class CScreenSpy
{
public:
	CScreenSpy(ULONG ulbiBitCount);
	~CScreenSpy();
	LPBITMAPINFO ConstructBI(ULONG ulBitCount, ULONG ulFullWidth, ULONG ulFullHeight);
	ULONG GetBISize();

	LPBITMAPINFO GetBIData();
	LPVOID GetFirstScreenData();
	ULONG GetFirstScreenLength();
	LPVOID GetNextScreenData(ULONG* ulNextSendLength);
	VOID WriteRectBuffer(LPBYTE szBuffer, ULONG ulLength);
	VOID ScanScreen(HDC hdcDest, HDC hdcSour, ULONG ulWidth, ULONG ulHeight);
	ULONG CompareBitmap(LPBYTE CompareSourData, LPBYTE CompareDestData,
		LPBYTE szBuffer, DWORD ulCompareLength);
public:
	ULONG m_ulbiBitCount;
	LPBITMAPINFO m_BitmapInfor_Full;
	ULONG  m_ulFullWidth, m_ulFullHeight;  //��Ļ�ֱ���
	HWND m_DeskTopWndHandle;      //��ǰ�������Ĵ��ھ��
	HDC  m_hFullDC;               //Explorer.exe �Ĵ����豸DC
	HDC  m_hFullMemDC;
	HBITMAP	m_BitmapHandle;
	PVOID   m_BitmapData_Full;
	DWORD  m_dwBitBltRop;
	BYTE* m_RectBuffer;
	ULONG m_RectBufferOffset;
	BYTE   m_bAlgorithm;
	CCursorInfor m_CursorInfor;
	HDC  m_hDiffMemDC;
	HBITMAP	m_DiffBitmapHandle;
	PVOID   m_DiffBitmapData_Full;
};

