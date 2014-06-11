#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Log.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma once

class SimpleSocket
{
    SOCKET TheSocket;
	WSADATA wsaData;
	struct addrinfoW *ptr, *result;
	std::wstring ipv4;

public:
	SimpleSocket();
	~SimpleSocket();
	bool InitializeSocket(std::wstring server, std::wstring port);
	bool CloseSocket();
	bool ConnectSocket();
	bool send_data(char* buffer, unsigned int bufferlen);	
	bool recv_data(char* buffer, unsigned int &bufferlen);
	inline bool isSocketInvalid() {return (TheSocket == INVALID_SOCKET) ;}
	inline bool isConnected() { return !(send_data((char*)L" ",1*sizeof(wchar_t)));};
	std::wstring SocketIP();
};

