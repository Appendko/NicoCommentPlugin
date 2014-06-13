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

#include <stdarg.h>
#include <string>
#pragma once

/*extern const wchar_t* LogFileName;
extern const wchar_t* SysFileName;
extern const wchar_t* DebugFileName;
extern const wchar_t* IRCFileName;*/

/****************************
* Default Logging Functions *
****************************/

void onLog( const wchar_t * format, ... );
void onSysMsg( const wchar_t * format, ... );
void onDebugMsg( const wchar_t * format, ... );
void onIRCMsg( const wchar_t * format, ... );

enum LogType{ SEND, RECEIVE };
void log(std::wstring msg, LogType type );