#include "windows.h"

#pragma once


template<typename T>
class MailboxClient {
public:
	explicit MailboxClient(LPCTSTR mailboxName);
	~MailboxClient();

	DWORD sendMessage(const T& message);
	BOOL success() const { return _success; }
	DWORD error() const { return _error; }

private:
	HANDLE mailbox;
	BOOL _success;
	DWORD _error;
	
	//No copying
	MailboxClient(const MailboxClient& that);
	MailboxClient& operator=(const MailboxClient& that);
};

template<typename T>
MailboxClient<T>::MailboxClient(LPCTSTR mailboxName) : _success(false), _error(0){
	mailbox = CreateFile(mailboxName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,  NULL);
	if (mailbox == INVALID_HANDLE_VALUE){
		_error = GetLastError();
	} else {
		_success = true;
	}
}

template<typename T>
MailboxClient<T>::~MailboxClient() {
	if (_success){
		CloseHandle(mailbox);
	}
}

template<typename T>
DWORD MailboxClient<T>::sendMessage(const T& message){
	DWORD bytesWritten = 0;
	if (_success) {
		WriteFile(mailbox, (LPCVOID)&message, sizeof(T), &bytesWritten, NULL);
	}

	return bytesWritten;
}