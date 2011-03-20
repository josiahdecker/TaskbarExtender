// TaskbarNativeCode.h


#include "Windows.h"
#include "PipeCommunicationStructures.h"
#include "PipeServer.h"
#include "SystemTaskbarCommunicator.h"
#include "AppbarMessenger.h"
#pragma once



namespace TaskbarExtender {
	namespace NativeMethods {

		public delegate void WindowMessageDelegate(ClrWindowMessage^ msg);

		public ref class NativeHookServer : System::IDisposable
		{
		public:
			NativeHookServer(System::String^ servername);
			virtual ~NativeHookServer();
			
			void StartListening() { messageLoop(); }
			event WindowMessageDelegate ^ MessageReceived;

		private:
			ClrWindowMessage^ getNextMessage();
			void messageLoop();

			PipeServer<WindowMessage>* server;
			HANDLE mailSlot;
		};

		public ref class NativeWindowUtilities
		{
		public:

			static System::IntPtr getIconHandle(System::IntPtr windowHandle){
				HWND hwnd = (HWND)windowHandle.ToPointer();
				hwnd = GetAncestor(hwnd, GA_ROOTOWNER);
				HICON icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);
				if (!icon){
					icon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0);
				}
				if (!icon){
					icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM);
				}
				if(!icon){
					icon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0);
				}
				if(!icon){
					icon = getIconFromModule(hwnd);
				}
				if(!icon){
					icon = LoadIcon(NULL, IDI_APPLICATION);
				}
				return (System::IntPtr)icon;
			}

			static HICON getIconFromModule(HWND handle){
				HINSTANCE inst = (HINSTANCE)GetWindowLongPtr(handle, GWLP_HINSTANCE);
				char buf[1000];
				LPWSTR filename = (LPWSTR)&buf[0];
				UINT read = GetWindowModuleFileName(handle, filename, 500);
				HICON icon = ExtractIcon(inst, filename, 0);
				icon = (int)icon == 1 ? NULL : icon;
				return icon;
			}
		};

	}
}
