/********************************************************************************
 Copyright (C) 2014 Append Huang <Append@gmail.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/

#include <string>
#include <map>
#include <concurrent_queue.h>
#pragma once
enum IRCMsgThreadStatus {IRC_NORMAL,IRC_DISCONNECTED,IRC_WRONGLOGIN,IRC_CLOSED};
class IRCMsgThread : public SimpleThread { // Dealing with Received Message
	friend class IRCBot;
	IRCBot *ircbotPtr;
	std::wstring recv_msg;
	Concurrency::concurrent_queue<TircMsg> IRCMsgQueue; 
	std::map<std::wstring,unsigned int> UserColorMap;
	virtual DWORD Run();
	std::wstring getUsername(std::wstring sender);
	void onChatMsg(std::wstring channel, std::wstring nickname, bool isOp, std::wstring message);
    void onChatAction(std::wstring channel, std::wstring nickname, std::wstring action);
	void onPrivateMsg(std::wstring nickname, std::wstring message);
    void parseMessage(std::wstring message);
	void SetUserColor(std::wstring User,std::wstring Color);	
	unsigned int GetUserColor(std::wstring User);	
	char buffer[DEFAULT_BUFLEN];
	SimpleSocket* TheSocketPtr;
public:
	IRCMsgThreadStatus iStatus;
	void sendRaw(std::wstring message);
	void SetSocket(SimpleSocket* SocketPtr);
	void SetIRCBot(IRCBot* Ptr);
	bool receiveMsg(TircMsg &ircmsg);
	bool QueueEmpty();
	inline void interruptSignal(){bKillThread=true;}

};

