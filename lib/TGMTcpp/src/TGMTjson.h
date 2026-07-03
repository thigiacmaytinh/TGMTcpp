#pragma once
#include <string>
#include <vector>
#include "json\json.h"

class TGMTjson
{	
	bool m_parseSuccess = false;
	Json::Value m_obj;	
public:
    TGMTjson(std::string jsonText);
    ~TGMTjson();

	std::string ReadValueString(std::string name, std::string default_value);
	int ReadValueInt( std::string name, int default_value);
	double ReadValueDouble(std::string name, double default_value);
	//valid true values are "true", "yes", "on", "1",
	// and valid false values are "false", "no", "off", "0" (not case sensitive).
	bool ReadValueBool(std::string name, bool default_value);

	static std::string JsonToString(Json::Value obj);
};

