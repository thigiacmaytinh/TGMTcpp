#include "TraffSign.h"


TraffSign::~TraffSign()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TraffSign::getResultIdentification() const
{
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TraffSign::GetResultImage() const
{
	return m_matDst;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TraffSign::GetTrafficSignalImage() const
{
	return trafficSignal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TraffSign::GetTrafficSignalYellow() const
{
	return trafficSignalYellow;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TraffSign::Process(cv::Mat _src)
{
	m_matSrc = _src;
	m_matDst = _src;
	cv::Mat hsv;
	std::vector<cv::Rect> rect;
	cv::cvtColor(_src, hsv, CV_BGR2HSV);
	cv::Mat binary;
	cv::Mat element = cv::getStructuringElement(2, cv::Size(3, 3), cv::Point(1, 1));
	cv::inRange(hsv, cv::Scalar(170, 150, 60), cv::Scalar(180, 256, 256), binary);
	cv::dilate(binary, binary, element, cv::Point(-1, -1), 10);

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	if (contours.size() > 0)
	{
		for (int i = 0; i < contours.size(); ++i)
		{
			cv::Rect r = boundingRect(contours.at(i));
			if (r.width > 100 && r.height > 100 & r.width < 450 && r.height < 450 &&
				(double)r.width / double(r.height) < 1.3 && (double)r.width / double(r.height) > 0.7)
			{
				//rectangle(dst, r, Scalar(0,255,0), 1);
				rect.push_back(r);
				drawContours(binary, contours, i, cv::Scalar(255), -1);
			}
		}
	}

	// loai bo bot rect
	if (rect.empty())
		return;
	cv::Rect noRect = cv::Rect(0, 0, 1, 1);
	cv::Mat subImg, subBinary, subR;
	for (int i = 0; i < rect.size(); ++i)
	{
		for (int j = 1; j < rect.size(); ++j)
		{
			if (rect[j].x > rect[i].x && rect[j].y > rect[i].y &&
				rect[j].x + rect[j].width < rect[i].x + rect[i].width &&
				rect[j].y + rect[j].height < rect[i].y + rect[i].height)
				rect[j] = noRect;
		}
	}

	for (int i = 0; i < rect.size(); ++i)
	{
		if (rect[i] != noRect)
		{
			rectangle(m_matDst, rect[i], cv::Scalar(0, 255, 0), 1);
			subR = m_matSrc(rect[i]);
			subBinary = binary(rect[i]);
		}
	}
	if (subR.empty())
		return;

	subR.copyTo(subImg);
	erode(binary, binary, element, cv::Point(-1, -1), 8);
	medianBlur(subBinary, subBinary, 9);
	resize(subBinary, subBinary, subImg.size());
	assert(subImg.size() == subBinary.size());
	for (int i = 0; i < subBinary.rows; ++i)
	{
		for (int j = 0; j < subBinary.cols; ++j)
		{
			if (subBinary.at<uchar>(i, j) == 0)
			{
				subImg.at<cv::Vec3b>(i, j)[0] = 255;
				subImg.at<cv::Vec3b>(i, j)[1] = 255;
				subImg.at<cv::Vec3b>(i, j)[2] = 255;
			}

		}
	}
	trafficSignal = subImg;
	cv::Mat subImg1, hsv1, binary1;
	subImg.copyTo(subImg1);
	cv::cvtColor(subImg1, hsv1, CV_BGR2HSV);
	cv::inRange(hsv1, cv::Scalar(10, 80, 80), cv::Scalar(30, 255, 255), binary1); // filter yellow

	cv::dilate(binary1, binary1, element, cv::Point(-1, -1), 5);
	std::vector<std::vector<cv::Point> > contours1;
	std::vector<cv::Vec4i> hierarchy1;
	cv::findContours(binary1, contours1, hierarchy1, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	if (contours.size() > 0)
	{
		for (int i = 0; i < contours1.size(); ++i)
		{
			drawContours(binary1, contours1, i, cv::Scalar(255), -1);
		}
	}
	cv::erode(binary1, binary1, element, cv::Point(-1, -1), 5);
	cv::medianBlur(binary1, binary1, 9);
	for (int i = 0; i < binary1.rows; ++i)
	{
		for (int j = 0; j < binary1.cols; ++j)
		{
			if (binary1.at<uchar>(i, j) == 0)
			{
				subImg1.at<cv::Vec3b>(i, j)[0] = 255;
				subImg1.at<cv::Vec3b>(i, j)[1] = 255;
				subImg1.at<cv::Vec3b>(i, j)[2] = 255;
			}

		}
	}
	trafficSignalYellow = subImg1;

	//not implemented yet
	//Identification();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//void TraffSign::Identification()
//{
//	cv::Ptr<SURF> surfObj = SURF::create(500);
//	cv::Ptr<SURF> surfData = SURF::create(500);
//	std::vector<cv::KeyPoint> keyObj;
//	cv::Mat descriptorObj;
//	surfObj->detect(trafficSignalYellow, keyObj);
//	surfObj->compute(trafficSignalYellow, keyObj, descriptorObj);
//	if (imgData.empty())
//		return;
//	int idx;
//	int maxFeature = 0;
//	cv::FlannBasedMatcher matcher;
//	for (int i = 0; i < imgData.size(); ++i)
//	{
//		cv::Mat img = imgData[i];
//		
//		std::vector<cv::KeyPoint> keyData;
//		cv::Mat descriptorData;
//		std::vector<std::vector<cv::DMatch > > matches;
//		std::vector<cv::DMatch > good_matches;
//		surfData->detect(img, keyData);
//		surfData->compute(img, keyData, descriptorData);
//
//		matcher.knnMatch(descriptorObj, descriptorData, matches, 2);
//		for (int j = 0; j < cv::min(descriptorObj.rows - 1, (int)matches.size()); j++) //THIS LOOP IS SENSITIVE TO SEGFAULTS
//		{
//			if ((matches[j][0].distance < 0.6*(matches[j][1].distance)) && ((int)matches[j].size() <= 2 && (int)matches[j].size() > 0))
//			{
//				good_matches.push_back(matches[j][0]);
//			}
//		}
//
//		if ((int)good_matches.size() > maxFeature)
//		{
//			maxFeature = good_matches.size();
//			idx = i;
//		}
//
//	}
//	if (maxFeature > 3)
//		result = imgFileName.at(idx);
//	else
//		result = "Khong xac dinh";
//}