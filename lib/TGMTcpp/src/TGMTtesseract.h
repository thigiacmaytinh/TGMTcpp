#pragma once
#include "stdafx.h"
#include <allheaders.h> // leptonica main header for image io
#include <baseapi.h> // tesseract main header

#define GetTGMTtesseract TGMTtesseract::GetInstance

class TGMTtesseract
{
	static TGMTtesseract* instance;

	std::string m_lang = "vie";
	tesseract::TessBaseAPI m_tesseract;

	std::string ReplaceChar(std::string input);
public:
	TGMTtesseract();
	~TGMTtesseract();

	enum Lang
	{
		Deu,
		Eng,
		Vie
	};


	static TGMTtesseract* GetInstance()
	{
		if (!instance)
			instance = new TGMTtesseract();
		return instance;
	}

	void LoadConfig();
	std::string ReadText(std::string filePath);
	std::string ReadText(cv::Mat matInput);

	void SetLang(Lang lang);
};

