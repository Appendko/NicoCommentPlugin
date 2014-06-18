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

#include <iostream>
#include <sstream>
#include <cwchar>
#include <regex>
#include "constants.h"
#include "Log.h"
#include "wstringfunc.h"

#include "SimpleSocket.h"
#include "SimpleThread.h"
#include "IRCMsgThread.h"
#include "IRCBot.h"

DWORD IRCMsgThread::Run() {
	onSysMsg(L"IRCMsgThread Start");
	iStatus=IRC_NORMAL;
	IRCMsgQueue.clear();
	unsigned int bufferlen;
	std::string recvstring,msg,umsg; //TwitchIRC use MultiByte
	while(!bKillThread ) { 	
		//Receive First
		bufferlen=0;		
		memset(buffer, 0, DEFAULT_BUFLEN);
		if(TheSocketPtr->recv_data((char*)buffer, bufferlen)) { //Disconnected
			iStatus=IRC_DISCONNECTED;
			break;
		}
		recvstring=std::string(buffer,bufferlen);
		//onDebugMsg(L"%ls",from_utf8(recvstring).c_str());
		if(recvstring.compare(":tmi.twitch.tv NOTICE * :Login unsuccessful\r\n")==0 || //Twitch
		   recvstring.compare(":tmi.twitch.tv NOTICE * :Error encountered while attempting login\r\n")==0 ){ //Justin
			Sleep(100); //Wait for logging
			onSysMsg(L"Login unsuccessful");
			iStatus=IRC_WRONGLOGIN;
			break;
		}//else if(recvstring.compare(":append!append@append.tmi.twitch.tv PRIVMSG #append :break\r\n")==0){TheSocketPtr->CloseSocket();}
		//Basic parsing: make it into a string, check for PING, if not-send for further parsing.
		std::stringstream recvstr(recvstring);
		while( recvstr.tellg() < bufferlen ) {
			std::getline(recvstr,msg,'\n');
			int colon_pos = (int)msg.find(":");
			if (msg[0] != ':' && colon_pos > 0) msg=msg.substr(colon_pos); //if an irc message begins with " ", then trim the leading spaces   
			if (msg[msg.length()-1]=='\r') msg=msg.substr(0,msg.length()-1); //trim the CR: getline only trim the LF		                   
			log(from_utf8(msg), LogType(RECEIVE)); //Although log file is in UTF8....using vswprintf
			umsg=msg.substr(0,5);
			ToUpperString(umsg);
            if (!umsg.compare("PING ")) sendRaw( L"PONG " + from_utf8(msg.substr(5)) ); // respond to PINGs //Reply Immediately 
            else parseMessage(from_utf8(msg));
		}
		Sleep(100);
	}
	//Determine the reason to break the loop.
	if(!bKillThread) {
		if(iStatus==IRC_DISCONNECTED) onDebugMsg(L"IRCMsgThread: Disconnect Unexpectedly");
		if(iStatus==IRC_WRONGLOGIN) onDebugMsg(L"IRCMsgThread: Wrong Login Information");
	}
	else iStatus=IRC_CLOSED;
	//onDebugMsg(L"[Last Buffer]%ls",from_utf8(recvstring).c_str());
	onSysMsg(L"IRCMsgThread Close");
	thread=0;
	return 0;
}

std::wstring IRCMsgThread::getUsername(std::wstring sender){
    //parse twitch username from twitch chats
    //ex: user!user@user.tmi.twitch.tv -> user
    if(sender.find(L"!")!=std::wstring::npos){
        return sender.substr(0,sender.find(L"!"));
    } else return sender;
}

void IRCMsgThread::onChatMsg(std::wstring channel, std::wstring nickname, bool isOp, std::wstring message){
    onIRCMsg(L"[CHAT] %ls: %ls\n", nickname.c_str(), message.c_str());
	if(nickname.compare(L"nightbot")!=0&&nickname.compare(L"moobot")!=0&&nickname.compare(L"jtv")!=0) {
		TircMsg ircMsg;
		ircMsg.user=nickname;
		ircMsg.msg=message;
		ircMsg.usercolor=GetUserColor(nickname);
		IRCMsgQueue.push(ircMsg);
	}
}

void IRCMsgThread::onChatAction(std::wstring channel, std::wstring nickname, std::wstring action){
    /*onIRCMsg(L"[ACTION] %ls %ls\n", nickname.c_str(), action.c_str());*/
}

void IRCMsgThread::onPrivateMsg(std::wstring nickname, std::wstring message){
    //onIRCMsg(L"[PRIVATE] %ls: %ls\n", nickname.c_str(), message.c_str());
	//Parsing PRIVATEMSG
    if(!nickname.compare(L"jtv")){
        if(!message.substr(0,10).compare(L"USERCOLOR ")){
            std::vector<std::wstring> parse = split(message,L' ',3);
			SetUserColor(parse[1],ToLowerString(parse[2]));
			//onDebugMsg(L"[COLOR] %ls -> %ls\n", parse[1].c_str(), parse[2].c_str());
        }
		/*else if(!message.substr(0,10).compare(L"CLEARCHAT ")){ //message.matches("^CLEARCHAT .*")
            vector<wstring> parse = split(message,L' ',2);
            onDebugMsg(L"[BAN OR TIMEOUT] %s has been banned or timeoutted\n",parse[1].c_str());
        }else if(!message.compare(L"CLEARCHAT")){
            onDebugMsg(L"[CLEARCHAT]\n");
        }else if(!message.substr(0,30).compare(L"You are banned from talking in")){
            onDebugMsg(L"[OOPS] %ls\n", message.c_str());
        }*/
    }		
}

