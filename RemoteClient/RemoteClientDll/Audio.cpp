#include "stdafx.h"
#include "Audio.h"


CAudio::CAudio()
{
	m_bIsWaveInUsed = FALSE;
	m_bIsWaveOutUsed = FALSE;
	m_dwWaveInIndex = 0;
	m_dwWaveOutIndex = 0;
	memset(&m_GSMWavefmt, 0, sizeof(GSM610WAVEFORMAT));
	m_GSMWavefmt.wfx.wFormatTag = WAVE_FORMAT_GSM610;
	m_GSMWavefmt.wfx.nChannels = 1;
	m_GSMWavefmt.wfx.nSamplesPerSec = 8000;
	m_GSMWavefmt.wfx.nAvgBytesPerSec = 1625;
	m_GSMWavefmt.wfx.nBlockAlign = 65;
	m_GSMWavefmt.wfx.wBitsPerSample = 0;
	m_GSMWavefmt.wfx.cbSize = 2;
	m_GSMWavefmt.wSamplesPerBlock = 320;
	m_ulBufferLength = 1000;
	int i = 0;
	for (i = 0; i < 2; i++)
	{
		m_InAudioData[i] = new BYTE[m_ulBufferLength];
		m_InAudioHeader[i] = new WAVEHDR;


		m_OutAudioData[i] = new BYTE[m_ulBufferLength];
		m_OutAudioHeader[i] = new WAVEHDR;
	}
}


CAudio::~CAudio()
{
	int i = 0;
	if (m_bIsWaveInUsed)
	{
		waveInStop(m_hWaveIn);
		waveInReset(m_hWaveIn);
		for (i = 0; i < 2; i++)
			waveInUnprepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));

		waveInClose(m_hWaveIn);

		TerminateThread(m_ThreadCallBack, -1);  //为了安全
	}

	for (i = 0; i < 2; i++)
	{
		delete[] m_InAudioData[i];
		m_InAudioData[i] = NULL;
		delete[] m_InAudioHeader[i];
		m_InAudioHeader[i] = NULL;
	}

	CloseHandle(m_EventWaveInHandle);
	CloseHandle(m_StartRecord);
	CloseHandle(m_ThreadCallBack);   //线程句柄






	if (m_bIsWaveOutUsed)
	{
		waveOutReset(m_hWaveOut);
		for (int i = 0; i < 2; i++)
			waveOutUnprepareHeader(m_hWaveOut, m_InAudioHeader[i], sizeof(WAVEHDR));
		waveOutClose(m_hWaveOut);
	}

	for (i = 0; i < 2; i++)
	{
		delete[] m_OutAudioData[i];
		m_OutAudioData[i] = NULL;
		delete[] m_OutAudioHeader[i];
		m_OutAudioHeader[i] = NULL;
	}

}

LPBYTE CAudio::GetRecordBuffer(LPDWORD dwBufferSize)
{
	//录音机
	if (m_bIsWaveInUsed == FALSE && InitializeWareIn() == FALSE)
	{
		return NULL;
	}
	if (dwBufferSize == NULL)
	{
		return NULL;
	}

	SetEvent(m_StartRecord);
	WaitForSingleObject(m_EventWaveInHandle, INFINITE);
	*dwBufferSize = m_ulBufferLength;
	return	m_InAudioData[m_dwWaveInIndex];              //返出真正数据
}

BOOL CAudio::InitializeWareIn()
{
	MMRESULT mmResult;
	DWORD dwThreadId = 0;

	m_ThreadCallBack = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)waveInCallBack, (LPVOID)this,
		CREATE_SUSPENDED, &dwThreadId);
	//打开录音设备COM  1 指定声音规格  2 支持通过线程回调 换缓冲
	mmResult = waveInOpen(&m_hWaveIn, (DWORD)WAVE_MAPPER,
		&(m_GSMWavefmt.wfx), (LONG)dwThreadId, (LONG)0, CALLBACK_THREAD);

	if (mmResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	//录音设备 需要的两个缓冲
	for (int i = 0; i < 2; i++)
	{
		m_InAudioHeader[i]->lpData = (LPSTR)m_InAudioData[i];   //m_lpInAudioData 指针数组
		m_InAudioHeader[i]->dwBufferLength = m_ulBufferLength;
		m_InAudioHeader[i]->dwFlags = 0;
		m_InAudioHeader[i]->dwLoops = 0;
		waveInPrepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));
	}

	waveInAddBuffer(m_hWaveIn, m_InAudioHeader[m_dwWaveInIndex], sizeof(WAVEHDR));

	ResumeThread(m_ThreadCallBack);
	waveInStart(m_hWaveIn);   //录音

	m_bIsWaveInUsed = TRUE;
}

DWORD WINAPI CAudio::waveInCallBack(LPVOID lParam)
{
	CAudio* This = (CAudio*)lParam;
	MSG Msg;
	while (GetMessage(&Msg,NULL,0,0))
	{
		if (Msg.message == MM_WIM_DATA)
		{
			SetEvent(This->m_EventWaveInHandle);
			WaitForSingleObject(This->m_StartRecord, INFINITE);

			Sleep(1);
			This->m_dwWaveInIndex = 1 - This->m_dwWaveInIndex;

			MMRESULT mmResult = waveInAddBuffer(This->m_hWaveIn,
				This->m_InAudioHeader[This->m_dwWaveInIndex], sizeof(WAVEHDR));
			if (mmResult != MMSYSERR_NOERROR)
			{
				return -1;
			}
				
		}
		if (Msg.message == MM_WIM_CLOSE)
		{

			//	cout<<"Close"<<endl;
			break;
		}
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return 0;
}

BOOL CAudio::PlayBuffer(LPBYTE szBuffer, DWORD dwBufferSize)
{

	if (!m_bIsWaveOutUsed && !InitializeWaveOut())  //1 音频格式   2 播音设备
	{
		return NULL;
	}
		

	for (int i = 0; i < dwBufferSize; i += m_ulBufferLength)
	{
		memcpy(m_OutAudioData[m_dwWaveOutIndex], szBuffer, m_ulBufferLength);
		waveOutWrite(m_hWaveOut, m_OutAudioHeader[m_dwWaveOutIndex], sizeof(WAVEHDR));
		m_dwWaveOutIndex = 1 - m_dwWaveOutIndex;
	}
	return true;
}

BOOL CAudio::InitializeWaveOut()
{

	if (!waveOutGetNumDevs())
	{
		return FALSE;
	}
		
	int i;
	for (i = 0; i < 2; i++)
	{
		memset(m_OutAudioData[i], 0, m_ulBufferLength);  //声音数据
	}
		

	MMRESULT	mmResult;
	mmResult = waveOutOpen(&m_hWaveOut, (WORD)WAVE_MAPPER, &(m_GSMWavefmt.wfx), (LONG)0, (LONG)0, CALLBACK_NULL);
	if (mmResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}
		

	for (i = 0; i < 2; i++)
	{
		m_OutAudioHeader[i]->lpData = (LPSTR)m_OutAudioData[i];
		m_OutAudioHeader[i]->dwBufferLength = m_ulBufferLength;
		m_OutAudioHeader[i]->dwFlags = 0;
		m_OutAudioHeader[i]->dwLoops = 0;
		waveOutPrepareHeader(m_hWaveOut, m_OutAudioHeader[i], sizeof(WAVEHDR));
	}

	m_bIsWaveOutUsed = TRUE;
	return TRUE;
}
