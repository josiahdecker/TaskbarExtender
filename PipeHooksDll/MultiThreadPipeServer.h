#pragma once
#include "Windows.h"
#include "atlstr.h"
#include <iostream>
#include "PipeServer.h"

template<typename T>
typedef PipeInstance<T>* auto_ptr<PipeInstanceHolder<T>>;

template<typename T>
class MultiThreadPipeServer
{
public:
	explicit MultiThreadPipeServer(CString pipeName, DWORD outBufferSize = sizeof(T), DWORD inBufferSize = sizeof(T), DWORD openMode = PIPE_ACCESS_DUPLEX,
		DWORD pipeMode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |PIPE_ACCEPT_REMOTE_CLIENTS, DWORD maxInstances = PIPE_UNLIMITED_INSTANCES,
		 DWORD defaultTimeOut = 0, LPSECURITY_ATTRIBUTES atts = NULL);
	
	virtual ~MultiThreadPipeServer(void);

	PipeInstance<T>* getNextInstance();

	class PipeInstanceHolder {
	public:
		explicit PipeInstanceHolder(const PipeServer<T>& serverInstance) : server(&serverInstance){}
		
		virtual ~PipeInstanceHolder() {
			delete server;
		}

		T& readMessage() { return  server->readFromPipe(); }
		void writeMessage(const T& msg) { server->writeToPipe(msg); }
		virtual ~PipeInstance() { delete server; }

	private:
		PipeInstanceHolder();
		PipeInstanceHolder(const PipeInstance<T>& that);
		PipeInstanceHolder<T> operator=(const PipeInstance<T>& that);

		PipeServer<T>* server;
	};

private:
	ATL::CString pipename;
	DWORD outBufferSize;				
	DWORD inBufferSize;					 
	DWORD openMode;						 
	DWORD pipeMode;				 
	DWORD maxInstances;				
	DWORD defaultTimeOut;				 
	LPSECURITY_ATTRIBUTES atts;
};

template<typename T>
MultiThreadPipeServer<T>::MultiThreadPipeServer(
		ATL::CString pipeName, 
		DWORD outBufferSize,				
		DWORD inBufferSize,					 
		DWORD openMode,						 
		DWORD pipeMode,						 
		DWORD maxInstances,					
		DWORD defaultTimeOut,				 
		LPSECURITY_ATTRIBUTES atts)			 
			 : pipeName(pipeName), outBufferSize(outBufferSize), inBufferSize(inBufferSize),
			 openMode(openMode), pipeMode(pipeMode), maxInstances(maxInstances), defaultTimeOut(defaultTimeOut),
			 atts(atts))
{
	
}

template<typename T>
PipeInstance<T>* MultiThreadPipeServer::getNextInstance(){
	PipeServer<T> server = new PipeServer<T>(pipeName, outBufferSize, inBufferSize, openMode, pipeMode, maxInstances, defaultTimeOut, atts);
	if (server == NULL) {
		PipeInstance<T>* blank(NULL);
		return blank;
	}
	PipeInstance<T> instance = new PipeInstanceHolder<T>(server);
	return instance;
}

