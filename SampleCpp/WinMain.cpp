// Test.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <opencv2/opencv.hpp>

std::string videoPath = "sample.mp4";
int numCol = 2;
int numRow = 2;
int width = 640;
int height = 480;
cv::VideoCapture videoCapture;

cv::Mat GetFrame(int index)
{
	videoCapture.set(CV_CAP_PROP_POS_FRAMES, index);
	cv::Mat frame;
	videoCapture.read(frame);
	return frame;
}

void main(int argc, char** argv)
{
	
	videoCapture = cv::VideoCapture(videoPath);
	if (!videoCapture.isOpened())
	{
		std::cout<< "Can not load video " << videoPath.c_str();
		return;
	}
	int totalVideoFrame = videoCapture.get(CV_CAP_PROP_FRAME_COUNT);

	int numFrame = numCol * numRow;
	int roiWidth = width / numCol;
	int roiHeight = height / numRow;

	cv::Mat matRoi = GetFrame(0);
	cv::resize(matRoi, matRoi, cv::Size(roiWidth, roiHeight));
	cv::Rect roi = cv::Rect(0, 0, roiWidth, roiHeight);

	cv::Mat matThumbnail = cv::Mat::ones(cv::Size(width, height), CV_8UC3);
	matRoi.copyTo(matThumbnail(roi));

	int numPart = numFrame - 1;
	int numFrameAtPart = totalVideoFrame / numPart;

	int i = 0;
	for(int r = 0; r<numRow; r++)
	{
		for (int c = 0; c < numCol; c++)
		{			
			int frameIdx = i * numFrameAtPart - 1;
			if (frameIdx < 0)
				frameIdx == 0;

			matRoi = GetFrame(frameIdx);
		
			roi = cv::Rect(roiWidth * c, roiHeight * r, roiWidth, roiHeight);
			cv::resize(matRoi, matRoi, cv::Size(roiWidth, roiHeight));

			matRoi.copyTo(matThumbnail(roi));
			i++;
		}
	}
	
	cv::imshow("thumbnail", matThumbnail);
	cv::imwrite("thumbnail.jpg", matThumbnail);

	videoCapture.release();
	cv::waitKey();
	getchar();
}