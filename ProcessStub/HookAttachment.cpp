#include "StdAfx.h"
#include "Windows.h"
#include "HookAttachment.h"





namespace TaskbarExtender {
	using namespace System;
	using namespace System::Runtime::InteropServices;

	HookAttachment::HookAttachment(String^ hook_dll_location, int ordinal, int systemCallToHook){
		data = gcnew HookData();
		data->DllLocation = hook_dll_location;
		data->FunctionOrdinal = ordinal;
		
		IntPtr location_ptr = Marshal::StringToHGlobalUni(hook_dll_location);
		LPCWSTR location = (LPCWSTR)location_ptr.ToPointer();
		data->hinstDll = LoadLibraryW(location);
		Marshal::FreeHGlobal(location_ptr);
	
		data->hkProc = (HOOKPROC)GetProcAddress(data->hinstDll, (LPCSTR)data->FunctionOrdinal); //"_windowSignalResponse@12" or "windowSignalResponse"
		data->hhook = SetWindowsHookEx(
				systemCallToHook,
				data->hkProc,
				data->hinstDll,
				0);
		if (!data->hhook) {
			int err = GetLastError();
			Console::WriteLine("Error setting hook- num: {0}", err);
		}
	}

	HookAttachment::~HookAttachment(){
		if (data->hhook){
			UnhookWindowsHookEx(data->hhook);
		}
	}

}