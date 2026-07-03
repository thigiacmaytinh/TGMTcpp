#include "TGMTobjDetect.h"
#include "TGMTfile.h"
#include "TGMTdebugger.h"
#include "TGMTimage.h"
#include "TGMTtransform.h"

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/features2d.hpp"

#ifdef HAVE_OPENCV_XFEATURES2D
#include "opencv2/xfeatures2d.hpp"
using namespace cv::xfeatures2d;
#endif
TGMTobjDetect* TGMTobjDetect::m_instance = nullptr;


////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTobjDetect::TGMTobjDetect()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTobjDetect::~TGMTobjDetect()
{
	m_cascade.~CascadeClassifier();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Mat2BinaryString(cv::Mat frame)
{
	cv::Mat frameGray, frameBW;
	//convert to gray
	cvtColor(frame, frameGray, cv::COLOR_BGR2GRAY);
	adaptiveThreshold(frameGray, frameBW, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 3, 0);
	std::string result;
	for (int row = 0; row < frameBW.rows; ++row)
	{
		for (int col = 0; col < frameBW.cols; ++col)
		{
			result += frameBW.at<char>(row, col) + 1; //+1 because values in [-1;0]
		}
	}
	return result;

}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<cv::Rect> TGMTobjDetect::Detect(cv::Mat matInput, float scaleFactor, int minNeighbors, cv::Size minSize, cv::Size maxSize)
{	
	
	std::vector<cv::Rect> result;
	if (!matInput.data)
	{
		PrintError("Load image failed");
		return result;
	}

	cv::Mat matGray = TGMTimage::ConvertToGray(matInput.clone());	

	// Enhance / Normalise the image contrast (optional)
	//cv::equalizeHist(detectImg, detectImg);
	
	m_cascade.detectMultiScale(matGray, result, scaleFactor, minNeighbors, 0, minSize, maxSize);


	return 	result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<cv::Rect> TGMTobjDetect::Detect(std::string filePath, float scaleFactor, int minNeighbors, cv::Size minSize, cv::Size maxSize)
{
	return Detect(cv::imread(filePath), scaleFactor, minNeighbors);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTobjDetect::Init(std::string cascadeFile)
{
	m_cascade = cv::CascadeClassifier(cascadeFile);
	ASSERT(!m_cascade.empty(), "Cascade file is empty");

	return !m_cascade.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<cv::Rect> TGMTobjDetect::FindTemplateMatching(cv::Mat matToFind, cv::Mat matTemplate, int method)
{
	//ASSERT(method != CV_TM_CCORR && method != CV_TM_CCOEFF, "Method CV_TM_CCORR & CV_TM_CCOEFF not precision");

	std::vector<cv::Rect> rects;
	if (matTemplate.rows > matToFind.rows || matTemplate.cols > matToFind.cols)
	{
		PrintError("Mat template must be smaller than matInput");
		return rects;
	}

	cv::Mat matDisplay, result;
	matToFind.copyTo(matDisplay);
	int result_cols = matToFind.cols - matTemplate.cols + 1;
	int result_rows = matToFind.rows - matTemplate.rows + 1;
	result.create(result_rows, result_cols, CV_32FC1);
	bool method_accepts_mask = (cv::TM_SQDIFF == method || method == cv::TM_CCORR_NORMED);

	bool use_mask = false; //not implemented yet
	if (use_mask && method_accepts_mask)
	{
		cv::Mat mask;
		cv::matchTemplate(matToFind, matTemplate, result, method, mask);
	}
	else
	{
		cv::matchTemplate(matToFind, matTemplate, result, method);
	}
	
	cv::threshold(result, result, 0.3, 1.0, cv::THRESH_TOZERO);
	double minval, maxval, threshold = 0.89;
	cv::Point minloc, maxloc;

	while (true)
	{			
		cv::minMaxLoc(result, &minval, &maxval, &minloc, &maxloc);
		std::cout << maxval <<"\n";
		if (maxval > threshold)
		{
			rects.push_back(cv::Rect(maxloc, cv::Point(maxloc.x + matTemplate.cols, maxloc.y + matTemplate.rows)));
			cv::floodFill(result, maxloc, cv::Scalar(0), 0, cv::Scalar(0.1), cv::Scalar(1.0));
		}
		else
			break;
	}
	return rects;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_OPENCV_XFEATURES2D
void TGMTobjDetect::SURFmatching(cv::Mat matScene, cv::Mat matObject)
{

	//-- Step 1: Detect the keypoints and extract descriptors using SURF
	int minHessian = 400;
	cv::Ptr<SURF> detector = SURF::create(minHessian);
	std::vector<cv::KeyPoint> keypoints_object, keypoints_scene;
	cv::Mat descriptors_object, descriptors_scene;
	detector->detectAndCompute(matObject, cv::Mat(), keypoints_object, descriptors_object);
	detector->detectAndCompute(matScene, cv::Mat(), keypoints_scene, descriptors_scene);
	//-- Step 2: cv::Matching descriptor vectors using FLANN cv::Matcher
	cv::FlannBasedMatcher Matcher;
	std::vector< cv::DMatch > Matches;
	Matcher.match(descriptors_object, descriptors_scene, Matches);
	double max_dist = 0; double min_dist = 100;
	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors_object.rows; i++)
	{
		double dist = Matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}
	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);
	//-- Draw only "good" cv::Matches (i.e. whose distance is less than 3*min_dist )
	std::vector< cv::DMatch > good_Matches;
	for (int i = 0; i < descriptors_object.rows; i++)
	{
		if (Matches[i].distance < 3 * min_dist)
		{
			good_Matches.push_back(Matches[i]);
		}
	}
	cv::Mat img_Matches;
	drawMatches(matObject, keypoints_object, matScene, keypoints_scene,
		good_Matches, img_Matches, cv::Scalar::all(-1), cv::Scalar::all(-1),
		std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	//-- Localize the object
	std::vector<cv::Point2f> obj;
	std::vector<cv::Point2f> scene;
	for (size_t i = 0; i < good_Matches.size(); i++)
	{
		//-- Get the keypoints from the good cv::Matches
		obj.push_back(keypoints_object[good_Matches[i].queryIdx].pt);
		scene.push_back(keypoints_scene[good_Matches[i].trainIdx].pt);
	}
	cv::Mat H = cv::findHomography(obj, scene, cv::RANSAC);
	//-- Get the corners from the image_1 ( the object to be "detected" )
	std::vector<cv::Point2f> obj_corners(4);
	obj_corners[0] = cvPoint(0, 0); obj_corners[1] = cvPoint(matObject.cols, 0);
	obj_corners[2] = cvPoint(matObject.cols, matObject.rows); obj_corners[3] = cvPoint(0, matObject.rows);
	std::vector<cv::Point2f> scene_corners(4);
	perspectiveTransform(obj_corners, scene_corners, H);
	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	line(img_Matches, scene_corners[0] + cv::Point2f(matObject.cols, 0), scene_corners[1] + cv::Point2f(matObject.cols, 0), cv::Scalar(0, 255, 0), 4);
	line(img_Matches, scene_corners[1] + cv::Point2f(matObject.cols, 0), scene_corners[2] + cv::Point2f(matObject.cols, 0), cv::Scalar(0, 255, 0), 4);
	line(img_Matches, scene_corners[2] + cv::Point2f(matObject.cols, 0), scene_corners[3] + cv::Point2f(matObject.cols, 0), cv::Scalar(0, 255, 0), 4);
	line(img_Matches, scene_corners[3] + cv::Point2f(matObject.cols, 0), scene_corners[0] + cv::Point2f(matObject.cols, 0), cv::Scalar(0, 255, 0), 4);
	//-- Show detected cv::Matches

	img_Matches = TGMTtransform::ResizeByHeight(img_Matches, 600);
	imshow("Good cv::Matches & Object detection", img_Matches);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MANAGED
#include "TGMTbridge.h"
using namespace System;
using namespace System::Drawing;
using namespace TGMT;

std::vector<cv::Rect> TGMTobjDetect::Detect(System::String^ filePath, float scaleFactor, int minNeighbors, cv::Size minSize, cv::Size maxSize)
{
	std::string sFilePath = TGMT::TGMTbridge::SystemStr2stdStr(filePath);
	std::vector<cv::Rect> rects = Detect(sFilePath, scaleFactor, minNeighbors, minSize, maxSize);
	return rects;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTobjDetect::Init(System::String^ cascadeFilePath)
{
	return Init(TGMT::TGMTbridge::SystemStr2stdStr(cascadeFilePath));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTobjDetect::Inited()
{
	return !m_cascade.empty();
}
#endif