// This is the main DLL file.
#include "stdafx.h"
#include "Windows.h"
#include "TaskbarNativeCode.h"

namespace TaskbarExtender {
	namespace NativeMethods {
		using namespace System;
		
NativeHookServer::NativeHookServer(System::String^ serverName):
	server(NULL)	
{
	
	//server->Init();

	using namespace System::Runtime::InteropServices;
	IntPtr servername_ptr = Marshal::StringToHGlobalUni(serverName);
	LPCWSTR name = (LPCWSTR)servername_ptr.ToPointer();
	
	mailSlot = CreateMailslot(name, sizeof(WindowMessage), MAILSLOT_WAIT_FOREVER, NULL);
	
	Marshal::FreeHGlobal(servername_ptr);
}

NativeHookServer::~NativeHookServer(){
	CloseHandle(mailSlot);
}

ClrWindowMessage^ NativeHookServer::getNextMessage(){
	//bool connected = server->waitForNextConnection();
	//WindowMessage msg = server->readFromPipe();
	//server->closeConnection();

	WindowMessage msg;
	DWORD bytesRead = 0;

	ReadFile(mailSlot, (LPVOID)&msg, sizeof(msg), &bytesRead, nullptr); 

	ClrWindowMessage^ clr_msg = gcnew ClrWindowMessage(msg);
	//clr_msg->hwnd = msg.handle;
	//clr_msg->message_type = (TaskbarMessage)msg.message_type;
	//clr_msg->x = msg.x;
	//clr_msg->y = msg.y;

	return clr_msg;
}

void NativeHookServer::messageLoop(){
	for(;;){
		ClrWindowMessage^ msg = getNextMessage();
		MessageReceived(msg);
	}
}
	}
}