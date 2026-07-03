#pragma once
#include "stdafx.h"

using namespace System::Drawing;

#define Str2str TGMTbridge::SystemStr2stdStr
#define Str2wstr TGMTbridge::SystemStr2stdWstr
#define str2Str TGMTbridge::stdStrToSystemStr
#define wstr2Str TGMTbridge::stdWstrToSystemStr

#define cvRectToRect TGMTbridge::CvRectToRectangle
#define RectTocvRect TGMTbridge::RectangleToCvRect

ref class TGMTbridge
{
public:
	static Bitmap^ MatToBitmap(const cv::Mat img);
	static cv::Mat BitmapToMat(Bitmap^ bitmap);
	static cv::Point2f Point2cvPoint(Point^ p);
	static Point CvPoint2Point(cv::Point2f p);

	static cv::Rect RectangleToCvRect(System::Drawing::Rectangle^ rect);
	static System::Drawing::Rectangle CvRectToRectangle(cv::Rect rect);

	//static array<System::Drawing::Rectangle^>^ TGMTbridge::CvRectsToRectangles(std::vector<cv::Rect> rects);

	static void StringToCharPointer(System::String^ string, char* output, int size);
	static System::String^ stdStrToSystemStr(std::string str);
	static System::String^ stdWstrToSystemStr(std::wstring str);
	static std::string SystemStr2stdStr(System::String^ str);

	//return UTF-16 string
	static std::wstring SystemStr2stdWstr(System::String^ str);
	static char* SystemStringToChar(System::String^ str);

	static int SystemStringToInt(System::String^ str);
};

