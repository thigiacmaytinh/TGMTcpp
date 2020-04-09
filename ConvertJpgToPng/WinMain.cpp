// Test.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <opencv2/opencv.hpp>


void main(int argc, char** argv)
{
	
	cv::Mat mat = cv::imread("dog.jpg");

	cv::Mat matGRBA;
	cv::cvtColor(mat, matGRBA, CV_BGR2BGRA);

	for (int y = 0; y < mat.rows; y++)
	{
		for (int x = 0; x < mat.cols; x++)
		{
			cv::Vec3b val = mat.at<cv::Vec3b>(y, x);
			if (val[0] > 200 && val[1] > 200 && val[2] > 200)
			{
				matGRBA.at<cv::Vec4b>(y, x) = cv::Vec4b(0, 0, 0, 0);
			}
		}
	}

	cv::imwrite("dog_rgba.png", matGRBA);
	cv::imshow("dog", matGRBA);

	cv::waitKey();
	getchar();
}