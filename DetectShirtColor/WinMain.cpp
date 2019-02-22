// DetectShirtColor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "tchar.h"
#include "TGMTshirtDetector.h"

int _tmain(int argc, _TCHAR* argv[])
{
	GetTMGMTshirtDetector()->DetectShirtColor("vohungvi.jpg");
	cv::waitKey(0);
	getchar();
	return 0;
}

