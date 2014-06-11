#include <iostream>
#include <cwchar>
#include "constants.h"
#include "Log.h"
#include "wstringfunc.h"

#include "SimpleSocket.h"
#include "SimpleThread.h"
#include "SendReceiveThread.h"
#include "SimpleTimer.h"
#include "IRCMsgThread.h"
#include "IRCBot.h"

DWORD IRCMsgThread::Run() {
	IRCMsgQueue.clear();
	while(!bKillThread ) { 	
		while(!ReceiveThreadPtr->QueueEmpty()||bKillThread) {
			ReceiveThreadPtr->receiveRaw(recv_msg);
			parseMessage(recv_msg);
		}
		Sleep(100);
	}
	onSysMsg(L"IRCMsgThread closed\n");
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
	IRCMsgQueue.push(message);
}

void IRCMsgThread::onChatAction(std::wstring channel, std::wstring nickname, std::wstring action){
    /*onIRCMsg(L"[ACTION] %ls %ls\n", nickname.c_str(), action.c_str());*/
}

void IRCMsgThread::onPrivateMsg(std::wstring nickname, std::wstring message){
    /*onIRCMsg(L"[PRIVATE] %ls: %ls\n", nickname.c_str(), message.c_str());
	//Parsing PRIVATEMSG
    if(!nickname.compare(L"jtv")){
        if(!message.substr(0,10).compare(L"USERCOLOR ")){
            vector<wstring> parse = split(message,L' ',3);
			onIRCMsg(L"[COLOR] %ls -> %ls\n", parse[1].c_str(), parse[2].c_str());
            //UserColorMapper.ins().setColor(parse[1], parse[2]); parse[1]=user, parse[2]=color.
        }else if(!message.substr(0,10).compare(L"CLEARCHAT ")){ //message.matches("^CLEARCHAT .*")
            vector<wstring> parse = split(message,L' ',2);
            onIRCMsg(L"[BAN OR TIMEOUT] %s has been banned or timeoutted\n",parse[1].c_str());
        }else if(!message.compare(L"CLEARCHAT")){
            onIRCMsg(L"[CLEARCHAT]\n");
        }else if(!message.substr(0,30).compare(L"You are banned from talking in")){
            onIRCMsg(L"[OOPS] %ls\n", message.c_str());
        }
    }*/		
}

void IRCMsgThread::parseMessage(std::wstring message){
	std::vector<std::wstring> firstParse = split(message,L' ',3);
    if(firstParse.size()<2) return; //Do Nothing
    if(!firstParse[1].compare(L"PRIVMSG")){
        std::vector<std::wstring> parse = split(message,L' ',4);
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
        if(!(getUsername(parse[0].substr(1)).compare(ToLowerString(ircbotPtr->lastIRCNickname)))){ //user succesffully joins a channel
      /*      onIRCMsg(L"Joined %ls\n",parse[2].c_str());*/
        }
	/*	else { //other users join the channel
			onIRCMsg(L"%ls Joined %ls\n",getUsername(parse[0].substr(1)).c_str(),parse[2].c_str());
		}*/
		
    }else if(!firstParse[1].compare(L"001")){ //login successful        
        ircbotPtr->loginSuccessful = true;
        ircbotPtr->onLoginSuccess();
    }
}

void IRCMsgThread::SetReceiveThread(ReceiveThread* Ptr){ReceiveThreadPtr=Ptr;};
void IRCMsgThread::SetIRCBot(IRCBot* Ptr){ircbotPtr=Ptr;};
bool IRCMsgThread::receiveMsg(std::wstring &message) {return IRCMsgQueue.try_pop(message);}
bool IRCMsgThread::QueueEmpty() {return IRCMsgQueue.empty();}
