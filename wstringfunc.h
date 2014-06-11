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