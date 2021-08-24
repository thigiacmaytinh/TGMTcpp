#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include <vector>
#include <string>

using namespace cv::xfeatures2d;

class TraffSign
{
	cv::Mat m_matSrc;
	cv::Mat m_matDst;

	cv::Mat trafficSignal;
	cv::Mat trafficSignalYellow;
	std::vector<cv::Mat> imgData;
	std::vector<std::string> imgFileName;
	std::string result;
	//void Identification();
public:
	TraffSign(){}
	~TraffSign();

	void Process(cv::Mat);

	cv::Mat GetResultImage() const;
	cv::Mat GetTrafficSignalImage() const;
	cv::Mat GetTrafficSignalYellow() const;
	std::string getResultIdentification() const;
};
