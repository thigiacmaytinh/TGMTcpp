#include "stdafx.h"
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#include <time.h>
#endif

#ifdef OS_LINUX
#include <stdarg.h>
#endif

#ifdef _MANAGED
#else
#include <thread>
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnMouseEvent(int event, int x, int y, int flags, void* userdata)
{
	switch (event)
	{
	case cv::EVENT_MOUSEMOVE:
		break;
	case cv::EVENT_LBUTTONDOWN:
		//print coordinate
		std::cout << "Left mouse click [" << x << ";" << y << "]\n";
		break;
	case cv::EVENT_RBUTTONDOWN:
		break;
	case cv::EVENT_MBUTTONDOWN:
		break;
	case cv::EVENT_LBUTTONUP:
		break;
	case cv::EVENT_RBUTTONUP:
		break;
	case cv::EVENT_MBUTTONUP:
		break;
	case cv::EVENT_LBUTTONDBLCLK:
		break;
	case cv::EVENT_RBUTTONDBLCLK:
		break;
	case cv::EVENT_MBUTTONDBLCLK:
		break; 
	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MANAGED
void ShowImage(cv::Mat img, std::string str)
#else
void ShowImage(cv::Mat img, const char* fmt, ...)
#endif
{
#ifndef _MANAGED
	va_list arg_list;
	char str[DEBUG_OUT_BUFFER_SIZE];
	va_start(arg_list, fmt);
	vsnprintf(str, DEBUG_OUT_BUFFER_SIZE - 1, fmt, arg_list);
#endif
	cv::imshow(str, img);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MANAGED
void DebugImage(cv::Mat img, std::string str)
#else
void DebugImage(cv::Mat img, const char* fmt, ...)
#endif
{
#ifndef _MANAGED
	va_list arg_list;
	char str[DEBUG_OUT_BUFFER_SIZE];
	va_start(arg_list, fmt);
	vsnprintf(str, DEBUG_OUT_BUFFER_SIZE - 1, fmt, arg_list);
#endif

	if (img.cols < 100 && img.rows < 100)
	{
		float ratio = 100.f / img.cols;
		cv::Mat matTemp;
		cv::resize(img, matTemp, cv::Size(img.cols * ratio, img.rows * ratio));
		cv::imshow(str, matTemp);
	}
	else
	{
		cv::imshow(str, img);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MANAGED
void WriteImageAsync(cv::Mat img, const char* fmt, ...)
{
	va_list arg_list;
	char str[DEBUG_OUT_BUFFER_SIZE];
	va_start(arg_list, fmt);
	vsnprintf(str, DEBUG_OUT_BUFFER_SIZE - 1, fmt, arg_list);

	std::thread([](char* chr, cv::Mat mat) 
	{
		cv::imwrite(chr, mat);
	}, str, img)
	.detach();
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteImage(cv::Mat img, const char* fmt, ...)
{
	va_list arg_list;
	char str[DEBUG_OUT_BUFFER_SIZE];
	va_start(arg_list, fmt);
	vsnprintf(str, DEBUG_OUT_BUFFER_SIZE - 1, fmt, arg_list);
	cv::imwrite(str, img);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintFPS(cv::Mat& img)
{
#ifdef _MANAGED

#else
	static time_t lastTime = clock() - 1;
	time_t delta = clock() - lastTime;
	time_t fps = 1000 / (delta == 0 ? 1 : delta);
	char str[10];
	sprintf(str, "FPS: %lld", fps);
	cv::putText(img, str, cv::Point(10, 30), cv::FONT_ITALIC, 1, cv::Scalar(0, 0, 255), 1);

	lastTime = clock();
#endif
}