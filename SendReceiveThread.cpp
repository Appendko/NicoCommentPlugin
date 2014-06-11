#include <iostream>
#include <sstream>
#include <cwchar>
#include "constants.h"
#include "Log.h"
#include "wstringfunc.h"

#include "SimpleSocket.h"
#include "SimpleThread.h"
#include "SendReceiveThread.h"


DWORD SendThread::Run() {

	unsigned int bufferlen;
	sendMsgQueue.clear();
	std::wstring msg;
	std::wstring sendstring;
	std::string utf8_string;
	while (!bKillThread) { //Doesn't need thread!=0: the function can only run with this thread.
		if ( !sendMsgQueue.empty() ) { //exist something to send
			bufferlen=0; //initialize: not neccessary
			sendstring=L"";
			while( !sendMsgQueue.empty() ||bKillThread){							 
				if( !sendMsgQueue.try_pop(msg) ) break; //true if an item was successfully dequeued,false otherwise.
				sendstring+=msg; 
				log(msg, LogType(SEND));
			}
			utf8_string=to_utf8(sendstring); //TwitchIRC use MultiByte
			while(utf8_string.length() > DEFAULT_BUFLEN ||bKillThread) { // Almost never happened....
				std::string smallstring=utf8_string.substr(0,DEFAULT_BUFLEN);
				utf8_string=utf8_string.substr(DEFAULT_BUFLEN);
				TheSocketPtr->send_data((char*)smallstring.c_str(),smallstring.length());
			}
			TheSocketPtr->send_data((char*)utf8_string.c_str(), utf8_string.length());
			
		}
		Sleep(100);
	}
	onSysMsg(L"SendThread closed\n");
	return 0;
}

void SendThread::sendRaw(std::wstring message) { sendMsgQueue.push(message+L"\r\n"); }
void SendThread::SetSocket(SimpleSocket* SocketPtr) { TheSocketPtr=SocketPtr; }

DWORD ReceiveThread::Run() { //Assume no huge message will be sent with two TCP packets due to the 512 bytes restrictions.
	unsigned int bufferlen;
	std::string recvstring,msg,umsg; //TwitchIRC use MultiByte
	recvMsgQueue.clear();
	
	while (!bKillThread) {
	    bufferlen=0;		
		memset(buffer, 0, DEFAULT_BUFLEN);
		if(TheSocketPtr->recv_data((char*)buffer, bufferlen)) break;
		recvstring=std::string(buffer,bufferlen);
		std::stringstream recvstr(recvstring);
		while( recvstr.tellg() < bufferlen || bKillThread ) {
			std::getline(recvstr,msg,'\n');
			int colon_pos = msg.find(":");
			if (msg[0] != ':' && colon_pos > 0) msg=msg.substr(colon_pos); //if an irc message begins with " ", then trim the leading spaces   
			if (msg[msg.length()-1]=='\r') msg=msg.substr(0,msg.length()-1); //trim the CR: getline only trim the LF		                   
			log(from_utf8(msg), LogType(RECEIVE)); //Although log file is in UTF8....using vswprintf
			umsg=msg.substr(0,5);
			ToUpperString(umsg);
               if (!umsg.compare("PING ")) { // respond to PINGs //Reply Immediately 
                   std::string response="PONG " + msg.substr(5) + "\r\n";
                   TheSocketPtr->send_data((char*)response.c_str(),response.length()); 
                   log(from_utf8(response), LogType(SEND));
               } else recvMsgQueue.push(from_utf8(msg)); // then parseMessage(msg) in IRCMsgThread;
		}
		Sleep(100);
	}
	onSysMsg(L"ReceiveThread closed\n");
	return 0;
}

bool ReceiveThread::receiveRaw(std::wstring &message) {return recvMsgQueue.try_pop(message);}
bool ReceiveThread::QueueEmpty() {return recvMsgQueue.empty();}
void ReceiveThread::SetSocket(SimpleSocket* SocketPtr) {TheSocketPtr=SocketPtr;}

