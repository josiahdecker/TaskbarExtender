#pragma once
#include "Windows.h"
#include "Shobjidl.h"
#include "atlbase.h"
#include "Winuser.h"
#include "SocketDll.h"

#import <mscorlib.tlb> raw_interfaces_only
#import "TaskbarComMessenger.tlb" no_namespace named_guids

class ComThread
{
public:
	explicit ComThread(HINSTANCE dllInstance, HANDLE comInitializationEvent, HANDLE comObjectClosedEvent, HANDLE processDetachEvent);
	~ComThread(void);

	HRESULT Success() const { return success; }
	void Loop();

private:
	HINSTANCE dllInstance;
	HWND windowHandle;
	
	HANDLE comObjectCloseEvent;
	HANDLE processDetachEvent;


	DWORD threadid;
	HRESULT success;
	CComQIPtr<ITaskbarComMessenger> messenger;

	ComThread(ComThread& that);
	ComThread& operator=(ComThread& that);
};

