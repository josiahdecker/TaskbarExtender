#include "Windows.h"
#include "atlstr.h"
#include <iostream>

#pragma once

enum ConnectionResult { SUCCESSFUL_CONNECTION, WAITING_FOR_DATA, ERROR_IN_CONNECTING };

template<typename T>
class PipeServer
{
public:
	explicit PipeServer(CString pipeName, DWORD outBufferSize = sizeof(T), DWORD inBufferSize = sizeof(T), DWORD openMode = PIPE_ACCESS_DUPLEX,
		DWORD pipeMode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |PIPE_ACCEPT_REMOTE_CLIENTS, DWORD maxInstances = PIPE_UNLIMITED_INSTANCES,
		 DWORD defaultTimeOut = 0, LPSECURITY_ATTRIBUTES atts = NULL);
	bool Init();
	virtual ~PipeServer(void);

	bool waitForNextConnection();
	virtual bool writeToPipe(const T& data);
	virtual T& readFromPipe();
	void closeConnection();

	bool getFailed() const { return failed; }
	bool getError() const { return error; }

private:
	PipeServer(void);
	PipeServer(PipeServer& other);
	PipeServer& operator=(PipeServer& other);
	
	ConnectionResult connectWaitLoop();

	ATL::CString pipeName; 
	DWORD outBufferSize; 
	DWORD inBufferSize; 
	DWORD openMode;
	DWORD pipeMode;
	DWORD maxInstances;
	DWORD defaultTimeOut;
	LPSECURITY_ATTRIBUTES atts;

	HANDLE pipeHandle;
	bool creationSucceeded;
	char buffer[sizeof(T)];
	bool failed;
	int error;
};


	using namespace ATL;

	template<typename T>
	PipeServer<T>::PipeServer(ATL::CString pipeName, 
		DWORD outBufferSize,				
		DWORD inBufferSize,					 
		DWORD openMode,						 
		DWORD pipeMode,						 
		DWORD maxInstances,					
		DWORD defaultTimeOut,				 
		LPSECURITY_ATTRIBUTES atts)			 
			 : pipeName(pipeName), outBufferSize(outBufferSize), inBufferSize(inBufferSize),
			 openMode(openMode), pipeMode(pipeMode), maxInstances(maxInstances), defaultTimeOut(defaultTimeOut),
			 atts(atts), pipeHandle(NULL), creationSucceeded(false), buffer(), failed(false), error(0)
	{
		memset(buffer, 0, sizeof(T));
	}

	template<typename T>
	bool PipeServer<T>::Init(){
		pipeHandle = CreateNamedPipe((LPCTSTR)pipeName, openMode, pipeMode, maxInstances, outBufferSize, inBufferSize, defaultTimeOut, atts);
		if (pipeHandle == INVALID_HANDLE_VALUE){
			error = GetLastError();
			std::cout << "Invalid pipe handle, error: " << error << "\n";
			failed = true;
			return false;
		}
		return true;
	}


	template<typename T>
	bool PipeServer<T>::waitForNextConnection(){
		ConnectionResult result;
		while ((result = connectWaitLoop()) == WAITING_FOR_DATA){
			Sleep(250);
		}
		if (result == SUCCESSFUL_CONNECTION){
			return true;
		} else {	//ERROR_IN_CONNECTING
			return false;
		}
	}

	
	template<typename T>
	ConnectionResult PipeServer<T>::connectWaitLoop(){
		bool result = ConnectNamedPipe(pipeHandle, NULL);
		int err = -1;
		if (!result && ((err = GetLastError()) != ERROR_PIPE_CONNECTED)){
			if (err == ERROR_NO_DATA){
				return WAITING_FOR_DATA;
			}
			std::cout << "Pipe failed to connect, error was: " << err << "\n";
			return ERROR_IN_CONNECTING;
		}
		return SUCCESSFUL_CONNECTION;
	}

	template<typename T>
	T& PipeServer<T>::readFromPipe(){
		if (!pipeHandle) { 
			memset((void*)buffer,0, sizeof(T)); 
			T* t = (T*)buffer;
			return *t;
		}
		DWORD bytes_read = 0;
		bool success = ReadFile(pipeHandle, (LPVOID)buffer, sizeof(T), &bytes_read, NULL);
		T*t = (T*)buffer;
		return *t;
	}

	template<typename T>
	bool PipeServer<T>::writeToPipe(const T& data){
		if (!pipeHandle) { return NULL; }
		DWORD bytes_read = 0;
		bool success = WriteFile(pipeHandle, (LPCVOID)&data, sizeof(T), &bytes_read, NULL);
		return success;
	}

	template<typename T>
	void PipeServer<T>::closeConnection(){
		DisconnectNamedPipe(pipeHandle);
	}

	template<typename T>
	PipeServer<T>::~PipeServer(void)
	{
		if (pipeHandle) {
			DisconnectNamedPipe(pipeHandle);
			CloseHandle(pipeHandle);
		}
	}




