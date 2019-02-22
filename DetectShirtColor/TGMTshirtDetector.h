#pragma once
#include "stdafx.h"
#include <string>

#define GetTMGMTshirtDetector TGMTshirtDetector::GetInstance
#define SHOW_DEBUG_IMAGE 1

class TGMTshirtDetector
{
	static TGMTshirtDetector* instance;
		
public:
	TGMTshirtDetector();
	~TGMTshirtDetector();

	static TGMTshirtDetector* GetInstance()
	{
		if (!instance)
			instance = new TGMTshirtDetector();
		return instance;
	}

	std::string DetectShirtColor(char* imgPath);
	std::string DetectShirtColor(cv::Mat imgInput);
	std::string DetectShirtColor(cv::Mat imgInput, std::vector<cv::Rect> rectFaces);
};

