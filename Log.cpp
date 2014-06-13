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

#include "OBSApi.h"
#include <wchar.h>
#include <string>
#include "wstringfunc.h"
#include "Log.h"

	/****************************
	* Default Logging Functions *
	****************************/
void onLog( const wchar_t * format, ... ){		
/*		wchar_t buffer[512];
		va_list args;
		va_start ( args, format );
		vswprintf_s ( buffer, 512, format, args );
		va_end ( args );
		Log(TEXT("[NICO][LOG]%ls\n"),buffer);*/
/*		FILE* fp;
		errno_t err=_wfopen_s(&fp, LogFileName,L"a, ccs=UTF-8");
		if(err==0){
			fputws ( buffer, fp );
			fclose(fp);
		}*/
		
}

void onSysMsg( const wchar_t * format, ... ){
		wchar_t buffer[512];
		va_list args;
		va_start ( args, format );
		vswprintf_s ( buffer, 512 , format, args );
		va_end ( args );
		Log(TEXT("[NICO][SYS]%ls\n"),buffer);
/*		FILE* fp; 
		errno_t err=_wfopen_s(&fp, SysFileName,L"a, ccs=UTF-8");
		if(err==0){
			fputws ( buffer, fp );
			fclose(fp);
		}*/
		
}

void onDebugMsg( const wchar_t * format, ... ){
		wchar_t buffer[512];
		va_list args;
		va_start ( args, format );
		vswprintf_s ( buffer, 512 , format, args );
		va_end ( args );
		Log(TEXT("[NICO][DEBUG]%ls\n"),buffer);
/*		FILE* fp; 
		errno_t err=_wfopen_s(&fp, DebugFileName,L"a, ccs=UTF-8");
		if(err==0){
			fputws ( buffer, fp );
			fclose(fp);
		}*/
	
}

void onIRCMsg( const wchar_t * format, ... ){
/*		wchar_t buffer[512];
		va_list args;
		va_start ( args, format );
		vswprintf_s ( buffer, 512 , format, args );
		Log(TEXT("[NICO][IRC]%ls\n"),buffer);*/
/*		FILE* fp; 
		errno_t err=_wfopen_s(&fp, IRCFileName,L"a, ccs=UTF-8");
		if(err==0){
			fputws ( buffer, fp );
			fclose(fp);
		}*/

}


void log(std::wstring msg, LogType type ){/*
	// initialization
	std::wstring result=L"";
	
	
	// add prefix >>> and <<< for directions
	if(type == SEND) result+=L">>> ";
	else if(type == RECEIVE) result+=L"<<< ";
	
	//delete CRLF symbol
	while(msg.find(L"\r\n")!= std::wstring::npos ) replaceFirst(msg,L"\r\n",L"");
	result += msg;
	int len=result.length(); 
	//discard PING sent by client and PONG by server
	if(len>=8)
		if( result.substr(0,8).compare(L">>> PING") && result.substr(0,8).compare(L"<<< PONG") ) 
			onLog(L"%ls\n",result.c_str()); //NEED onLog(wstring msg);*/
}

