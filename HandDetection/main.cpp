#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "roi.hpp"
#include "handGesture.hpp"
#include <vector>
#include <cmath>
#include "TGMTdraw.h"

#define NSAMPLES 7

int square_len = 20;
int avgColor[NSAMPLES][3];
int c_lower[NSAMPLES][3];
int c_upper[NSAMPLES][3];
std::vector <My_ROI> roi;
HandGesture hg;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WaitForPalmCover()
{
	hg.GetImageFromCamera();
	roi.push_back(My_ROI(cv::Point(hg.src.cols / 3, hg.src.rows / 6), cv::Point(hg.src.cols / 3 + square_len, hg.src.rows / 6 + square_len), hg.src));
	roi.push_back(My_ROI(cv::Point(hg.src.cols / 4, hg.src.rows / 2), cv::Point(hg.src.cols / 4 + square_len, hg.src.rows / 2 + square_len), hg.src));
	roi.push_back(My_ROI(cv::Point(hg.src.cols / 3, hg.src.rows / 1.5), cv::Point(hg.src.cols / 3 + square_len, hg.src.rows / 1.5 + square_len), hg.src));
	roi.push_back(My_ROI(cv::Point(hg.src.cols / 2, hg.src.rows / 2), cv::Point(hg.src.cols / 2 + square_len, hg.src.rows / 2 + square_len), hg.src));
	roi.push_back(My_ROI(cv::Point(hg.src.cols / 2.5, hg.src.rows / 2.5), cv::Point(hg.src.cols / 2.5 + square_len, hg.src.rows / 2.5 + square_len), hg.src));
	roi.push_back(My_ROI(cv::Point(hg.src.cols / 2, hg.src.rows / 1.5), cv::Point(hg.src.cols / 2 + square_len, hg.src.rows / 1.5 + square_len), hg.src));
	roi.push_back(My_ROI(cv::Point(hg.src.cols / 2.5, hg.src.rows / 1.8), cv::Point(hg.src.cols / 2.5 + square_len, hg.src.rows / 1.8 + square_len), hg.src));

	while(cv::waitKey(30) != 13) //Enter key
	{
		hg.GetImageFromCamera();
		for (int j = 0; j < NSAMPLES; j++)
		{
			roi[j].draw_rectangle(hg.src);
		}

		TGMTdraw::PutText(hg.src, cv::Point(hg.src.cols / 2, hg.src.rows / 10), cv::Scalar(200, 0, 0), "Cover rectangles with palm");

		cv::imshow("img1", hg.src);

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetMedian(std::vector<int> val)
{
	int median;
	size_t size = val.size();
	std::sort(val.begin(), val.end());
	if (size % 2 == 0) {
		median = val[size / 2 - 1];
	}
	else {
		median = val[size / 2];
	}
	return median;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GetAvgColor(My_ROI roi, int avg[3])
{
	cv::Mat r;
	roi.roi_ptr.copyTo(r);
	std::vector<int> hm, sm, lm;

	// generate vectors
	for (int i = 2; i < r.rows - 2; i++)
	{
		for (int j = 2; j < r.cols - 2; j++)
		{
			hm.push_back(r.data[r.channels()*(r.cols*i + j) + 0]);
			sm.push_back(r.data[r.channels()*(r.cols*i + j) + 1]);
			lm.push_back(r.data[r.channels()*(r.cols*i + j) + 2]);
		}
	}
	avg[0] = GetMedian(hm);
	avg[1] = GetMedian(sm);
	avg[2] = GetMedian(lm);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Average()
{
	for (int i = 0; i < 10; i++)
	{
		hg.GetImageFromCamera();
		cv::cvtColor(hg.src, hg.src, CV_BGR2HLS);
		for (int j = 0; j < NSAMPLES; j++)
		{
			GetAvgColor(roi[j], avgColor[j]);
			roi[j].draw_rectangle(hg.src);
		}
		cv::cvtColor(hg.src, hg.src, CV_HLS2BGR);

		TGMTdraw::PutText(hg.src, cv::Point(hg.src.cols / 2, hg.src.rows / 10), cv::Scalar(200, 0, 0), "Finding average color of hand");
		cv::imshow("img1", hg.src);
		int key = cv::waitKey(30);
		if (key >= 0 && key != 255)
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InitTrackbars()
{
	for (int i = 0; i < NSAMPLES; i++)
	{
		c_lower[i][0] = 12;
		c_upper[i][0] = 7;
		c_lower[i][1] = 30;
		c_upper[i][1] = 40;
		c_lower[i][2] = 80;
		c_upper[i][2] = 80;
	}
	cv::createTrackbar("lower1", "trackbars", &c_lower[0][0], 255);
	cv::createTrackbar("lower2", "trackbars", &c_lower[0][1], 255);
	cv::createTrackbar("lower3", "trackbars", &c_lower[0][2], 255);
	cv::createTrackbar("upper1", "trackbars", &c_upper[0][0], 255);
	cv::createTrackbar("upper2", "trackbars", &c_upper[0][1], 255);
	cv::createTrackbar("upper3", "trackbars", &c_upper[0][2], 255);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NormalizeColors()
{
	// copy all boundries read from trackbar
	// to all of the different boundries
	for (int i = 1; i < NSAMPLES; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			c_lower[i][j] = c_lower[0][j];
			c_upper[i][j] = c_upper[0][j];
		}
	}
	// normalize all boundries so that 
	// threshold is whithin 0-255
	for (int i = 0; i < NSAMPLES; i++)
	{
		if ((avgColor[i][0] - c_lower[i][0]) < 0)
			c_lower[i][0] = avgColor[i][0];

		if ((avgColor[i][1] - c_lower[i][1]) < 0)
			c_lower[i][1] = avgColor[i][1];

		if ((avgColor[i][2] - c_lower[i][2]) < 0)
			c_lower[i][2] = avgColor[i][2];

		if ((avgColor[i][0] + c_upper[i][0]) > 255)
			c_upper[i][0] = 255 - avgColor[i][0];

		if ((avgColor[i][1] + c_upper[i][1]) > 255)
			c_upper[i][1] = 255 - avgColor[i][1];

		if ((avgColor[i][2] + c_upper[i][2]) > 255)
			c_upper[i][2] = 255 - avgColor[i][2];
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProduceBinaries()
{
	cv::Scalar upperBound, lowerBound;

	for (int i = 0; i < NSAMPLES; i++)
	{
		NormalizeColors();
		lowerBound = cv::Scalar(avgColor[i][0] - c_lower[i][0], avgColor[i][1] - c_lower[i][1], avgColor[i][2] - c_lower[i][2]);
		upperBound = cv::Scalar(avgColor[i][0] + c_upper[i][0], avgColor[i][1] + c_upper[i][1], avgColor[i][2] + c_upper[i][2]);
		hg.bwList.push_back(cv::Mat(hg.srcLR.rows, hg.srcLR.cols, CV_8U));
		inRange(hg.srcLR, lowerBound, upperBound, hg.bwList[i]);
	}
	hg.bwList[0].copyTo(hg.bw);
	for (int i = 1; i < NSAMPLES; i++)
	{
		hg.bw += hg.bwList[i];
	}
	cv::medianBlur(hg.bw, hg.bw, 7);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShowWindows()
{
	cv::pyrDown(hg.bw, hg.bw);
	cv::pyrDown(hg.bw, hg.bw);
	cv::Rect roi(cv::Point(3 * hg.src.cols / 4, 0), hg.bw.size());
	std::vector<cv::Mat> channels;
	cv::Mat result;
	for (int i = 0; i < 3; i++)
	{
		channels.push_back(hg.bw);
	}		
	cv::merge(channels, result);
	result.copyTo(hg.src(roi));
	cv::imshow("img1", hg.src);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FindBiggestContour(std::vector<std::vector<cv::Point> > contours)
{
	int indexOfBiggestContour = -1;
	int sizeOfBiggestContour = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > sizeOfBiggestContour)
		{
			sizeOfBiggestContour = contours[i].size();
			indexOfBiggestContour = i;
		}
	}
	return indexOfBiggestContour;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MakeContours()
{
	cv::Mat aBw;
	cv::pyrUp(hg.bw, hg.bw);
	hg.bw.copyTo(aBw);
	cv::findContours(aBw, hg.contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	hg.InitVectors();
	hg.cIdx = FindBiggestContour(hg.contours);
	if (hg.cIdx != -1)
	{
		//		approxPolyDP( Mat(hg.contours[hg.cIdx]), hg.contours[hg.cIdx], 11, true );
		hg.bRect = cv::boundingRect(cv::Mat(hg.contours[hg.cIdx]));
		cv::convexHull(cv::Mat(hg.contours[hg.cIdx]), hg.hullP[hg.cIdx], false, true);
		cv::convexHull(cv::Mat(hg.contours[hg.cIdx]), hg.hullI[hg.cIdx], false, false);
		approxPolyDP(cv::Mat(hg.hullP[hg.cIdx]), hg.hullP[hg.cIdx], 18, true);

		if (hg.contours[hg.cIdx].size() > 3)
		{
			convexityDefects(hg.contours[hg.cIdx], hg.hullI[hg.cIdx], hg.defects[hg.cIdx]);
			hg.EleminateDefects();
		}

		hg.PrintGestureInfo(hg.src);
		if (hg.IsHand())
		{
			hg.GetFingerTips();
			hg.DrawFingerTips();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	hg = HandGesture(0);
	WaitForPalmCover();
	Average();

	InitTrackbars();

	for (;;)
	{
		//hg.frameNumber++;
		hg.GetImageFromCamera();
		cv::pyrDown(hg.src, hg.srcLR);
		cv::blur(hg.srcLR, hg.srcLR, cv::Size(3, 3));
		cv::cvtColor(hg.srcLR, hg.srcLR, CV_BGR2HLS);
		ProduceBinaries();
		cv::cvtColor(hg.srcLR, hg.srcLR, CV_HLS2BGR);
		MakeContours();
		hg.GetFingerNumber();
		ShowWindows();

		if (cv::waitKey(30) == char('q')) break;
	}

	cv::destroyAllWindows();
	hg.~HandGesture();
	return 0;
}
