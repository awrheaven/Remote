#pragma once


#if !defined(AFX_CAPTUREVIDEO_H__0984BB8E_6DCB_4A5C_8E03_1217AE6E409D__INCLUDED_)
#define AFX_CAPTUREVIDEO_H__0984BB8E_6DCB_4A5C_8E03_1217AE6E409D__INCLUDED_
#endif
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Objbase.h>
#include <uuids.h>
#include <strmif.h>
#include <CONTROL.H>
#include <ATLBASE.H>



#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__

#include <qedit.h>
#include <amvideo.h>
#include <DShow.h>

#pragma comment(lib,"Strmiids.lib")


enum {
	CMD_CAN_COPY,
	CMD_CAN_SEND
};

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x ) \
	if ( NULL != x ) \
{ \
	x->Release( ); \
	x = NULL; \
}
#endif
class CSampleGrabberCB :
	public ISampleGrabberCB
{
public:
	CSampleGrabberCB();
	~CSampleGrabberCB();

	LPBITMAPINFO GetBmpInfor()	    //  
	{
		if (m_BitmapInfor_Full == NULL)  //ͷ��Ϣ
		{
			ConstructBI(24);
		}

		return m_BitmapInfor_Full;
	}


	LPBITMAPINFO ConstructBI(ULONG ulbiBitCount)
	{


		int	ColorNum = ulbiBitCount <= 8 ? 1 << ulbiBitCount : 0;
		ULONG ulBitmapLength = sizeof(BITMAPINFOHEADER) + (ColorNum * sizeof(RGBQUAD));   //BITMAPINFOHEADER +����ɫ��ĸ���

		m_BitmapInfor_Full = (BITMAPINFO *) new BYTE[ulBitmapLength];

		BITMAPINFOHEADER* BitmapInforHeader = &(m_BitmapInfor_Full->bmiHeader);

		BitmapInforHeader->biSize = sizeof(BITMAPINFOHEADER);//pi si 
		BitmapInforHeader->biWidth = m_ulFullWidth;
		BitmapInforHeader->biHeight = m_ulFullHeight;
		BitmapInforHeader->biPlanes = 1;
		BitmapInforHeader->biBitCount = ulbiBitCount;
		BitmapInforHeader->biCompression = BI_RGB;
		BitmapInforHeader->biXPelsPerMeter = 0;
		BitmapInforHeader->biYPelsPerMeter = 0;
		BitmapInforHeader->biClrUsed = 0;
		BitmapInforHeader->biClrImportant = 0;

		BitmapInforHeader->biSizeImage =                   //ͼ������
			(((BitmapInforHeader->biWidth * BitmapInforHeader->biBitCount + 31) & ~31) >> 3)
			* BitmapInforHeader->biHeight;
		// 16λ���Ժ��û����ɫ��ֱ�ӷ���


		//!!  
		m_dwSize = BitmapInforHeader->biSizeImage;    //���ݴ�С
		m_BitmapData_Full = new BYTE[m_dwSize + 10];
		ZeroMemory(m_BitmapData_Full, m_dwSize + 10);


		return m_BitmapInfor_Full;
	}

	STDMETHODIMP_(ULONG) AddRef() { return 2; }
	STDMETHODIMP_(ULONG) Release() { return 1; }


	STDMETHODIMP QueryInterface(REFIID riid, void ** lParam)
	{
		//???
		if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
			*lParam = (void *) static_cast<ISampleGrabberCB*> (this);
			return NOERROR;
		}

		return E_NOINTERFACE;
	}

	BYTE* GetNextScreen(DWORD &dwSize)
	{
		dwSize = m_dwSize;
		return (BYTE*)m_BitmapData_Full;
	}

	STDMETHODIMP SampleCB(double dbSampleTime, IMediaSample * Sample)
	{
		return 0;
	}

	//�ص����� ������õ� bmp ������
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE * szBuffer, long ulBufferSize)
	{

		if (!szBuffer)
		{
			return E_POINTER;
		}


		if (bStact == CMD_CAN_COPY)         //δ��ʼ�� ���͵�ͬ�����һ��
		{

			//��ͼ�����ݿ��������ǵ��ڴ�
			memcpy(m_BitmapData_Full, szBuffer, ulBufferSize);    //λͼ   

			InterlockedExchange((LPLONG)&bStact, CMD_CAN_SEND);      //ԭ���������Է���  
		}
	}


public:
	ULONG  m_ulFullWidth;
	ULONG  m_ulFullHeight;
	LPBITMAPINFO m_BitmapInfor_Full;
	BYTE*        m_BitmapData_Full;
	BOOL         bStact;
	DWORD        m_dwSize;
};

class CCaptureVideo
{
public:
	CCaptureVideo();
	~CCaptureVideo();
	HRESULT Open(int iDeviceID, int iPress);
	HRESULT InitCaptureGraphBuilder();
	BOOL BindVideoFilter(int deviceId, IBaseFilter **pFilter);
	void FreeMediaType(AM_MEDIA_TYPE& mt);
	HRESULT SetupVideoWindow();
	void ResizeVideoWindow();
	void SendEnd();
	LPBYTE GetDIB(DWORD & dwSize);
	LPBITMAPINFO GetBmpInfor();
public:
	HWND m_hWnd;
	IGraphBuilder* m_pGB;//ͨ����ֵ���Է��� FCDO   Filter Control Device Object
	ICaptureGraphBuilder2* m_pCapture;//ͨ����ֵ���Է��� ��ʵCDO
	IMediaControl*    m_pMC;             //�����豸�Ľӿ�
	IVideoWindow*     m_pVW;
	IBaseFilter*       m_pBF;              //FDO    
	ISampleGrabber*    m_pGrabber;		   //���� 24Color 
};