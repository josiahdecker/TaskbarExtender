
#include "windows.h"


#pragma once

#ifdef __cplusplus
extern "C" {
#endif


	enum TASKBAR_MESSAGE {
		MSG_WINDOW_MOVED = 1,
		MSG_WINDOW_OPENED = 2,
		MSG_WINDOW_CLOSED = 3,
		MSG_SIZEMOVE_EXIT = 4,
		MSG_SIZEMOVE_ENTER = 5,
		MSG_SIZE_MAXIMIZED = 6,
		MSG_SIZE_MINIMIZED = 7,
		MSG_SIZE_RESTORED = 8,
		MSG_WINDOW_ACTIVATED = 9,
		MSG_WINDOW_DEACTIVATED = 10,
		MSG_WINDOW_DESTROYED = 11
	};

	enum TASKBAR_WINDOW_MESSAGE {
		_MSG_WINDOW_MOVED = 0x8001,
		_MSG_WINDOW_OPENED = 0x8002,
		_MSG_WINDOW_CLOSED = 0x8003,
		_MSG_SIZEMOVE_EXIT = 0x8004,
		_MSG_SIZEMOVE_ENTER = 0x8005,
		_MSG_SIZE_MAXIMIZED = 0x8006,
		_MSG_SIZE_MINIMIZED = 0x8007,
		_MSG_SIZE_RESTORED = 0x8008,
		_MSG_WINDOW_ACTIVATED = 0x8009,
		_MSG_WINDOW_DEACTIVATED = 0x8010,
		_MSG_WINDOW_DESTROYED = 0x8011,
		_MSG_DLL_DETACHED = 0x8012
	};

	class WindowMessage {
		public:
			DWORD64 handle;
			int message_type;
			int x;
			int y;
		};

__declspec(dllexport) LRESULT CALLBACK windowSignalResponse(int ncode, WPARAM wparam, LPARAM lparam);
__declspec(dllexport) int __stdcall confirmLoad();

#ifdef __cplusplus
}
#endif