#pragma once

#include "Windows.h"
#include "Shellapi.h"

namespace TaskbarExtender{
	namespace NativeMethods {

		public ref class AppbarMessenger
{
public:
	AppbarMessenger(System::IntPtr hwnd, System::String^ windowMessage);

	System::Windows::Rect QueryPosition(System::Windows::Rect pos, UINT edge);
	System::Windows::Rect SetPosition(System::Windows::Rect pos, UINT edge);
	
	System::Windows::Rect GetSystemTaskbarPosition();

	void Activate();

	void WindowPositionChanged();

	void Remove();

	bool MoveWindow(int x, int y, int width, int height, bool repaint);

	UINT GetMessageID() { return messageID; }

	const static UINT BOTTOM = ABE_BOTTOM;
	const static UINT LEFT = ABE_LEFT;
	const static UINT RIGHT = ABE_RIGHT;
	const static UINT TOP = ABE_TOP;

private:
	void createPositionStruct(System::Windows::Rect pos, UINT edge, APPBARDATA* data);

	UINT messageID;
	System::IntPtr hwnd;
};

	}
}