void IRCMsgThread::parseMessage(std::wstring message){
	std::vector<std::wstring> firstParse = split(message,L' ',3);
    if(firstParse.size()<2) return; //Do Nothing
    if(!firstParse[1].compare(L"PRIVMSG")){
        std::vector<std::wstring> parse = split(message,L' ',4);
		if(parse.size() < 4) return; //discard this incomplete message
        /*
         * parse[0].substr(1): sender
         * parse[2]: target channel/user
         * parse[3].subString(1): message
         */
        if(parse[2][0]==L'#'){
            std::wstring content = parse[3].substr(1);	
			
            if(!content.substr(0,8).compare(L"ACTION ")){
                onChatAction(parse[2].c_str(), getUsername(parse[0].substr(1)).c_str(), replaceFirst(content.substr(8),L"", L"").c_str());
            }else{
                onChatMsg(parse[2].c_str(), getUsername(parse[0].substr(1)).c_str(), false, parse[3].substr(1).c_str());
            }
        }else{
            onPrivateMsg(getUsername(parse[0].substr(1)), parse[3].substr(1));
        }
        
    }else if(!firstParse[1].compare(L"JOIN")){
        std::vector<std::wstring> parse = split(message,L' ',3);
		if(parse.size() < 3) return; //discard this incomplete message
        if(!(getUsername(parse[0].substr(1)).compare(ToLowerString(ircbotPtr->lastIRCNickname)))){ //user succesffully joins a channel
            onSysMsg(L"Joined %ls",parse[2].c_str());
        }
	/*	else { //other users join the channel
			onIRCMsg(L"%ls Joined %ls\n",getUsername(parse[0].substr(1)).c_str(),parse[2].c_str());
		}*/
		
    }else if(!firstParse[1].compare(L"001")){ //login successful        
        ircbotPtr->loginSuccessful = true;
        ircbotPtr->onLoginSuccess();
    }
}

void IRCMsgThread::SetUserColor(std::wstring User,std::wstring Color){ //already lowercase
	//std::map<std::wstring,unsigned int> UserColorMap
	unsigned int iColor;
	std::wregex color_pattern(L"^#[0-9a-fA-F]{6}$");
	//std::wsmatch base_match;
	if(std::regex_match(Color, color_pattern)){
		int iErrno=swscanf_s(Color.substr(1).c_str(),L"%x",&iColor);
	}
	else {
		iColor=1<<24;
		if(Color.compare(L"red")==0)				iColor=0x00FF0000;
		else if(Color.compare(L"blue")==0)			iColor=0x000000FF;
		else if(Color.compare(L"green")==0)			iColor=0x00008000;
		else if(Color.compare(L"firebrick")==0)		iColor=0x00B22222;
		else if(Color.compare(L"coral")==0)			iColor=0x00FF7F50;
		else if(Color.compare(L"yellowgreen")==0)	iColor=0x009ACD32;
		else if(Color.compare(L"orangered")==0)		iColor=0x00FF4500;
		else if(Color.compare(L"seagreen")==0)		iColor=0x002E8B57;
		else if(Color.compare(L"goldenrod")==0)		iColor=0x00D2691E;
		else if(Color.compare(L"cadetblue")==0)		iColor=0x005F9EA0;
		else if(Color.compare(L"dodgerblue")==0)	iColor=0x001E90FF;
		else if(Color.compare(L"hotpink")==0)		iColor=0x00FF69B4;
		else if(Color.compare(L"blueviolet")==0)	iColor=0x008A2BE2;
		else if(Color.compare(L"springgreen")==0)	iColor=0x0000FF7F;
		else if(Color.compare(L"black")==0)			iColor=0x00000000;
		else if(Color.compare(L"gray")==0)			iColor=0x00808080;
		else if(Color.compare(L"darkred")==0)		iColor=0x008B0000;
		else if(Color.compare(L"midnightblue")==0)	iColor=0x00191970;
		else if(Color.compare(L"deeppink")==0)		iColor=0x00FF1493;
	}
	//onDebugMsg(L"SetUserColor: %ls, %ls, 0X%08X\n",User.c_str(),Color.c_str(),iColor);
		UserColorMap.insert( std::pair<std::wstring,unsigned int>(User,iColor|0xFF000000) );
}

unsigned int IRCMsgThread::GetUserColor(std::wstring User){
	std::map<std::wstring,unsigned int>::iterator it=UserColorMap.find(User);
	if(it==UserColorMap.end()) {
		//onDebugMsg(L"GetUserColor: %ls, NOTFOUND\n",User.c_str());
		return (unsigned int)0; 
	}
	else {
		//onDebugMsg(L"GetUserColor: %ls, 0X%08X\n",User.c_str(),it->second);
		return it->second;
	}
}

void IRCMsgThread::sendRaw(std::wstring message){
		std::string send_msg=to_utf8(message+L"\r\n");
	TheSocketPtr->send_data((char*)send_msg.c_str(),(unsigned int)send_msg.length()); 
    log(message, LogType(SEND));
}

bool IRCMsgThread::receiveMsg(TircMsg &ircmsg) {return IRCMsgQueue.try_pop(ircmsg);}
void IRCMsgThread::SetSocket(SimpleSocket* SocketPtr) {TheSocketPtr=SocketPtr;}
void IRCMsgThread::SetIRCBot(IRCBot* Ptr){ircbotPtr=Ptr;};
bool IRCMsgThread::QueueEmpty() {return IRCMsgQueue.empty();}

