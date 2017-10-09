#include "stdafx.h"
#include "Audio.h"


CAudio::CAudio()
{
	m_bIsWaveInUsed = FALSE;
	m_bIsWaveOutUsed = FALSE;
	m_nWaveInIndex = 0;
	m_nWaveOutIndex = 0;
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
	if (m_bIsWaveInUsed)
	{
		waveInStop(m_hWaveIn);
		waveInReset(m_hWaveIn);
		for (int i = 0; i < 2; i++)
			waveInUnprepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));
		waveInClose(m_hWaveIn);
		TerminateThread(m_hThreadCallBack, -1);  //Ϊ�˰�ȫ
	}
	for (int i = 0; i < 2; i++)
	{
		delete[] m_InAudioData[i];
		m_InAudioData[i] = NULL;
		delete[] m_InAudioHeader[i];
		m_InAudioHeader[i] = NULL;
	}

	CloseHandle(m_hEventWaveIn);
	CloseHandle(m_hStartRecord);
	CloseHandle(m_hThreadCallBack);   //�߳̾��

	if (m_bIsWaveOutUsed)
	{
		waveOutReset(m_hWaveOut);
		for (int i = 0; i < 2; i++)
			waveOutUnprepareHeader(m_hWaveOut, m_InAudioHeader[i], sizeof(WAVEHDR));
		waveOutClose(m_hWaveOut);
	}

	for (int i = 0; i < 2; i++)
	{
		delete[] m_OutAudioData[i];
		m_OutAudioData[i] = NULL;
		delete[] m_OutAudioHeader[i];
		m_OutAudioHeader[i] = NULL;
	}

}

BOOL CAudio::PlayBuffer(LPBYTE szBuffer, DWORD dwBufferSize)
{
	if (!m_bIsWaveOutUsed && !InitializeWaveOut())
	{
		return FALSE;
	}
	for (int i = 0; i < dwBufferSize; i += m_ulBufferLength)
	{
		memcpy(m_OutAudioData[m_nWaveOutIndex], szBuffer, m_ulBufferLength);
		waveOutWrite(m_hWaveOut, m_OutAudioHeader[m_nWaveOutIndex], sizeof(WAVEHDR));
		m_nWaveOutIndex = 1 - m_nWaveOutIndex;
	}
	return TRUE;
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
		memset(m_OutAudioData[i], 0, m_ulBufferLength);  //��������
	}
		

	MMRESULT	mmResult;
	mmResult = waveOutOpen(&m_hWaveOut, (WORD)WAVE_MAPPER, &(m_GSMWavefmt.wfx), (LONG)0, (LONG)0, CALLBACK_NULL);
	if (mmResult != MMSYSERR_NOERROR)
	{
		return false;
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

LPBYTE CAudio::GetRecordBuffer(LPDWORD dwBufferSize)
{
	if (m_bIsWaveInUsed == FALSE && InitializeWaveIn() == FALSE)
	{
		return NULL;
	}
	if (dwBufferSize == 0)
	{
		return NULL;
	}
	SetEvent(m_hStartRecord);
	WaitForSingleObject(m_hEventWaveIn, INFINITE);
	*dwBufferSize = m_ulBufferLength;
	return m_InAudioData[m_nWaveInIndex];
}

BOOL CAudio::InitializeWaveIn()
{
	MMRESULT	mmResult;
	DWORD		dwThreadID = 0;

	m_hThreadCallBack = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)waveInCallBack, (LPVOID)this,
		CREATE_SUSPENDED, &dwThreadID);



	//��¼���豸COM  1 ָ���������  2 ֧��ͨ���̻߳ص� ������
	mmResult = waveInOpen(&m_hWaveIn, (WORD)WAVE_MAPPER,
		&(m_GSMWavefmt.wfx), (LONG)dwThreadID, (LONG)0, CALLBACK_THREAD);


	//m_hWaveIn ¼�������
	if (mmResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	//¼���豸 ��Ҫ����������
	for (int i = 0; i < 2; i++)
	{
		m_InAudioHeader[i]->lpData = (LPSTR)m_InAudioData[i];   //m_lpInAudioData ָ������
		m_InAudioHeader[i]->dwBufferLength = m_ulBufferLength;
		m_InAudioHeader[i]->dwFlags = 0;
		m_InAudioHeader[i]->dwLoops = 0;
		waveInPrepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));
	}
	waveInAddBuffer(m_hWaveIn, m_InAudioHeader[m_nWaveInIndex], sizeof(WAVEHDR));
	ResumeThread(m_hThreadCallBack);
	waveInStart(m_hWaveIn);   //¼��
	m_bIsWaveInUsed = TRUE;
	return TRUE;
}

DWORD WINAPI CAudio::waveInCallBack(LPVOID lParam)
{
	CAudio	*This = (CAudio *)lParam;
	MSG	Msg;
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		if (Msg.message == MM_WIM_DATA)
		{
			SetEvent(This->m_hEventWaveIn);
			WaitForSingleObject(This->m_hStartRecord, INFINITE);
			Sleep(1);
			This->m_nWaveInIndex = 1 - This->m_nWaveInIndex;
			//���»��� 
			MMRESULT mmResult = waveInAddBuffer(This->m_hWaveIn,
				This->m_InAudioHeader[This->m_nWaveInIndex], sizeof(WAVEHDR));
			if (mmResult != MMSYSERR_NOERROR)
				return -1;

		}
		if (Msg.message == MM_WIM_CLOSE)
		{			
			break;
		}
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return 0;
}
