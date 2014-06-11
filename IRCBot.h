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
