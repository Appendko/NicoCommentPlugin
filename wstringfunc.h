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
#include <vector>
#pragma once

std::vector<std::wstring> &split(const std::wstring &s, wchar_t delim, unsigned int maximum, std::vector<std::wstring> &elems);
std::vector<std::wstring> split(const std::wstring &s, wchar_t delim, unsigned int maximum);
std::wstring replaceFirst(std::wstring &s, std::wstring toReplace, std::wstring replaceWith);
std::wstring ToUpperString (std::wstring &s);
std::string ToUpperString (std::string &s);
std::wstring ToLowerString (std::wstring &s);
std::string ToLowerString (std::string &s);


std::string to_utf8(const wchar_t* buffer, int len);
std::string to_utf8(const std::wstring& str);
std::wstring from_utf8(const char* buffer, int len);
std::wstring from_utf8(const std::string& str);