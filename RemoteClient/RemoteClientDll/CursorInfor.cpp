#include "stdafx.h"
#include "CursorInfor.h"
#define MAX_CURSOR_TYPE 16

CCursorInfor::CCursorInfor()
{
}


CCursorInfor::~CCursorInfor()
{
}

int CCursorInfor::GetCurrentCursorIndex()
{
	CURSORINFO	ci;
	ci.cbSize = sizeof(CURSORINFO);
	if (!GetCursorInfo(&ci) || ci.cbSize != CURSOR_SHOWING)
	{
		return -1;
	}
	int Index = 0;
	for (Index = 0; Index < MAX_CURSOR_TYPE;Index++)
	{
		break;
	}
	DestroyCursor(ci.hCursor);
	return Index == MAX_CURSOR_TYPE ? -1 : Index;
}
