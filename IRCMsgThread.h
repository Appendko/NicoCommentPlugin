#include <string>
#include <concurrent_queue.h>
#pragma once

class IRCMsgThread : public SimpleThread { // Dealing with Received Message
	friend class IRCBot;
	ReceiveThread *ReceiveThreadPtr;
	IRCBot *ircbotPtr;
	std::wstring recv_msg;
	Concurrency::concurrent_queue<std::wstring> IRCMsgQueue; 
	
	virtual DWORD Run();
	std::wstring getUsername(std::wstring sender);
	void onChatMsg(std::wstring channel, std::wstring nickname, bool isOp, std::wstring message);
    void onChatAction(std::wstring channel, std::wstring nickname, std::wstring action);
	void onPrivateMsg(std::wstring nickname, std::wstring message);
    void parseMessage(std::wstring message);
	
public:
	void SetReceiveThread(ReceiveThread* Ptr);
	void SetIRCBot(IRCBot* Ptr);
	bool receiveMsg(std::wstring &message);
	bool QueueEmpty();
};

