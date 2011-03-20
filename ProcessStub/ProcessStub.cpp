// ProcessStub.cpp : main project file.

#include "stdafx.h"
#include "HookAttachment.h"

using namespace System;


//attaches hooks and then waits for the close signal over stdin, hooks detach on close
int main(array<System::String ^> ^args)
{
	System::String^ dll_name = args[1];
	int ordinal = System::Int32::Parse(args[2]);
	int windowSignal = System::Int32::Parse(args[3]);

    TaskbarExtender::HookAttachment hookAttachment(dll_name, ordinal, windowSignal);

	for (;;) {
		System::String^ msg = Console::ReadLine();
		if (msg->ToLower()->Trim() == "close"){
			break;
		} else {
			Console::WriteLine("Got Message: " + msg);
		}
	}


    return 0;
}
