#include "StdAfx.h"
#include "SystemTaskbarCommunicator.h"
#include "AppbarMessenger.h"

namespace TaskbarExtender{
	namespace NativeMethods {

SystemTaskbarCommunicator::SystemTaskbarCommunicator(void)
{
	taskbarList = new CComQIPtr<ITaskbarList>();
	CoInitialize(NULL);
	HRESULT success = taskbarList->CoCreateInstance(CLSID_TaskbarList);

	if (SUCCEEDED(success)){
		(*taskbarList)->HrInit();
	}else {
		CoUninitialize();
		throw gcnew System::Exception(System::String::Format("Taskbar communicator initialzation failed, error {0}", GetLastError()));
	}

}


SystemTaskbarCommunicator::~SystemTaskbarCommunicator(void)
{
	taskbarList->Release();
	CoUninitialize();
	delete taskbarList;
}

void SystemTaskbarCommunicator::AddTabToBar(System::IntPtr handle){
	(*taskbarList)->AddTab((HWND)handle.ToPointer());
}

void SystemTaskbarCommunicator::RemoveTabFromBar(System::IntPtr handle){
	(*taskbarList)->DeleteTab((HWND)handle.ToPointer());
}


}
}
