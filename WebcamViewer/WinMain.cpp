// DetectShirtColor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "tchar.h"

void StartCamera(int camID)
{
	cv::VideoCapture cap = cv::VideoCapture(0);
	if (cap.isOpened())  // check if we succeeded
	{
		//set FPS
		cap.set(CV_CAP_PROP_FPS, 25);


		//set size of frame
		cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

		//loop to read
		cv::Mat frame;
		while (true)
		{
			cap >> frame;
			cv::imshow("Web cam", frame);
			cv::waitKey(30);
		}
	}
	else
	{
		std::cout<< "Not found any camera";
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	StartCamera(0);

	cv::waitKey(0);
	getchar();
	return 0;
}

