#pragma once


#if !defined(AFX_AUDIO_H__56854DE7_5FE4_486F_9AFC_CE3726EF7CBC__INCLUDED_)
#define AFX_AUDIO_H__56854DE7_5FE4_486F_9AFC_CE3726EF7CBC__INCLUDED_
#endif 
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MMSYSTEM.H>
#include <MMReg.h>
class CAudio
{
public:
	CAudio();
	~CAudio();
	LPBYTE GetRecordBuffer(LPDWORD dwBufferSize);
	BOOL InitializeWareIn();
	static DWORD WINAPI waveInCallBack(LPVOID lParam);

	BOOL PlayBuffer(LPBYTE szBuffer, DWORD dwBufferSize);
	BOOL InitializeWaveOut();

public:
	ULONG m_ulBufferLength;
	LPWAVEHDR m_InAudioHeader[2];
	GSM610WAVEFORMAT m_GSMWavefmt;
	LPBYTE m_InAudioData[2];
	HANDLE m_EventWaveInHandle;
	HANDLE m_StartRecord;
	HWAVEIN m_hWaveIn;
	BOOL m_bIsWaveInUsed;
	DWORD m_dwWaveInIndex;



	HWAVEOUT m_hWaveOut;	
	BOOL m_bIsWaveOutUsed;
	DWORD m_dwWaveOutIndex;
	LPWAVEHDR m_OutAudioHeader[2];   //两个头
	LPBYTE    m_OutAudioData[2];     //两个数据   保持声音的连续


	HANDLE m_ThreadCallBack;   //回调函数句柄



};

