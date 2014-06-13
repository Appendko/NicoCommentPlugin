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

