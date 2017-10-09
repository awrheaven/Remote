#pragma once

#define U_PAGE_ALIGNMENT   3
#define F_PAGE_ALIGNMENT 3.0
class CBuffer
{
public:
	CBuffer();
	~CBuffer();
	BOOL WriteBuffer(PBYTE Buffer, ULONG ulLength);
	ULONG ReAllocateBuffer(ULONG ulLength);

	ULONG GetBufferLength();
	ULONG GetBufferMaxLength();
	VOID ClearBuffer();
	ULONG DeAllocateBuffer(ULONG ulLength);
	PBYTE GetBuffer(ULONG ulPos = 0);
	ULONG ReadBuffer(PBYTE Buffer, ULONG ulLength);
protected:
	PBYTE m_Base = NULL;
	PBYTE m_Ptr = NULL;
	ULONG m_ulMaxLength = 0;
	CRITICAL_SECTION m_cs;

};

