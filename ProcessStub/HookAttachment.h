#pragma once
#include "Windows.h"

namespace TaskbarExtender {
	using namespace System;
public ref class HookData {
	public:
		String^ DllLocation;
		int FunctionOrdinal;
		HOOKPROC hkProc; 
		HINSTANCE hinstDll;
		HHOOK hhook;
	};

public ref class HookAttachment : public IDisposable
	{
	public:
		HookAttachment(String^ hookDllLocation, int funcOrdinal, int systemCallToHook);
		virtual ~HookAttachment();

	private:
		HookData^ data;
	};

}
