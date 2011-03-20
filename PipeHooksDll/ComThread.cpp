#include "ComThread.h"
#include "SocketDll.h"



LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	//any msg that makes it here isn't interesting for out purposes, just pass it on
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

ComThread::ComThread(HINSTANCE dllInstance, HANDLE threadInitializationEvent, HANDLE comObjectCloseEvent, HANDLE processDetachEvent) : dllInstance(dllInstance), messenger(), comObjectCloseEvent(comObjectCloseEvent), processDetachEvent(processDetachEvent)
{
	/*
	if (*wndClassAtom = 0) {
		WNDCLASSEX wnd;
		wnd.cbSize = sizeof(WNDCLASSEX);
		wnd.style = 0;
		wnd.lpfnWndProc = wndProc;
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hInstance = dllInstance;
		wnd.hIcon = NULL;
		wnd.hIconSm = NULL;
		wnd.hCursor = NULL;
		wnd.hbrBackground = NULL;
		wnd.lpszMenuName = NULL;
		wnd.lpszClassName = L"MessageThreadWnd";
		
		*wndClassAtom = RegisterClassEx(&wnd);
	}

	windowHandle = CreateWindow(L"MessageThreadWnd", L"messageReciever", WS_EX_NOPARENTNOTIFY, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL); 
	*/

	//forces creation of a message queue
	//unclear if this is needed, as CoInitialize indirectly creates windows, which theoretically should create a messge queue as well
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	success = messenger.CoCreateInstance(CLSID_TaskbarComMessenger);
	if (SUCCEEDED(success)){
		messenger->Initialize();
		//signal thread intialization
		SetEvent(threadInitializationEvent);
	} else {
		//TODO- error handling code
		//signal the event object anyway, so the creating thread can stop waiting
		SetEvent(threadInitializationEvent);
	}
}


ComThread::~ComThread(void)
{
	if (success) {
		messenger->Close();
	}
	CoUninitialize();
}

void ComThread::Loop()
{
	MSG msg;

	HANDLE waitingObjects[] = { comObjectCloseEvent, processDetachEvent };

	for (;;) {
		DWORD waitResult = MsgWaitForMultipleObjectsEx(2, waitingObjects, INFINITE, QS_ALLEVENTS, 0);

		if (waitResult == (WAIT_OBJECT_0 + 2)) {	//a message is in the queue
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
				if (msg.message == MSG_SIZEMOVE_EXIT) {
					messenger->Moved((long)msg.wParam, (int)msg.lParam);
				}  else {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		} else { //either we got an error, or one of the events was signaled, in either case we kill the thread
			//no call to ExitThread because we need destructors to run
			break;
		}
	}
}
