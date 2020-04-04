#ifndef _HAND_GESTURE_
#define _HAND_GESTURE_ 

#include "stdafx.h"
#include <vector>
#include <string>

class HandGesture
{
	public:
		HandGesture();
		HandGesture(int cameraId);
		~HandGesture();
		std::vector<std::vector<cv::Point> > contours;
		std::vector<std::vector<int> >hullI;
		std::vector<std::vector<cv::Point> >hullP;
		std::vector<std::vector<cv::Vec4i> > defects;
		std::vector <cv::Point> fingerTips;
		void PrintGestureInfo(cv::Mat src);
		int cIdx;
		int frameNumber = 0;
		int mostFrequentFingerNumber;
		int nrOfDefects;
		cv::Rect bRect;
		double bRect_width;
		double bRect_height;
		bool isHand;
		bool IsHand();
		void InitVectors();
		void GetFingerNumber();
		void EleminateDefects();
		void GetFingerTips();
		void DrawFingerTips();

		cv::Mat srcLR;
		cv::Mat src;
		cv::Mat bw;
		std::vector<cv::Mat> bwList;
		cv::VideoCapture cap;
		void GetImageFromCamera();

	private:
		int prevNrFingerTips;		
		std::vector<int> fingerNumbers;
		std::vector<int> numbers2Display;

		void CheckForOneFinger();
		float getAngle(cv::Point s, cv::Point f, cv::Point e);
		void AddFingerNumberToVector();
		float DistanceP2P(cv::Point a, cv::Point b);
		void ComputeFingerNumber();
		void RemoveRedundantEndPoints(std::vector<cv::Vec4i> newDefects);
		void removeRedundantFingerTips();
};

#endif
