#include "stdafx.h"
#include "Buffer.h"

#include <math.h>
CBuffer::CBuffer()
{
	InitializeCriticalSection(&m_cs);
}


CBuffer::~CBuffer()
{
}

BOOL CBuffer::WriteBuffer(PBYTE Buffer, ULONG ulLength)
{
	EnterCriticalSection(&m_cs);
	if (ReAllocateBuffer(ulLength + GetBufferLength()) == -1)
	{
		LeaveCriticalSection(&m_cs);
		return FALSE;
	}
	CopyMemory(m_Ptr, Buffer, ulLength);
	m_Ptr += ulLength;
	LeaveCriticalSection(&m_cs);
	return TRUE;
}

ULONG CBuffer::ReAllocateBuffer(ULONG ulLength)
{
	if (ulLength < GetBufferMaxLength())
	{
		return 0;
	}

	ULONG ulNewMaxLength = (ULONG)ceil(ulLength / F_PAGE_ALIGNMENT)*U_PAGE_ALIGNMENT;

	PBYTE NewBase = (PBYTE)VirtualAlloc(NULL, ulNewMaxLength, 
		MEM_COMMIT, PAGE_READWRITE);

	if (NewBase == NULL)
	{
		return -1;
	}

	ULONG ulV1 = GetBufferLength();
	CopyMemory(NewBase, m_Base, ulV1);
	if (m_Base)
	{
		VirtualFree(m_Base, 0, MEM_RELEASE);
	}
	m_Base = NewBase;
	m_Ptr = m_Base + ulV1;

	m_ulMaxLength = ulNewMaxLength;
	return m_ulMaxLength;

}

ULONG CBuffer::GetBufferLength()
{
	if (m_Base == NULL)
	{
		return 0;
	}
	return (ULONG)m_Ptr - (ULONG)m_Base;
}

ULONG CBuffer::GetBufferMaxLength()
{
	return m_ulMaxLength;
}

VOID CBuffer::ClearBuffer()
{
	EnterCriticalSection(&m_cs);
	m_Ptr = m_Base;
	DeAllocateBuffer(1024);
	LeaveCriticalSection(&m_cs);
}

ULONG CBuffer::DeAllocateBuffer(ULONG ulLength)
{
	if (ulLength < GetBufferLength())
	{
		return 0;
	}

	ULONG ulNewMaxLength = (ULONG)ceil(ulLength / F_PAGE_ALIGNMENT)*U_PAGE_ALIGNMENT;
	if (GetBufferMaxLength() <= ulNewMaxLength)
	{
		return 0;
	}

	PBYTE NewBase = (PBYTE)VirtualAlloc(NULL, ulNewMaxLength, MEM_COMMIT,PAGE_READWRITE);

	ULONG ulV1 = GetBufferLength();
	CopyMemory(NewBase, m_Base, ulV1);
	VirtualFree(m_Base, 0, MEM_RELEASE);
	m_Base = NewBase;
	m_Ptr = m_Base + ulV1;
	m_ulMaxLength = ulNewMaxLength;
	return m_ulMaxLength;

}

PBYTE CBuffer::GetBuffer(ULONG ulPos)
{
	if (m_Base == NULL)
	{
		return NULL;
	}
	if (ulPos >= GetBufferLength())
	{
		return NULL;
	}
	return m_Base + ulPos;
}

ULONG CBuffer::ReadBuffer(PBYTE Buffer, ULONG ulLength)
{
	EnterCriticalSection(&m_cs);
	if (ulLength > GetBufferMaxLength())
	{
		LeaveCriticalSection(&m_cs);
		return 0;
	}
	if (ulLength > GetBufferLength())
	{
		ulLength = GetBufferLength();
	}
	if (ulLength)
	{
		CopyMemory(Buffer, m_Base, ulLength);
		MoveMemory(m_Base, m_Base + ulLength, GetBufferMaxLength() - ulLength);
		m_Ptr -= ulLength;

	}
	DeAllocateBuffer(GetBufferLength());
	LeaveCriticalSection(&m_cs);
	return ulLength;

}
