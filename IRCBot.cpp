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
#include <cwchar>
#include <string>
#include "constants.h"
#include "Log.h"
#include "wstringfunc.h"

#include "SimpleSocket.h"
#include "SimpleThread.h"
#include "SimpleTimer.h"
//#include "SendReceiveThread.h"
#include "IRCMsgThread.h"
#include "IRCBot.h"
        
/********************
* BASIC IRC Actions *
********************/    
   
bool IRCBot::connectTask(std::wstring server, std::wstring port, std::wstring nickname, std::wstring login, std::wstring password){ //true for connect
    // Procedure for Connecting to the IRC server.
	// return false for default, true only if successfully connected.

		//int iResult;
		if ( TheSocket->InitializeSocket(server,port) ) // Initialize Winsock //0 for success, 1 for error 
			onDebugMsg(L"Error in socket initialization");
			
		if ( TheSocket->ConnectSocket() ) // Connect to server //0 for success, 1 for error
			onDebugMsg(L"Error in establishing socket connection");

		else { //Successfully Connected
			wchar_t buffer[256]=L"";
			swprintf_s(buffer,256,L"Connected to %ls(%ls):%ls",server.c_str(),TheSocket->SocketIP().c_str(),port.c_str());
            onSysMsg(L"%ls",buffer);
			                
            // Log on to the server, must sent before creating thread
			std::wstring msg=L"";
            if(password!=L""){
				msg+=L"PASS " + password + L"\r\n";
                log(L"PASS ******", LogType(SEND));
            }
            if(nickname!=(L"")){
                msg+=L"NICK " + nickname + L"\r\n";
                log(L"NICK " + nickname, LogType(SEND));
            }
            if(login!=(L"")){
                msg+=L"USER " + login + L"r\n";
                log(L"USER " + login, LogType(SEND));
            	}
			std::string narrowmsg = to_utf8(msg);
            TheSocket->send_data((char*)narrowmsg.c_str(),(unsigned int)narrowmsg.length()); //Sending Message without queue.
            loginSuccessful = false; //Not known yet; Initialize for further checking.
			
            //setup input and output thread 
            threadRunning = true; //not for reconnect Task
            //receiveThread->StartThread();
            ircmsgThread->StartThread();
            return true; //True for connect
        }
    return false;
}    

void IRCBot::IRC_join(std::wstring channel) { ircmsgThread->sendRaw(L"JOIN "+channel); }
void IRCBot::IRC_chat(std::wstring channel, std::wstring message) { ircmsgThread->sendRaw(L"PRIVMSG " + channel + L" :" + message); } 

IRCBot::IRCBot(){

	threadRunning = false; //not for reconnect Task
	loginSuccessful = false;

	TheSocket = new SimpleSocket;
	
	ircmsgThread = new IRCMsgThread;
	ircmsgThread->SetIRCBot(this);								
	ircmsgThread->SetSocket(TheSocket);
		
	//Timer Settings;
	aliveCheckTask = new SimpleTimer ; 
	aliveCheckTask->TickFunc=std::bind(&IRCBot::AliveCheckTask,this);
	aliveCheckTask->SetTickTime(5000);            

	reconnectTask = new SimpleTimer ;
	reconnectTask->TickFunc=std::bind(&IRCBot::ReconnectTask,this);
	reconnectTask->SetTickTime(5000); 

	lastIRCServer = L"";
	lastIRCPort = L"";
	lastIRCServerPass = L"";
	lastIRCNickname = L"";
	lastIRCLogin = L"";
	lastIRCChannel = L"";
}

IRCBot::~IRCBot(){
	close();
	delete aliveCheckTask; 
	delete reconnectTask;
	delete ircmsgThread;
	delete TheSocket ;
}

void IRCBot::AliveCheckTask() {
	if( !( ircmsgThread->isAlive() ) ){ 
		close();
		onAccidentDisconnection();
	} 
	else ircmsgThread->sendRaw(L"PING");
}

void IRCBot::ReconnectTask() {
	onDebugMsg(L"Reconnect Task");
	reconnect();
}

void IRCBot::LoginCheckTask() {
	if(!loginSuccessful){
		onSysMsg(L"Incorrect login information");
		onLoginFailed();
	}
}

bool IRCBot::isConnected(){
    if(TheSocket->isSocketInvalid()) return false;
    else if( TheSocket->isConnected() && ircmsgThread->isAlive() ) return true;
    else return false;
}

void IRCBot::reconnect(){
	bool bReconnectAlive=reconnectTask->isAlive();
	bool bConnect=connectTask(lastIRCServer,lastIRCPort,lastIRCNickname,lastIRCLogin,lastIRCServerPass);
    if(bConnect) { //success 
		if(bReconnectAlive) reconnectTask->StopThread();//Stop reconnectTask			
        onConnectSuccess();
    } else if(!bReconnectAlive) reconnectTask->StartThread();
}

void IRCBot::connect(std::wstring server, std::wstring port, std::wstring nickname, std::wstring login, std::wstring password, std::wstring channel){
	close();
    lastIRCServer = server;
    lastIRCPort = port;
    lastIRCServerPass = password;
    lastIRCNickname = nickname;
    lastIRCLogin = login;
	lastIRCChannel = channel;
    reconnect();
}

void IRCBot::close(){
        if(reconnectTask->isAlive()) {
			onDebugMsg(TEXT("Closing IRCBot-reconnectTask"));
			reconnectTask->StopThread();
		}
        if(threadRunning) {
				threadRunning = false;
				onDebugMsg(TEXT("Closing Running Threads of IRCBot"));
				//cancel alive checking task
				onDebugMsg(TEXT("Closing IRCBot-aliveCheckTask"));
				if(aliveCheckTask->isAlive()) aliveCheckTask->StopThread();
				//close threads
				onDebugMsg(TEXT("Closing IRCBot-ircmsgThread"));
				if(ircmsgThread->isAlive())	ircmsgThread->StopThread();
				onDebugMsg(L"All threads closed, IRCBot Stopped");
		}
}		


void IRCBot::onLoginFailed(){ //Call from LoginCheckTask or after ReceiveThread dropped connection
	onSysMsg(L"Failed to Login");
	close();
}

void IRCBot::onLoginSuccess(){ //Call from IRCMsgThread
	onSysMsg(L"Login Successfully");
	IRC_join(lastIRCChannel);
    ircmsgThread->sendRaw(L"TWITCHCLIENT 2");
}
  
void IRCBot::onAccidentDisconnection(){ //the connection is closed by accident
	onSysMsg(L"Accidentally disconnected from server: Reconnecting...");
	reconnect();
}

void IRCBot::onConnectSuccess(){
	onSysMsg(L"Connect Tasks Successfully Finished");
	Sleep(500);
	//setup disconnection checking timer
	if(ircmsgThread->isAlive())	 { //ircmsgThread will drop very quickly if login failed
		aliveCheckTask->StartThread();
		Sleep(1000); //wait for ircmsgThread parsing message.
		LoginCheckTask();
	}
	else{ //ircmsgThread Dropped-login fail or socket drop.
		onLoginFailed();
	}
}

bool IRCBot::receiveMsg(TircMsg &ircmsg) {return ircmsgThread->receiveMsg(ircmsg);}
bool IRCBot::QueueEmpty() {return ircmsgThread->QueueEmpty();}
