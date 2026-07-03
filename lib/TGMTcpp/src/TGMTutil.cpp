#include "TGMTutil.h"
#include <sstream>
#include <string>
#ifdef WIN32
#include "windows.h"
#endif
#include "TGMTdebugger.h"
#include <algorithm>
#include <random>
#include <cctype>
#include <cwctype>
#include <regex>

#ifdef OS_LINUX
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#endif
#define DEBUG_OUT_BUFFER_SIZE			(64*1024)

#include <codecvt>

#ifndef max
#define max std::max
#endif

#ifndef min
#define min std::min
#endif

//TGMTutil::TGMTutil()
//{
//}
//
//
//TGMTutil::~TGMTutil()
//{
//}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::IntToString(int number)
{
	std::stringstream ss;
	ss << number;
	return ss.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename WideCharT>
std::basic_string<WideCharT> convert_UTF8_to_UTF16(const std::string& str) {
	const char* str_ptr = str.c_str();
	int str_len = static_cast<int>(str.size());
	int wstr_len = MultiByteToWideChar(CP_UTF8, 0, str_ptr, str_len, nullptr, 0);
	std::basic_string<WideCharT> wstr(wstr_len, WideCharT{});
	MultiByteToWideChar(CP_UTF8, 0, str_ptr, str_len, reinterpret_cast<wchar_t*>(wstr.data()), wstr_len);
	return wstr;
}

template<typename WideCharT>
std::string convert_UTF16_to_UTF8(const std::basic_string<WideCharT>& wstr) {
	const wchar_t* wstr_ptr = reinterpret_cast<const wchar_t*>(wstr.c_str());
	int wstr_len = static_cast<int>(wstr.size());
	int str_len = WideCharToMultiByte(CP_UTF8, 0, wstr_ptr, wstr_len, nullptr, 0, nullptr, nullptr);
	std::string str(str_len, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr_ptr, wstr_len, str.data(), str_len, nullptr, nullptr);
	return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTutil::String2WString(const std::string s)
{
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
	//C++17 specific stuff here
	if (s.empty()) return std::wstring();
	return convert_UTF8_to_UTF16<wchar_t>(s);
#else
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(s);
#endif	
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::WString2String(const std::wstring s)
{
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
	//C++17 specific stuff here
	if (s.empty()) return std::string();
	return convert_UTF16_to_UTF8(s);
#else
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(s);
#endif	
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ANDROID
std::string TGMTutil::FormatString(const char* fmt, ...)
{
	va_list arg_list;
	char str[DEBUG_OUT_BUFFER_SIZE];
	va_start(arg_list, fmt);
	vsnprintf(str, DEBUG_OUT_BUFFER_SIZE - 1, fmt, arg_list);
	return str;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ANDROID

#ifdef UNICODE
std::string TGMTutil::GetParameter(int argc, wchar_t* argv[], char* key, char* defaultValue)
#else
std::string TGMTutil::GetParameter(int argc, char* argv[], char* key, const char* defaultValue)
#endif
{
	if (argv[0] == nullptr)
		return defaultValue;
	
	for (int i = 1; i < argc; i++)
	{
#ifdef UNICODE
		std::string strArgv = TGMTutil::WString2String(argv[i]);
		std::string strKey = key;
		
		if (strArgv == strKey)
		{
			if (argv[i + 1] != nullptr)
				return TGMTutil::WString2String(argv[i + 1]);
			else
				return defaultValue;
		}
#else
		if (strcmp((char*)argv[i], key) == 0)
		{
			if (argv[i + 1] != nullptr)
				return (char*)argv[i + 1];
			else
				return defaultValue;
		}
#endif
	}
	return defaultValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
bool TGMTutil::CheckParameterExist(int argc, wchar_t* argv[], char* key)
#else
bool TGMTutil::CheckParameterExist(int argc, char* argv[], char* key)
#endif
{
	if (TGMTutil::GetParameter(argc, argv, key) == "")
	{
		PrintError("Missing value of parameter %s", key);
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
bool TGMTutil::CheckParameterAloneExist(int argc, wchar_t* argv[], char* key)
#else
bool TGMTutil::CheckParameterAloneExist(int argc, char* argv[], char* key)
#endif
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp((char*)argv[i], key) == 0)
		{		
			return true;
		}
	}
	return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::ReplaceString(std::string &inputStr, char oldchar, char newchar)
{
	std::replace(inputStr.begin(), inputStr.end(), oldchar, newchar);
	return inputStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::ReplaceString(std::string &inputStr, std::string oldStr, std::string newStr)
{
	return std::regex_replace(inputStr, std::regex(oldStr), newStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTutil::ReplaceWstring(std::wstring& inputStr, std::wstring oldStr, std::wstring newStr)
{
	return std::regex_replace(inputStr, std::wregex(oldStr), newStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTutil::ReplaceWstring(std::wstring& inputStr, wchar_t oldStr, wchar_t newStr)
{
	return std::regex_replace(inputStr, std::wregex((wchar_t*)oldStr), (wchar_t*)newStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> TGMTutil::SplitString(std::string str, char separator)
{
	std::vector<std::string> elems;
	std::stringstream ss;
	ss.str(str);
	std::string item;
	while (std::getline(ss, item, separator)) {
		elems.push_back(item);
	}
	return elems;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::wstring> TGMTutil::SplitWString(std::wstring str, wchar_t separator)
{
	std::vector<std::wstring> elems;
	std::wstringstream ss;
	ss.str(str);
	std::wstring item;
	while (std::getline(ss, item,separator)) 
	{
		elems.push_back(item);
	}
	return elems;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTutil::GetRandomInt(int _min, int _max)
{
	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<> dist(_min, _max);
	return dist(rng);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::JoinVectorString(std::vector<std::string> strings, const char* separator)
{
	
	if (strings.size() == 0)
		return "";

	std::string result = "";

	for (size_t i = 0; i < strings.size(); i++)
	{
		result += strings[i];
		if (i < strings.size() - 1)
		{
			result += separator;
		}
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTutil::JoinVectorString(std::vector<std::wstring> strings, const wchar_t* separator)
{

	if (strings.size() == 0)
		return L"";

	std::wstring result = L"";

	for (size_t i = 0; i < strings.size(); i++)
	{
		result += strings[i];
		if (i < strings.size() - 1)
		{
			result += separator;
		}
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::RemoveString(std::string inputStr, char chrWantRemove)
{
	inputStr.erase(std::remove(inputStr.begin(), inputStr.end(), chrWantRemove), inputStr.end());
	return inputStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTutil::RemoveWstring(std::wstring wtext, std::wstring strWantRemove)
{
	std::wstring::size_type i = wtext.find(strWantRemove);

	if (i != std::wstring::npos)
		wtext.erase(i, strWantRemove.length());

	return wtext;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::RemoveSpecialCharacter(std::string inputStr)
{
	inputStr.resize(std::remove_if(inputStr.begin(), inputStr.end(), [](char x) {return !isalnum(x) && !isspace(x); }) - inputStr.begin());
	return inputStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTutil::RemoveSpecialCharacterW(std::wstring inputStr)
{
	inputStr.resize(std::remove_if(inputStr.begin(), inputStr.end(), [](wchar_t x) {return !iswalnum(x) && !iswspace(x); }) - inputStr.begin());
	return inputStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::ToAlpha(std::string inputStr)
{
	inputStr.erase(remove_if(inputStr.begin(), inputStr.end(), [](char c) { return !isalpha(c); }), inputStr.end());
	return inputStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::ToAlphanumeric(std::string inputStr)
{
	inputStr.erase(remove_if(inputStr.begin(), inputStr.end(), [](char c) { return !isalnum(c); }), inputStr.end());
	return inputStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::ToAscii(std::wstring inputStr)
{
	std::string asciiStr;
	for (wchar_t wc : inputStr) {
		if (wc <= 127) { // ASCII range
			asciiStr += static_cast<char>(wc);
		}
	}
	return asciiStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTutil::ToLowerW(std::wstring str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::ToLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void ltrim(std::string &s) 
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) 
	{
		return !std::isspace(ch);
	}));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void rtrim(std::string &s) 
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) 
	{
		return !std::isspace(ch);
	}).base(), s.end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void ltrim(std::wstring &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch)
	{
		return !std::iswspace(ch);
	}));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void rtrim(std::wstring &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
	{
		return !std::iswspace(ch);
	}).base(), s.end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTutil::WTrim(std::wstring str)
{
	ltrim(str);
	rtrim(str);
	return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::Trim(std::string str)
{
	ltrim(str);
	rtrim(str);
	return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTutil::IsNumber(std::string str)
{
	return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}

bool TGMTutil::IsNumber(std::wstring wstr)
{
	return std::all_of(wstr.begin(), wstr.end(), ::iswdigit); // C++11
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTutil::IsDecimal(std::string str)
{
	for (int i = 0; i < str.length(); i++)
	{
		char c = str[i];
		if (!isdigit(c) && c !='.' && c != ',')
			return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTutil::ContainNumber(std::string str)
{
	for (int i = 0; i < str.length(); i++)
	{
		if (isdigit(str[i]))
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTutil::ContainNumber(std::wstring wstr)
{
	for (int i = 0; i < wstr.length(); i++)
	{
		if (iswdigit(wstr[i]))
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTutil::StringToInt(std::string str)
{
	return atoi((char*)str.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTutil::StringToInt(std::wstring wstr)
{
	return atoi((char*)wstr.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined(WIN32) || defined(WIN64)) && defined(UNICODE)
wchar_t* TGMTutil::Utf2Wchar(const char *str)
{
	size_t size = strlen(str) + 1;
	wchar_t* wchr = new wchar_t[size];

	size_t outSize;
	mbstowcs_s(&outSize, wchr, size, str, size - 1);

	return wchr;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTutil::IsContain(std::string text, std::string str)
{
	size_t idx = text.find(str);
	return idx != std::string::npos;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTutil::IsContain(std::wstring text, std::wstring str)
{
	size_t idx = text.find(str);
	return idx != std::wstring::npos;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::GenerateRandomString(const int len)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[GetRandomInt() % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTutil::GetCurrentDateTime(bool removeSpecialCharacter)
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	if (removeSpecialCharacter)
	{
		std::string temp = buf;
		std::replace(temp.begin(), temp.end(), ':', '-');
		return temp;
	}

	return buf;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTutil::StringSimilarity(std::string str1, std::string str2)
{
	int m = str1.length();
	int n = str2.length();

	//std::vector<int> a = std::vector<int>(n);
	std::vector<std::vector<int>> T = std::vector<std::vector<int>>(m + 1);
	T[0] = std::vector<int>(n + 1);

	//int* T = new int[m + 1][n + 1];
	for (int i = 1; i <= m; i++) 
	{
		T[i] = std::vector<int>(n + 1);
		T[i][0] = i;
	}

	for (int j = 1; j <= n; j++) 
	{
		T[0][j] = j;
	}

	for (int i = 1; i <= m; i++) 
	{
		for (int j = 1; j <= n; j++) 
		{
			int weight = str1[i - 1] == str2[j - 1] ? 0 : 1;
			T[i][j] = min(min(T[i - 1][j] + 1, T[i][j - 1] + 1), T[i - 1][j - 1] + weight);
		}
	}

	int distance = T[m][n];

	double max_length = max(str1.length(), str2.length());
	if (max_length > 0) {
		return 100 * (max_length - distance) / max_length;
	}
	return 1.0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTutil::CountChar(std::string str1, char c)
{
	std::string str2 = TGMTutil::RemoveString(str1, c);
	return str1.length() - str2.length();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<std::wstring> VietnameseSigns =
{
	L"aAeEoOuUiIdDyY",
	L"áàạảãâấầậẩẫăắằặẳẵ",
	L"ÁÀẠẢÃÂẤẦẬẨẪĂẮẰẶẲẴ",
	L"éèẹẻẽêếềệểễ",
	L"ÉÈẸẺẼÊẾỀỆỂỄ",
	L"óòọỏõôốồộổỗơớờợởỡ",
	L"ÓÒỌỎÕÔỐỒỘỔỖƠỚỜỢỞỠ",
	L"úùụủũưứừựửữ",
	L"ÚÙỤỦŨƯỨỪỰỬỮ",
	L"íìịỉĩ",
	L"ÍÌỊỈĨ",
	L"đ",
	L"Đ",
	L"ýỳỵỷỹ",
	L"ÝỲỴỶỸ"
};


std::string TGMTutil::RemoveVNaccent(std::wstring accentedStr)
{
	for (int i = 1; i < VietnameseSigns.size(); i++)
	{
		for (int j = 0; j < VietnameseSigns[i].size(); j++)
		{
			std::wstring a = std::wstring(1, VietnameseSigns[i][j]);
			std::wstring b = std::wstring(1, VietnameseSigns[0][i - 1]);
			accentedStr = TGMTutil::ReplaceWstring(accentedStr, a, b);			
		}			
	}

	return WString2String(accentedStr);
}
