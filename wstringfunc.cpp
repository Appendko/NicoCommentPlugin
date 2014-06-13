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

#include <windows.h>
#include <cwctype>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include "wstringfunc.h"

//split from http://stackoverflow.com/questions/236129/how-to-split-a-wstring-in-c ;
std::vector<std::wstring> &split(const std::wstring &s, wchar_t delim, unsigned int maximum, std::vector<std::wstring> &elems) {
	std::wstringstream ss(s);
    std::wstring item;

    while (elems.size() < maximum-1) {
		if(std::getline(ss, item, delim)) {
			if( (item.length()==0)) continue;
			elems.push_back(item);
		}
		else break;
    }
	if ( (unsigned int)ss.tellg() < s.length()) {
		unsigned int i = (unsigned int)ss.tellg();
		while(i<s.length()) {
			if(s[i]==delim) i++;
			else {elems.push_back(s.substr(i)); break;}
		}
	}
    return elems;
}

std::vector<std::wstring> split(const std::wstring &s, wchar_t delim, unsigned int maximum) {
    std::vector<std::wstring> elems;
    split(s, delim, maximum, elems);
    return elems;
}

std::wstring replaceFirst(std::wstring &s,
                      std::wstring toReplace,
                      std::wstring replaceWith) {
    return(s.replace(s.find(toReplace), toReplace.length(), replaceWith));
}

std::wstring ToUpperString (std::wstring &s)
{ std::transform (s.begin(), s.end(), s.begin(), towupper); return s;}
std::string ToUpperString (std::string &s)
{ std::transform (s.begin(), s.end(), s.begin(), toupper); return s;}
std::wstring ToLowerString (std::wstring &s)
{ std::transform (s.begin(), s.end(), s.begin(), towlower); return s;}
std::string ToLowerString (std::string &s)
{ std::transform (s.begin(), s.end(), s.begin(), tolower); return s;}


std::string to_utf8(const wchar_t* buffer, int len) {
	int nChars = WideCharToMultiByte(CP_UTF8, 0, buffer, len,  NULL, 0, NULL, NULL);
	if (nChars == 0) return "";
	std::string newbuffer;
	newbuffer.resize(nChars);
	WideCharToMultiByte(CP_UTF8, 0, buffer, len, const_cast<char*>(newbuffer.c_str()), nChars, NULL, NULL); 
	return newbuffer;
}
 
std::string to_utf8(const std::wstring& str){
	return to_utf8(str.c_str(), (int)str.size());
}


std::wstring from_utf8(const char* buffer, int len){
	int nChars = MultiByteToWideChar(CP_UTF8, 0, buffer, len,  NULL, 0);
	if (nChars == 0) return L"";
	std::wstring newbuffer;
	newbuffer.resize(nChars);
	MultiByteToWideChar(CP_UTF8, 0, buffer, len, const_cast<wchar_t*>(newbuffer.c_str()), nChars); 
	return newbuffer;
}
std::wstring from_utf8(const std::string& str){
	return from_utf8(str.c_str(), (int)str.size());
}

