// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


std::string windowWebcam = "Webcam";
std::string windowResult = "Detected";
cv::Scalar currentColor;
cv::Mat rgbImg;
int iLowH = 0;
int iHighH = 179;

int iLowS = 0;
int iHighS = 255;

int iLowV = 0;
int iHighV = 255;

int iLastX = -1;
int iLastY = -1;

char text[50];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat DetectCircle()
{
	cv::Mat imgHSV;
	cv::cvtColor(rgbImg, imgHSV, cv::COLOR_BGR2HSV);

	cv::Scalar lower = cv::Scalar(iLowH, iLowS, iLowV);
	cv::Scalar higher = cv::Scalar(iHighH, iHighS, iHighV);

	cv::Mat imgColor, imgRemoveNoise;
	cv::inRange(imgHSV, lower, higher, imgColor);
	cv::Mat kernel = cv::Mat::ones(cv::Size(3, 3), CV_8U);
	morphologyEx(imgColor, imgRemoveNoise, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);


	cv::Mat imgResult;
	cv::cvtColor(imgRemoveNoise, imgResult, CV_GRAY2BGR);

	cv::Moments oMoments = cv::moments(imgRemoveNoise);
	double dM01 = oMoments.m01;
	double dM10 = oMoments.m10;
	double dArea = oMoments.m00;

	// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
	if (dArea > 10000)
	{
		//calculate the position of the ball
		int posX = dM10 / dArea;
		int posY = dM01 / dArea;

		if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
		{
			//Draw a red line from the previous point to the current point
			cv::line(imgResult, cv::Point(posX, posY), cv::Point(iLastX, iLastY), cv::Scalar(0, 0, 255), 2);
			sprintf_s(text, "(%d, %d)", posX, posY);
			cv::putText(imgResult, text, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
		}

		iLastX = posX;
		iLastY = posY;
	}


	return imgResult;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TrackingColor()
{
	cv::namedWindow(windowWebcam, 1);
	cvCreateTrackbar("LowH", windowWebcam.c_str(), &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", windowWebcam.c_str(), &iHighH, 179);

	cvCreateTrackbar("LowS", windowWebcam.c_str(), &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", windowWebcam.c_str(), &iHighS, 255);

	cvCreateTrackbar("LowV", windowWebcam.c_str(), &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", windowWebcam.c_str(), &iHighV, 255);

	cv::VideoCapture cap = cv::VideoCapture(0);
	while (true)
	{
		cap.read(rgbImg);
		cv::imshow(windowWebcam, rgbImg);
		cv::imshow(windowResult, DetectCircle());
		cv::waitKey(40);

	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	TrackingColor();

	cv::waitKey();
	getchar();
	return 0;
}