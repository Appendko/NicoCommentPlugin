#include <wchar.h>
#include <string>
#include "wstringfunc.h"
#include "Log.h"

	/****************************
	* Default Logging Functions *
	****************************/
void onLog( const wchar_t * format, ... ){		
		wchar_t buffer[512];
		va_list args;
		va_start ( args, format );
		vswprintf_s ( buffer, 512, format, args );
		va_end ( args );
		//printf("LogFileOpen %ls",buffer);
		FILE* fp;
		errno_t err=_wfopen_s(&fp, LogFileName,L"a, ccs=UTF-8");
		if(err==0){
			fputws ( buffer, fp );
			fclose(fp);
		}
		//printf("LogFileClose %ls",buffer);
}

void onSysMsg( const wchar_t * format, ... ){
		wchar_t buffer[512];
		va_list args;
		va_start ( args, format );
		vswprintf_s ( buffer, 512 , format, args );
		va_end ( args );
		//printf("SysFileOpen %ls",buffer);
		FILE* fp; 
		errno_t err=_wfopen_s(&fp, SysFileName,L"a, ccs=UTF-8");
		if(err==0){
			fputws ( buffer, fp );
			fclose(fp);
		}
		//printf("SysFileClose %ls",buffer);
}

void onDebugMsg( const wchar_t * format, ... ){
		wchar_t buffer[512];
		va_list args;
		va_start ( args, format );
		vswprintf_s ( buffer, 512 , format, args );
		va_end ( args );
		//printf("DebugFileOpen %ls",buffer);
		FILE* fp; 
		errno_t err=_wfopen_s(&fp, DebugFileName,L"a, ccs=UTF-8");
		if(err==0){
			fputws ( buffer, fp );
			fclose(fp);
		}
		//printf("DebugFileClose %ls",buffer);
}

void onIRCMsg( const wchar_t * format, ... ){
		wchar_t buffer[512];
		va_list args;
		va_start ( args, format );
		vswprintf_s ( buffer, 512 , format, args );
		//printf("IRCFileOpen %ls",buffer);
		FILE* fp; 
		errno_t err=_wfopen_s(&fp, IRCFileName,L"a, ccs=UTF-8");
		if(err==0){
			fputws ( buffer, fp );
			fclose(fp);
		}
		//printf("IRCFileClose %ls",buffer);
}


void log(std::wstring msg, LogType type ){
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
			onLog(L"%ls\n",result.c_str()); //NEED onLog(wstring msg);
}

