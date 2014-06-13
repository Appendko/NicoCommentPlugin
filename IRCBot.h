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

class IRCBot {
	friend class IRCMsgThread; 

    SimpleSocket* TheSocket;
	
    bool threadRunning; //not for reconnect Task
    bool loginSuccessful;
	
    SendThread* sendThread; 
    ReceiveThread* receiveThread; 
    IRCMsgThread* ircmsgThread;
	
    SimpleTimer* aliveCheckTask;
    SimpleTimer* pingTask;
    SimpleTimer* loginCheckTask;
    SimpleTimer* reconnectTask;  
	
    //latest connect info
    std::wstring lastIRCServer;
    std::wstring lastIRCPort;
    std::wstring lastIRCServerPass;
    std::wstring lastIRCNickname;
    std::wstring lastIRCLogin;
	std::wstring lastIRCChannel;
         
	/********************
	* BASIC IRC Actions *
	********************/    
   
	bool connectTask(std::wstring server, std::wstring port, std::wstring nickname, std::wstring login, std::wstring password); //true for connect
    
public:
	IRCBot();
	~IRCBot();
	void IRC_join(std::wstring channel);
	void IRC_chat(std::wstring channel, std::wstring message);
    void AliveCheckTask();
    void ReconnectTask();
    void PingTask();
    void LoginCheckTask();
    bool isConnected();
    void reconnect();
	void connect(std::wstring server, std::wstring port, std::wstring nickname, std::wstring login, std::wstring password, std::wstring channel);
    void close();
    void onLoginFailed();
    void onLoginSuccess();
    void onAccidentDisconnection(); //the connection is closed by accident
    void onConnectSuccess();
	bool receiveMsg(std::wstring &message);
	bool QueueEmpty();
};
