#pragma once

#if !defined(AFX_AUDIO_H__56854DE7_5FE4_486F_9AFC_CE3726EF7CBC__INCLUDED_)
#define AFX_AUDIO_H__56854DE7_5FE4_486F_9AFC_CE3726EF7CBC__INCLUDED_
#endif 
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MMSYSTEM.H>
#include <MMReg.h>
#pragma comment(lib, "Winmm.lib")
class CAudio
{
public:
	CAudio();
	~CAudio();
	BOOL PlayBuffer(LPBYTE szBuffer, DWORD dwBufferSize);
	BOOL InitializeWaveOut();
	LPBYTE GetRecordBuffer(LPDWORD dwBufferSize);
	BOOL InitializeWaveIn();
	static DWORD WINAPI waveInCallBack(LPVOID lParam);

public:
	GSM610WAVEFORMAT m_GSMWavefmt;
	ULONG m_ulBufferLength;
	LPWAVEHDR m_InAudioHeader[2];   //����ͷ
	LPBYTE    m_InAudioData[2];     //��������   ��������������
	HANDLE	  m_hEventWaveIn;
	HANDLE	  m_hStartRecord;    //�����¼�
	HWAVEIN   m_hWaveIn;         //�豸���	
	DWORD     m_nWaveInIndex;
	HANDLE    m_hThreadCallBack;
	BOOL m_bIsWaveInUsed;
	HWAVEOUT m_hWaveOut;
	BOOL     m_bIsWaveOutUsed;
	DWORD    m_nWaveOutIndex;
	LPWAVEHDR m_OutAudioHeader[2];   //����ͷ
	LPBYTE    m_OutAudioData[2];
};

