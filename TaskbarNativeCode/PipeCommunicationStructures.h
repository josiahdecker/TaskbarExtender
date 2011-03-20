#include "Windows.h"

#pragma once


#define TASKBAR_MESSAGE_WINDOW_MOVED 1
#define TASKBAR_MESSAGE_WINDOW_OPENED 2
#define TASKBAR_MESSAGE_WINDOW_CLOSED 3

namespace TaskbarExtender {
	namespace NativeMethods {

		//from SocketDll.h
		public enum class TaskbarMessage : int {
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

		//from SocketDll.h
		class WindowMessage {
		public:
			DWORD64 handle;
			int message_type;
			int x;
			int y;
		};

		public ref class ClrWindowMessage{
		public:
			ClrWindowMessage(const WindowMessage& msg) : hwnd(msg.handle), message_type((TaskbarMessage)msg.message_type), x(msg.x), y(msg.y) {}

			System::UInt64 hwnd;
			TaskbarMessage message_type;
			int x;
			int y;
		};

		
	}
}