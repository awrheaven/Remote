#pragma once

#include <Windows.h>
class CBuffer
{
public:
	CBuffer();
	~CBuffer();
	VOID ClearBuffer();
	ULONG DeAllocateBuffer(ULONG ulLength);
	ULONG ReAllocateBuffer(ULONG ulLength);
	ULONG GetBufferLength();
	ULONG GetBufferMaxlength();
	ULONG RemoveComletedBuffer(ULONG ulLength);
	PBYTE GetBuffer(ULONG ulPos = 0);
	BOOL WriteBuffer(PBYTE Buffer, ULONG ulLength);
	ULONG ReadBuffer(PBYTE Buffer, ULONG ulLength);
private:
	PBYTE m_Base;
	PBYTE m_Ptr;
	ULONG m_ulMaxLength;
	CRITICAL_SECTION m_cs;

};

