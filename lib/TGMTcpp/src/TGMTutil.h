#pragma once
#include <vector>
#include <string>

#define Str2Int TGMTutil::StringToInt
#define Int2Str TGMTutil::IntToString
class TGMTutil
{
public:
	//TGMTutil();
	//~TGMTutil();

	static std::string IntToString(int number);
	static int StringToInt(std::string str);
	static int StringToInt(std::wstring str);
	static std::wstring String2WString(const std::string s);
	static std::string WString2String(const std::wstring s);
	
#ifndef ANDROID
	static std::string FormatString(const char* fmt, ...);
#endif
	static std::string ReplaceString(std::string &inputStr, char oldchar, char newchar);
	static std::string ReplaceString(std::string &inputStr, std::string oldStr, std::string newStr);
	static std::wstring ReplaceWstring(std::wstring& inputStr, std::wstring oldStr, std::wstring newStr);
	static std::wstring ReplaceWstring(std::wstring& inputStr, wchar_t oldStr, wchar_t newStr);
	static std::string RemoveString(std::string inputStr, char chrWantRemove);
	static std::wstring RemoveWstring(std::wstring wtext, std::wstring strWantRemove);
	static std::string RemoveSpecialCharacter(std::string inputStr);
	static std::wstring RemoveSpecialCharacterW(std::wstring inputStr);
	static std::string ToAlpha(std::string inputStr);
	static std::string ToAlphanumeric(std::string inputStr);
	static std::string ToAscii(std::wstring inputStr);

#ifndef ANDROID
#ifdef UNICODE
	static std::string GetParameter(int argc, wchar_t* argv[], char* key, char* defaultValue = "");
	static bool CheckParameterExist(int argc, wchar_t* argv[], char* key);
	static bool CheckParameterAloneExist(int argc, wchar_t* argv[], char* key);
	static wchar_t* Utf2Wchar(const char *str);
#else
	static std::string GetParameter(int argc, char* argv[], char* key, const char* defaultValue = "");
	static bool CheckParameterExist(int argc, char* argv[], char* key);
	static bool CheckParameterAloneExist(int argc, char* argv[], char* key);
#endif
	
#endif

	
	static std::vector<std::string> SplitString(std::string str, char separator);
	static std::vector<std::wstring> SplitWString(std::wstring str, wchar_t separator);
	static std::string JoinVectorString(std::vector<std::string> strings, const char* separator = ";");
	static std::wstring JoinVectorString(std::vector<std::wstring> strings, const wchar_t* separator);

	static int GetRandomInt(int min = 0, int max = 255);

	static std::wstring ToLowerW(std::wstring str);
	static std::string ToLower(std::string str);

	static std::wstring WTrim(std::wstring str);
	static std::string Trim(std::string str);

	static bool IsNumber(std::string str);
	static bool IsNumber(std::wstring wstr);
	static bool IsDecimal(std::string str);

	static bool ContainNumber(std::string str);
	static bool ContainNumber(std::wstring wstr);

	static bool IsContain(std::string text, std::string str);
	static bool IsContain(std::wstring text, std::wstring str);
	static std::string GenerateRandomString(const int len = 10);

	static std::string GetCurrentDateTime(bool removeSpecialCharacter);

	static int StringSimilarity(std::string str1, std::string str2);
	static int CountChar(std::string str1, char c);
	static std::string RemoveVNaccent(std::wstring accentedStr);
};

