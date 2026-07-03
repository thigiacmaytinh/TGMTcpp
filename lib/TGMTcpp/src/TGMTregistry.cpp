#include "TGMTregistry.h"
#include "TGMTdebugger.h"
#include "TGMTutil.h"

#ifdef UNICODE
#define TGMTSTR LPCWSTR
#else
#define TGMTSTR LPCSTR
#endif

TGMTregistry* TGMTregistry::m_instance = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTregistry::TGMTregistry()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTregistry::~TGMTregistry()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTregistry::Init(std::string key)
{
	long lRes = RegOpenKeyEx(HKEY_CURRENT_USER, (TGMTSTR)("SOFTWARE\\" + key).c_str(), 0, KEY_ALL_ACCESS, &hKey);
	
	bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
	if (bDoesNotExistsSpecifically)
	{
		lRes = RegCreateKeyEx(HKEY_CURRENT_USER, (TGMTSTR)("SOFTWARE\\" + key).c_str(), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
		DEBUG_OUT("Create registry key %s", key.c_str());
	}
	m_inited = lRes == ERROR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTregistry::WriteValueString(std::string key, std::string value)
{
	if (!m_inited)
	{
		PrintError("you must init registry key before use");
		return;
	}
#ifdef UNICODE
	LONG ret = RegSetValueEx(hKey, (LPCWSTR)key.c_str(), NULL, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
#else
	LONG ret = RegSetValueEx(hKey, (LPCSTR)key.c_str(), NULL, REG_SZ, (LPBYTE)value.c_str(), value.length() + 1);
#endif
	PrintResult(ret);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTregistry::WriteValueBool(std::string valueName, bool valueData)
{
	if (!m_inited)
	{
		PrintError("you must init registry key before use");
		return;
	}

	LONG ret = RegSetValueEx(hKey, (TGMTSTR)valueName.c_str(), NULL, REG_DWORD, (LPBYTE)&valueData, sizeof(DWORD));

	PrintResult(ret);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTregistry::WriteValueInt(std::string valueName, int valueData)
{
	if (!m_inited)
	{
		PrintError("you must init registry key before use");
		return;
	}
	RegSetValueEx(hKey, (TGMTSTR)valueName.c_str(), NULL, REG_DWORD, (LPBYTE)&valueData, sizeof(DWORD));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTregistry::WriteValueFloat(std::string valueName, float valueData)
{	
	std::string value = TGMTutil::FormatString("%f", valueData);
	WriteValueString(valueName, value);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTregistry::ReadValueInt(std::string key, int defaultValue)
{
	std::wstring wstrKey = TGMTutil::String2WString(key);
	return ReadValueInt(wstrKey, defaultValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTregistry::ReadValueInt(std::wstring key, int defaultValue)
{
	if (!m_inited)
	{
		PrintError("you must init registry key before use");
		return defaultValue;
	}
	int value = defaultValue;
	DWORD dwBufferSize(sizeof(DWORD));
	DWORD nResult(0);
	LONG nError = ::RegQueryValueExW(hKey,
		key.c_str(),
		0,
		NULL,
		reinterpret_cast<LPBYTE>(&nResult),
		&dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		value = nResult;
	}
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

float TGMTregistry::ReadValueFloat(std::string key, float defaultValue)
{
	std::string value = ReadValueString(key);
	if (value.empty())
		return defaultValue;
	return stof(value);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTregistry::ReadValueBool(std::string key, bool defaultValue)
{
	std::wstring wstrKey = TGMTutil::String2WString(key);
	return ReadValueBool(wstrKey, defaultValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTregistry::ReadValueBool(std::wstring key, bool defaultValue)
{
	if (!m_inited)
	{
		PrintError("you must init registry key before use");
		return defaultValue;
	}

	DWORD dwBufferSize(sizeof(DWORD));
	DWORD nResult(defaultValue);
	LONG nError = ::RegQueryValueExW(hKey,
		key.c_str(),
		0,
		NULL,
		reinterpret_cast<LPBYTE>(&nResult),
		&dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		return nResult;
	}
	return nError;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTregistry::ReadValueString(std::string key, std::string defaultValue)
{
	std::wstring wKey = TGMTutil::String2WString(key);	
	std::wstring wstrDefaultValue = TGMTutil::String2WString(defaultValue);
	std::wstring wstrValue = ReadValueWString(wKey, wstrDefaultValue);
	return TGMTutil::WString2String(wstrValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring TGMTregistry::ReadValueWString(std::wstring key, const std::wstring defaultValue)
{
	if (!m_inited)
	{
		PrintError("you must init registry key before use");
		return defaultValue;
	}

	std::wstring value = defaultValue;
	WCHAR szBuffer[1024];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueExW(hKey, key.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		value = szBuffer;
	}
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTregistry::PrintResult(LONG result)
{
	switch (result)
	{
	case 5:
		PrintError("Access denied");
		break;
	default:
		break;
	}
}