#include "TGMTimage.h"
#include "TGMTdebugger.h"

//TGMTimage::TGMTimage(void)
//{
//
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//TGMTimage::~TGMTimage(void)
//{
//
//}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTimage::GetImageType(cv::Mat img)
{
	std::string r = "";

	uchar depth = img.type() & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (img.type() >> CV_CN_SHIFT);

	switch (depth)
	{
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTimage::ConvertToGray(const cv::Mat matInput)
{
	ASSERT(matInput.data, "Mat input is null");

	cv::Mat matGray;
	if (matInput.channels() == 3) 
	{
		cv::cvtColor(matInput, matGray, cv::COLOR_BGR2GRAY);
	}
	else if (matInput.channels() == 4) 
	{
		cv::cvtColor(matInput, matGray, cv::COLOR_BGRA2GRAY);
	}
	else
	{
		matGray = matInput;
	}		
	return matGray;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTimage::Compare(cv::Mat mat1, cv::Mat mat2)
{
	if (!mat1.data || !mat2.data)
		return false;

	return (cv::sum(mat1 != mat2) == cv::Scalar(0, 0, 0, 0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTimage::CalcBlurriness(const cv::Mat src)
{
	cv::Mat Gx, Gy;
	Sobel(src, Gx, CV_32F, 1, 0);
	Sobel(src, Gy, CV_32F, 0, 1);
	double normGx = norm(Gx);
	double normGy = norm(Gy);
	double sumSq = normGx * normGx + normGy * normGy;
	return static_cast<int>(sumSq / src.size().area());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTimage::GetBitDepth(cv::Mat matInput)
{
	uchar depth = matInput.depth() & CV_MAT_DEPTH_MASK;

	switch (depth)
	{
	case CV_8U:
	case CV_8S:
		return 8;
	case CV_16U:
	case CV_16S:
		return 16;
	case CV_32S:
	case CV_32F:
		return 32;
	case CV_64F:
		return 64;
	default:
		ASSERT(false, "What happen with this image?");
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTimage::Blend(cv::Mat mat1, cv::Mat mat2, float alpha)
{
	ASSERT(mat1.size() == mat2.size(), "2 mats must same size");
	cv::Mat dst = cv::Mat(mat1.size(), CV_8UC3);
	float beta = 1.f - alpha;
	cv::addWeighted(mat1, alpha, mat2, beta, 0.0, dst);
	return dst;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTimage::SelectRoi(cv::Mat matInput, cv::Rect roi)
{
	cv::Mat mat = cv::Mat::zeros(matInput.size(), matInput.type());
	matInput(roi).copyTo(mat(roi));
	return mat;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTimage::CalcBlurry(cv::Mat matInput)
{
	cv::Mat gray = TGMTimage::ConvertToGray(matInput);
	cv::Mat laplacianImage;
	cv::Laplacian(gray, laplacianImage, CV_64F);
	cv::Scalar mean, stddev; // 0:1st channel, 1:2nd channel and 2:3rd channel
	cv::meanStdDev(laplacianImage, mean, stddev, cv::Mat());
	double variance = stddev.val[0] * stddev.val[0];

	return variance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTimage::IsBlurryImage(cv::Mat matInput, int thresh)
{
	int variance = CalcBlurry(matInput);
	if (variance < thresh)
	{
		PrintMessageYellow("Blurry factor: %d", variance);
		return true;
	}
	return false;
}