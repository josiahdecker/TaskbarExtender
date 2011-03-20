#include "StdAfx.h"
#include "AppbarMessenger.h"
#include "Windows.h"
#include "Shellapi.h"

namespace TaskbarExtender{
	namespace NativeMethods {

using namespace System;
using namespace System::Windows;
using namespace System::Runtime::InteropServices;

AppbarMessenger::AppbarMessenger(IntPtr hwnd, String^ windowMessage) : hwnd(hwnd)
{
	IntPtr bmsg = Marshal::StringToBSTR(windowMessage);
	messageID = RegisterWindowMessage((LPCTSTR)bmsg.ToPointer());
	Marshal::FreeBSTR(bmsg);
	
	APPBARDATA data;
	data.hWnd = (HWND)hwnd.ToPointer();
	data.cbSize = sizeof(APPBARDATA);
	data.uCallbackMessage = messageID;

	SHAppBarMessage(ABM_NEW, &data);
}


void AppbarMessenger::createPositionStruct(Rect position, UINT edge, APPBARDATA* data) {
	data->hWnd = (HWND)hwnd.ToPointer();
	data->cbSize = sizeof(APPBARDATA);
	data->uEdge = edge;
	data->rc.left = (LONG)position.Left;
	data->rc.top = (LONG)position.Top;
	data->rc.right = (LONG)position.Right;
	data->rc.bottom = (LONG)position.Bottom;
}

Rect AppbarMessenger::QueryPosition(Rect pos, UINT edge) {
	APPBARDATA data;
	createPositionStruct(pos, edge, &data);
	
	SHAppBarMessage(ABM_QUERYPOS, &data);

	return Rect(
		Point(data.rc.left, data.rc.top), 
		Point(data.rc.right, data.rc.bottom)
		);
}

Rect AppbarMessenger::SetPosition(Rect pos, UINT edge) {
	APPBARDATA data;
	createPositionStruct(pos, edge, &data);

	bool success = SHAppBarMessage(ABM_SETPOS, &data);

	return Rect(
		Point(data.rc.left, data.rc.top), 
		Point(data.rc.right, data.rc.bottom)
		);
}

Rect AppbarMessenger::GetSystemTaskbarPosition(){
	APPBARDATA data;
	data.hWnd = (HWND)hwnd.ToPointer();
	data.cbSize = sizeof(APPBARDATA);

	SHAppBarMessage(ABM_GETTASKBARPOS, &data);

	return Rect(
		Point(data.rc.left, data.rc.top), 
		Point(data.rc.right, data.rc.bottom)
		);
}

void AppbarMessenger::Activate(){
	APPBARDATA data;
	data.hWnd = (HWND)hwnd.ToPointer();
	data.cbSize = sizeof(APPBARDATA);

	SHAppBarMessage(ABM_ACTIVATE, &data);
}

void AppbarMessenger::WindowPositionChanged(){
	APPBARDATA data;
	data.hWnd = (HWND)hwnd.ToPointer();
	data.cbSize = sizeof(APPBARDATA);

	SHAppBarMessage(ABM_WINDOWPOSCHANGED, &data);
}

void AppbarMessenger::Remove(){
	APPBARDATA data;
	data.hWnd = (HWND)hwnd.ToPointer();
	data.cbSize = sizeof(APPBARDATA);
	
	SHAppBarMessage(ABM_REMOVE, &data);
}

bool AppbarMessenger::MoveWindow(int x, int y, int width, int height, bool repaint){
	return ::MoveWindow((HWND)hwnd.ToPointer(), x, y, width, height, repaint);
}
	
	}
}