#include "SocketDll.h"
#include <string>
#include <sstream>
#include <stdio.h>
#include "PipeClient.h"
#include "MailboxClient.h"


HINSTANCE dllInstance;
MailboxClient<WindowMessage> mailboxClient(L"\\\\.\\mailslot\\TaskbarExtenderMailslot");


static
void write_debug(const char* msg, const char* data){
	FILE* file = fopen("c:\\Users\\josiah\\dll_debug.log", "a");
	fprintf(file, msg, data);
	fclose(file);
}

__declspec(dllexport) int __stdcall confirmLoad(){
	printf("Dll load successful");
	return 0;
}


static
void sendMessageToPipe(HWND handle, TASKBAR_MESSAGE message_type, int pos, int ypos);

static
void processWindowMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

static
void processDestroyedMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

static
RECT windowPosition(HWND handle){
	RECT pos;
	ZeroMemory(&pos, sizeof(RECT));
	GetWindowRect(handle, &pos);
	return pos;
}

static
bool isChildWindow(HWND hwnd) {
	WINDOWINFO info;
	info.cbSize = sizeof(WINDOWINFO);
	return (GetWindowInfo(hwnd, &info) && (info.dwStyle & WS_CHILD)); 
}

__declspec(dllexport) LRESULT CALLBACK windowSignalResponse(int ncode, WPARAM wparam, LPARAM lparam){
	if (ncode != HC_ACTION){
        return CallNextHookEx(NULL, ncode, wparam, lparam);
    } else {
        CWPSTRUCT* msg_info = (CWPSTRUCT*)lparam;
		//only interested in top level windows
		if (!isChildWindow(msg_info->hwnd) && IsWindowVisible(msg_info->hwnd)) {
				processWindowMessage(msg_info->hwnd, msg_info->message, msg_info->wParam, msg_info->lParam);
		}
    }
    return CallNextHookEx(NULL, ncode, wparam, lparam);
}

static 
void processWindowMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
        switch (msg){
			/*
            case WM_MOVE:
                int xpos = (int)(short)LOWORD(lparam);
                int ypos = (int)(short)HIWORD(lparam);
                std::stringstream message_out;
                message_out << "window moved\nxpos: " << xpos << "\n" << "ypos: " << ypos << std::endl;
                sendMessageToPipe(hwnd, xpos, ypos);
                break;
			
			case (WM_WINDOWPOSCHANGED):{
				WINDOWPOS* pos = (WINDOWPOS*)lparam;
				sendMessageToPipe(pos->hwnd, pos->x, pos ->y);
				break;
									   }
			
			case (WM_MOVING):{
				RECT* rect = (RECT*)lparam;
				sendMessageToPipe(hwnd, MSG_WINDOW_MOVED, rect->left, rect->top);
				break;
							 }
			
			case (WM_CREATE):{
				CREATESTRUCT* crStr = (CREATESTRUCT*)lparam;
				if (crStr->hwndParent == NULL) {
					sendMessageToPipe(hwnd, MSG_WINDOW_OPENED, crStr->x, crStr->y);
				}
				break;
							 }
			case (WM_CLOSE):{
				RECT pos;
				ZeroMemory(&pos, sizeof(RECT));
				GetWindowRect(hwnd, &pos);
				sendMessageToPipe(hwnd, MSG_WINDOW_CLOSED, pos.left, pos.top);
				break;
							  }
			*/
			case (WM_ENTERSIZEMOVE): {
				//PostThreadMessage(threadID, _MSG_WINDOW_MOVED, wparam, lparam);
				sendMessageToPipe(hwnd, MSG_SIZEMOVE_ENTER, 0, 0);
				break;
									 }
			//TODO- should be in the spawned thread?
			case (WM_EXITSIZEMOVE):{
				RECT pos;
				ZeroMemory(&pos, sizeof(RECT));
				GetWindowRect(hwnd, &pos);
				sendMessageToPipe(hwnd, MSG_SIZEMOVE_EXIT, pos.left, pos.top);
				break;
								   }
			case (WM_SIZE): {
				int type = (int)wparam;
				if (type == SIZE_MAXIMIZED){
					sendMessageToPipe(hwnd, MSG_SIZE_MAXIMIZED, 0, 0);
				} else if (type == SIZE_MINIMIZED) {
					sendMessageToPipe(hwnd, MSG_SIZE_MINIMIZED, 0, 0);
				} else if (type == SIZE_RESTORED){
					sendMessageToPipe(hwnd, MSG_SIZE_RESTORED, 0, 0);
				}
				break;
								 }
			case (WM_ACTIVATEAPP):{
				if ((BOOL)wparam){
					sendMessageToPipe(hwnd, MSG_WINDOW_ACTIVATED, 0, 0);
				}else {
					sendMessageToPipe(hwnd, MSG_WINDOW_DEACTIVATED, 0, 0);
				}
				break;
								  }
			
        }

}

__declspec(dllexport) LRESULT CALLBACK shellSignalResponse(int ncode, WPARAM wparam, LPARAM lparam){
	if (ncode < 0 ){
        return CallNextHookEx(NULL, ncode, wparam, lparam);
    } else {
       switch (ncode) {
			case HSHELL_WINDOWCREATED: {
				HWND handle = (HWND)wparam;
				RECT pos = windowPosition(handle);
				sendMessageToPipe(handle, MSG_WINDOW_OPENED, pos.left, pos.top);
				break;
									   }
			case HSHELL_WINDOWDESTROYED: {
				HWND handle = (HWND)wparam;
				RECT pos = windowPosition(handle);
				sendMessageToPipe(handle, MSG_WINDOW_CLOSED, pos.left, pos.top);
				break;
										 }
			/*case HSHELL_WINDOWREPLACED:
				log("window replaced: %d\n", wparam);
				log("replaced by: %d\n", lparam);
				break;
			*/
	   }
    }
    return CallNextHookEx(NULL, ncode, wparam, lparam);
}



static
void sendMessageToPipe(HWND handle,TASKBAR_MESSAGE message_type, int xpos, int ypos){

	WindowMessage msg; 
	
	msg.handle = (DWORD64)handle;
	msg.message_type = static_cast<int>(message_type);
	msg.x = xpos;
	msg.y = ypos;
	
	mailboxClient.sendMessage(msg);
}
