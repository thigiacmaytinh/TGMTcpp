#include "TGMTjson.h"


///////////////////////////////////////////////////////////////////////////////////////////////////

TGMTjson::TGMTjson(std::string jsonText)
{	
	Json::Reader reader;
	m_parseSuccess = reader.parse(jsonText, m_obj);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TGMTjson::~TGMTjson()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTjson::ReadValueString(std::string key, std::string defaultValue)
{
	if (!m_parseSuccess || m_obj[key].isNull())
		return defaultValue;

	const Json::Value obj = m_obj[key];
	
	return obj.asString();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTjson::ReadValueInt(std::string key, int defaultValue)
{
	if (!m_parseSuccess || m_obj[key].isNull())
		return defaultValue;

	
	const Json::Value obj = m_obj[key];
	return obj.asInt();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTjson::ReadValueBool(std::string key, bool defaultValue)
{
	if (!m_parseSuccess || m_obj[key].isNull())
		return defaultValue;

	const Json::Value obj = m_obj[key];
	return obj.asBool();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

double TGMTjson::ReadValueDouble(std::string key, double defaultValue)
{
	if (!m_parseSuccess || m_obj[key].isNull())
		return defaultValue;

	const Json::Value obj = m_obj[key];
	return obj.asDouble();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTjson::JsonToString(Json::Value obj)
{
	Json::FastWriter fastWriter;
	return fastWriter.write(obj);
}