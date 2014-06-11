#include <string>
#include <concurrent_queue.h>

#pragma once

class SendThread : public SimpleThread {
    Concurrency::concurrent_queue<std::wstring> sendMsgQueue; 
	SimpleSocket* TheSocketPtr;
	virtual DWORD Run();

public:
	void sendRaw(std::wstring message);
	void SetSocket(SimpleSocket* SocketPtr);
};

class ReceiveThread : public SimpleThread {
	Concurrency::concurrent_queue<std::wstring> recvMsgQueue; 
	char buffer[DEFAULT_BUFLEN];
	SimpleSocket* TheSocketPtr;
	virtual DWORD Run();

public:
	bool receiveRaw(std::wstring &message);
	bool QueueEmpty();
	void SetSocket(SimpleSocket* SocketPtr);
};
