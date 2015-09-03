//------------------------------------------------------------------------------
// File: DibHelper.cpp
//
// Desc: DirectShow sample code - In-memory push mode source filter
//       Helper routines for manipulating bitmaps.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>

#include "CamStudioFilterHelper.h"

#include <stdio.h>
#include <assert.h>
#include <dwmapi.h>

void AddMouse(HDC hMemDC, LPRECT lpRect, HDC hScrDC, HWND hwnd) {
	POINT p;

	// GetCursorPos(&p); // get current x, y 0.008 ms
	
	CURSORINFO globalCursor;
	globalCursor.cbSize = sizeof(CURSORINFO); // could cache I guess...
	::GetCursorInfo(&globalCursor);
	HCURSOR hcur = globalCursor.hCursor;

	if(hwnd)
	  ScreenToClient(hwnd, &p); // 0.010ms
	else
	  GetCursorPos(&p);

	ICONINFO iconinfo;
	BOOL ret = ::GetIconInfo(hcur, &iconinfo); // 0.09ms

	if(ret) {
		p.x -= iconinfo.xHotspot; // align mouse, I guess...
		p.y -= iconinfo.yHotspot;

		// avoid some memory leak or other...
		if (iconinfo.hbmMask) {
			::DeleteObject(iconinfo.hbmMask);
		}
		if (iconinfo.hbmColor) {
			::DeleteObject(iconinfo.hbmColor);
		}
	}
	
	DrawIcon(hMemDC, p.x-lpRect->left, p.y-lpRect->top, hcur); // 0.042ms
}


void DrawMouse(HDC hMemDC,float fScaleX, float fScaleY,  DWORD  xHotspot, DWORD  yHotspot,LPRECT lpRect)
{
		//Draw the Cursor                
	CURSORINFO cursorinfo;
	memset(&cursorinfo, 0, sizeof(CURSORINFO));
	cursorinfo.cbSize = sizeof(CURSORINFO);
	// get cursor handle and position
	int ret = GetCursorInfo(&cursorinfo);
	if (ret == 0)
	{
		assert(false);
		return ;
	} 

	//if (m_uiCurMonitor == 2)
	//{
	//    cursorinfo.ptScreenPos.x -= mymonitor[1].offsetx;
	//} 
 
	if (cursorinfo.flags == CURSOR_SHOWING)
	{
	     cursorinfo.ptScreenPos.x -= lpRect->left;
	     cursorinfo.ptScreenPos.y -= lpRect->top; 
		::DrawIcon(hMemDC, (int)(cursorinfo.ptScreenPos.x*fScaleX-xHotspot), (int)(cursorinfo.ptScreenPos.y*fScaleY-yHotspot), cursorinfo.hCursor); 
	} 

}

 
//CamStudioFilterLogger::CamStudioFilterLogger() //: m_filePtr(NULL) 
//{
//	//Initialize();
//}
//CamStudioFilterLogger::~CamStudioFilterLogger(void)
//{
//}
