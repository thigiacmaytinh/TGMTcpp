// DetectShirtColor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


void DetectLineByHough(cv::Mat matInput)
{
	cv::Mat gray;
	cv::Mat mat = matInput.clone();
	cv::cvtColor(mat, gray, CV_BGR2GRAY);
	cv::blur(gray, gray, cv::Size(3, 3));


	// Tim duong thang
	cv::Mat canny;
	cv::Canny(gray, canny, 50, 200, 3, true);


	ShowImage(canny, "Mat canny to detect line");

	std::vector<cv::Vec2f> lines;

	cv::HoughLines(canny, lines, 1, CV_PI / 180, 185);

	std::cout<< "Detected lines: " << lines.size();
	for (int i = 0; i < lines.size(); i++)
	{
		cv::Point2d pt1, pt2;
		float rho = lines[i][0], theta = lines[i][1];
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));

		cv::line(mat, pt1, pt2, cv::Scalar(0, 0, 255), 2);
	}
	ShowImage(mat, "Mat draw");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void DetectLineByHoughP(cv::Mat mat)
{
	cv::Mat gray;
	cv::cvtColor(mat, gray, CV_BGR2GRAY);
	cv::blur(gray, gray, cv::Size(3, 3));


	// Tim duong thang
	cv::Mat canny;
	cv::Canny(gray, canny, 50, 200, 3, true);


	ShowImage(canny, "Mat canny to detect line");

	std::vector<cv::Vec4i> lines;

	int threshold = 20;
	int minLength = 300;
	HoughLinesP(canny, lines, 1, CV_PI / 180, threshold, minLength, 8);
	std::cout << "Detected " << lines.size();

	// Ve duong thang, duong tron len anh
	for (int i = 0; i < lines.size(); i++)
	{
		cv::Vec4i l = lines[i];
		line(mat, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), RED, 2);
	}
	ShowImage(mat, "Mat draw HoughP");
}


///////////////////////////////////////////////////////////////////////////////////////////////////

int main(void) 
{

	std::string imgPath = "duong-bang-san-bay-1.jpg";
	cv::Mat mat = cv::imread(imgPath.c_str());

	DetectLineByHough(mat);
	DetectLineByHoughP(mat);

	cv::waitKey();
	getchar();
}

