
#include "Windows.h"
#include "Shobjidl.h"
#include "atlbase.h"
#include "Winuser.h"

#pragma once



namespace TaskbarExtender{
	namespace NativeMethods {


		public ref class SystemTaskbarCommunicator
{
public:
	SystemTaskbarCommunicator(void);
	virtual ~SystemTaskbarCommunicator(void);

	void RemoveTabFromBar(System::IntPtr handle);
	void AddTabToBar(System::IntPtr handle);

private:
	CComQIPtr<ITaskbarList>* taskbarList;
};

}
}

