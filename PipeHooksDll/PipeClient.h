

#pragma once

#include "Windows.h"
#include "atlstr.h"
#include <exception>

typedef void (*ErrorLogFunction)(const ATL::CString& msg);

template<typename T>
class PipeClient
{
public:
	explicit PipeClient(const ATL::CString& name);	

	virtual bool connectToServer(const ATL::CString& name = "", DWORD timeout = 0);
	virtual void disconnectFromServer();
	virtual bool writeToServer(const T& data);
	virtual T& readFromServer();
	virtual ~PipeClient(void);

	const T& getBufferConst() const { return buffer; }
	T& getBuffer() { return buffer; }

	const ATL::CString& getName() const { return name; }
	void setName(const ATL::CString new_name) { this.name = new_name; }

	void setErrorFunction(ErrorLogFunction err){ errorFunc = err; }


private:

	void writeError(const ATL::CString& msg) {
		if (errorFunc){		
			errorFunc(msg);
		}
	}

	PipeClient(void);
	HANDLE pipeHandle;
	ATL::CString name;
	char buffer[sizeof(T)];
	ErrorLogFunction errorFunc;
};

using namespace ATL;


template<typename T>
PipeClient<T>::PipeClient(const CString& name) : name(name), pipeHandle(0), errorFunc(NULL) 
{
}


template<typename T>
bool PipeClient<T>::connectToServer(const CString& new_name, DWORD timeout){
	if (pipeHandle && pipeHandle != INVALID_HANDLE_VALUE){
		CloseHandle(pipeHandle);
		pipeHandle = NULL;
	}
	if (new_name != ""){
		name = new_name;
	}


	for(;;) {
		pipeHandle = CreateFile((LPCTSTR)name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, 0, NULL);
		if (pipeHandle != INVALID_HANDLE_VALUE){
			break;
		} else {
			int err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PIPE_BUSY){
				Sleep(500);
			} else {
				throw new std::exception("wierd create file error");
			}
		}
	}
	/*
	while(!WaitNamedPipe(name, NMPWAIT_USE_DEFAULT_WAIT)) {
		if (GetLastError() == ERROR_SEM_TIMEOUT){
			Sleep(500);
		} else {
			writeError("client timed out waiting for server");
			return false;
		}
	}
	*/

	if (pipeHandle == INVALID_HANDLE_VALUE){
		ATL::CString msg = "Creating client pipe handle failed, error code: ";
		int error = GetLastError();
		char* erBuf = (char*)malloc((error / 10) + 2);	//one for \0 at end
		sprintf(erBuf, "%d", error);
		try {
			writeError(msg + erBuf);
		} catch (std::exception e) {
			free(erBuf);
			throw e;
		}
	}
	
	return true;
}


template<typename T>
void PipeClient<T>::disconnectFromServer(){
	if (pipeHandle){
		CloseHandle(pipeHandle);
	}
}

template<typename T>
bool PipeClient<T>::writeToServer(const T& data){
	if (pipeHandle == NULL) { return false; }

	DWORD bytes_read = 0;
	return WriteFile(pipeHandle, (LPCVOID)&data, sizeof(T), &bytes_read, NULL);
}

template<typename T>
T& PipeClient<T>::readFromServer(){
	if (pipeHandle == NULL) { 
		T* t = (T*)buffer;
		return *t; 
	}

	DWORD bytes_read = 0;
	bool success = ReadFile(pipeHandle, (LPVOID)buffer, sizeof(T), &bytes_read, NULL);
	T* data = (T*)buffer;
	return *data;
}



template<typename T>
PipeClient<T>::~PipeClient(void)
{
	if (pipeHandle){
		CloseHandle(pipeHandle);
	}
}


