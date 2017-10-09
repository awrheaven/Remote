#include "stdafx.h"
#include "Buffer.h"

#define F_PAGE_ALIGNMENT 3.0
#define U_PAGE_ALIGNMENT   3
CBuffer::CBuffer()
{
	m_ulMaxLength = 0;
	m_Ptr = m_Base = NULL;
	InitializeCriticalSection(&m_cs);
}


CBuffer::~CBuffer()
{
	if (m_Base)
	{
		VirtualFree(m_Base, 0, MEM_RELEASE);
		m_Base = NULL;
	}

	DeleteCriticalSection(&m_cs);

	m_Base = m_Ptr = NULL;
	m_ulMaxLength = 0;
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
	ULONG ulNewMaxLength = (ULONG)ceil(ulLength / F_PAGE_ALIGNMENT)* U_PAGE_ALIGNMENT;;
	if (GetBufferMaxlength() <= ulNewMaxLength)
	{
		return 0;
	}

	PBYTE NewBase = (PBYTE)VirtualAlloc(NULL, ulNewMaxLength, MEM_COMMIT, PAGE_READWRITE);
	ULONG ulV1 = GetBufferLength();
	CopyMemory(NewBase, m_Base, ulV1);

	VirtualFree(m_Base, 0, MEM_RELEASE);
	m_Base = NewBase;
	m_Ptr = m_Base + ulV1;

	m_ulMaxLength = ulNewMaxLength;
	return m_ulMaxLength;


}

ULONG CBuffer::ReAllocateBuffer(ULONG ulLength)
{
	if (ulLength < GetBufferMaxlength())
	{
		return 0;
	}
	ULONG ulNewMaxLength = (ULONG)ceil(ulLength / F_PAGE_ALIGNMENT)*U_PAGE_ALIGNMENT;
	//重新申请内存
	PBYTE NewBase = (PBYTE)VirtualAlloc(NULL, ulNewMaxLength, MEM_COMMIT, PAGE_READWRITE);
	if (NewBase == NULL)
	{
		return -1;
	}

	ULONG ulv1 = GetBufferLength();
	CopyMemory(NewBase, m_Base, ulv1);
	if (m_Base)
	{
		//释放原来的数据
		VirtualFree(m_Base, 0, MEM_RELEASE);
	}
	m_Base = NewBase;
	m_Ptr = m_Base + ulv1;

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

ULONG CBuffer::GetBufferMaxlength()
{
	return m_ulMaxLength;
}

ULONG CBuffer::RemoveComletedBuffer(ULONG ulLength)
{
	if (ulLength > GetBufferMaxlength())
	{
		return 0;
	}
	if (ulLength > GetBufferLength())
	{
		ulLength = GetBufferLength();
	}
	if (ulLength)
	{
		MoveMemory(m_Base, m_Base + ulLength, GetBufferLength() - ulLength);
		m_Ptr -= ulLength;
	}
	DeAllocateBuffer(GetBufferLength());
	return ulLength;
}

BOOL CBuffer::WriteBuffer(PBYTE Buffer, ULONG ulLength)
{
	EnterCriticalSection(&m_cs);
	if (ReAllocateBuffer(ulLength+GetBufferLength()) == -1)
	{
		LeaveCriticalSection(&m_cs);
		return FALSE;
	}
	CopyMemory(m_Ptr, Buffer, ulLength);
	m_Ptr += ulLength;
	LeaveCriticalSection(&m_cs);
	return TRUE;
}

ULONG CBuffer::ReadBuffer(PBYTE Buffer, ULONG ulLength)
{

	int a = GetBufferMaxlength();
	EnterCriticalSection(&m_cs);
	if (ulLength > GetBufferMaxlength())
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
		MoveMemory(m_Base, m_Base + ulLength, GetBufferMaxlength() - ulLength);
		m_Ptr -= ulLength;
	}
	DeAllocateBuffer(GetBufferLength());
	LeaveCriticalSection(&m_cs);
	return ulLength;
}

PBYTE CBuffer::GetBuffer(ULONG ulPos)
{
	if (m_Base == NULL)
	{
		return NULL;

	}
	if (ulPos > GetBufferLength())
	{
		return NULL;
	}
	return m_Base + ulPos;
}
