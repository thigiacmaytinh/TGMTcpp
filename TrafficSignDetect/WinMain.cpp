// TGMTtemplate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <tchar.h>
#include "TGMTConfig.h"
#include "TGMTvideo.h"
#include "TGMTdebugger.h"
#include "TraffSign.h"
#include "TGMTfile.h"

int g_totalFrame;
TraffSign tsi;

#define INI_APP_CONFIG "TraffSign"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Process(cv::Mat matInput)
{
	tsi.Process(matInput);
	cv::Mat matResult = tsi.GetResultImage();
	cv::imshow("result", matResult);

	cv::imshow("result1", tsi.GetTrafficSignalImage());
	cv::imshow("result2", tsi.GetTrafficSignalYellow());
	std::cout << tsi.getResultIdentification();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnVideoFrame(cv::Mat frame)
{
	Process(frame);
	cv::waitKey(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	GetTGMTConfig()->LoadSettingFromFile();

	std::string imageFile = GetTGMTConfig()->ReadValueString(INI_APP_CONFIG, "image");
	std::string videoFile = GetTGMTConfig()->ReadValueString(INI_APP_CONFIG, "video");

	

	if (TGMTfile::FileExist(imageFile))
	{
		cv::Mat mat = cv::imread(imageFile);
		Process(mat);
	}
	else if (TGMTfile::FileExist(videoFile))
	{
		int w = GetTGMTConfig()->ReadValueInt(INI_APP_CONFIG, "input_width");
		int h = GetTGMTConfig()->ReadValueInt(INI_APP_CONFIG, "input_height");
		int startIdx = GetTGMTConfig()->ReadValueInt(INI_APP_CONFIG, "start_frame");

		g_totalFrame = GetTGMTvideo()->GetAmountFrame(videoFile);

		GetTGMTvideo()->OnNewFrame = OnVideoFrame;
		GetTGMTvideo()->PlayVideo(videoFile, cv::Size(w, h), 0, startIdx);
	}
	else
	{
		PrintError("Can not load input");
	}
	
	cv::waitKey();
	getchar();
	return 0;
}