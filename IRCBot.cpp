#include <iostream>
#include <cwchar>
#include <string>
#include "constants.h"
#include "Log.h"
#include "wstringfunc.h"

#include "SimpleSocket.h"
#include "SimpleThread.h"
#include "SimpleTimer.h"
#include "SendReceiveThread.h"
#include "IRCMsgThread.h"
#include "IRCBot.h"
        
/********************
* BASIC IRC Actions *
********************/    
   
bool IRCBot::connectTask(std::wstring server, std::wstring port, std::wstring nickname, std::wstring login, std::wstring password){ //true for connect
    // Procedure for Connecting to the IRC server.
	// return false for default, true only if successfully connected.
    // try {
		//int iResult;
		if ( TheSocket->InitializeSocket(server,port) ) // Initialize Winsock //0 for success, 1 for error 
			onDebugMsg(L"Error in socket initialization.\n");
			
		if ( TheSocket->ConnectSocket() ) // Connect to server //0 for success, 1 for error
			onDebugMsg(L"Error in establishing socket connection.\n");

		else { //Successfully Connected
			wchar_t buffer[256]=L"";
			swprintf_s(buffer,256,L"Connected to %ls(%ls):%ls\n",server.c_str(),TheSocket->SocketIP().c_str(),port.c_str());
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
            TheSocket->send_data((char*)narrowmsg.c_str(),narrowmsg.length()); //Sending Message without queue.
            loginSuccessful = false; //Not known yet; Initialize for further checking.
			
            //setup input and output thread 
            threadRunning = true; //not for reconnect Task
            receiveThread->StartThread();
            sendThread->StartThread();
            ircmsgThread->StartThread();
			
            return true; //True for connect
        }
    // } catch() onSysLog(L"Cannot connect to "+server+L" ; \r\n");   
    return false;
}    

void IRCBot::IRC_join(std::wstring channel) { sendThread->sendRaw(L"JOIN "+channel); }
void IRCBot::IRC_chat(std::wstring channel, std::wstring message) { sendThread->sendRaw(L"PRIVMSG " + channel + L" :" + message); } 

IRCBot::IRCBot(){
	FILE* fp;
	_wfopen_s(&fp,LogFileName,L"w, ccs=UTF-8");   fwprintf(fp, L"IRCBot Log File Begin\n");   fclose(fp);
	_wfopen_s(&fp,SysFileName,L"w, ccs=UTF-8");   fwprintf(fp, L"IRCBot Sys File Begin\n");   fclose(fp);
	_wfopen_s(&fp,DebugFileName,L"w, ccs=UTF-8"); fwprintf(fp, L"IRCBot Debug File Begin\n"); fclose(fp);
	_wfopen_s(&fp,IRCFileName,L"w, ccs=UTF-8"); fwprintf(fp, L"IRCBot IRC File Begin\n"); fclose(fp);

	threadRunning = false; //not for reconnect Task
	loginSuccessful = false;

	TheSocket = new SimpleSocket;

    sendThread = new SendThread;
	sendThread->SetSocket(TheSocket);

	receiveThread = new ReceiveThread;
	receiveThread->SetSocket(TheSocket);

	ircmsgThread = new IRCMsgThread;
	ircmsgThread->SetReceiveThread(receiveThread);								
	ircmsgThread->SetIRCBot(this);								
		
	//Timer Settings;
	aliveCheckTask = new SimpleTimer ; 
	aliveCheckTask->TickFunc=std::bind(&IRCBot::AliveCheckTask,this);
	aliveCheckTask->SetTickTime(10000);
	
	pingTask = new SimpleTimer ;
	pingTask->TickFunc=std::bind(&IRCBot::PingTask,this);
	pingTask->SetTickTime(30000);                

	loginCheckTask = new SimpleTimer ;
	loginCheckTask->TickFunc=std::bind(&IRCBot::LoginCheckTask,this);
	loginCheckTask->SetTickTime(10000); 	
	
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
	delete pingTask;
	delete loginCheckTask;
	delete reconnectTask;

	delete receiveThread;
    delete sendThread;
	delete ircmsgThread;
	delete TheSocket ;

}

void IRCBot::AliveCheckTask() {
	if( !( sendThread->isAlive() && receiveThread->isAlive() ) ){ //One of these two is off
		close();
		onAccidentDisconnection();
	} 
}

void IRCBot::ReconnectTask() {
	onDebugMsg(L"Reconnect Task\n");
	reconnect();
}

void IRCBot::PingTask() { sendThread->sendRaw(L"PING"); }

void IRCBot::LoginCheckTask() {
	if(!loginSuccessful){
		onSysMsg(L"Incorrect login information\n");
		close();
		onLoginFailed();
	}
}

bool IRCBot::isConnected(){
    if(TheSocket->isSocketInvalid()) return false;
    else if( TheSocket->isConnected() && sendThread->isAlive() && receiveThread->isAlive() ) return true;
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
    lastIRCServer = server;
    lastIRCPort = port;
    lastIRCServerPass = password;
    lastIRCNickname = nickname;
    lastIRCLogin = login;
	lastIRCChannel = channel;
    reconnect();
}

void IRCBot::close(){
//      try{
        if(reconnectTask->isAlive()) reconnectTask->StopThread();
        if(threadRunning){
            //cancel alive checking task
            aliveCheckTask->StopThread();
            pingTask->StopThread();
            loginCheckTask->StopThread();
            onSysMsg(L"Timer closed\n");

            //close input and output thread
            threadRunning = false;
			ircmsgThread->StopThread();
            sendThread->StopThread();
			TheSocket->CloseSocket();
			receiveThread->StopThread();
			onSysMsg(L"Threads closed\n");

            onSysMsg(L"Disconnected from server\n");
        } else onSysMsg(L"Connection has already closed\n");
//		} catch(Exception e) {
//          if(e instanceof IOException){
//              //threw by writer
//          }
//      }
}


void IRCBot::onLoginFailed(){ //Call from LoginCheckTask
	onSysMsg(L"Failed to Login.\n");
}

void IRCBot::onLoginSuccess(){ //Call from IRCMsgThread
	onSysMsg(L"Login Successfully.\n");
	IRC_join(lastIRCChannel);
    sendThread->sendRaw(L"JTVCLIENT");
}
  
void IRCBot::onAccidentDisconnection(){ //the connection is closed by accident
	onSysMsg(L"Accidentally disconnected from server: Reconnecting...\n");
	reconnect();
}

void IRCBot::onConnectSuccess(){
	onSysMsg(L"Connect Tasks Successfully Finished.\n");
	//setup disconnection checking timer
	Sleep(2000);
	aliveCheckTask->StartThread();
	pingTask->StartThread();
	loginCheckTask->StartThread();
}

bool IRCBot::receiveMsg(std::wstring &message) {return ircmsgThread->receiveMsg(message);}
bool IRCBot::QueueEmpty() {return ircmsgThread->QueueEmpty();}
