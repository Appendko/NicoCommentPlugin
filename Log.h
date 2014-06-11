#include <stdarg.h>
#include <string>
#pragma once

extern const wchar_t* LogFileName;
extern const wchar_t* SysFileName;
extern const wchar_t* DebugFileName;
extern const wchar_t* IRCFileName;

/****************************
* Default Logging Functions *
****************************/

void onLog( const wchar_t * format, ... );
void onSysMsg( const wchar_t * format, ... );
void onDebugMsg( const wchar_t * format, ... );
void onIRCMsg( const wchar_t * format, ... );

enum LogType{ SEND, RECEIVE };
void log(std::wstring msg, LogType type );