#include "TGMTblob.h"
#include "TGMTdebugger.h"
#include "TGMTimage.h"
#include "TGMTcolor.h"
#include "TGMTtransform.h"

//TGMTblob::TGMTblob(void)
//{
//}
//
//
//TGMTblob::~TGMTblob(void)
//{
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTblob::Demo(std::string imgPath)
{
	cv::Mat matInput = cv::imread(imgPath);
	cv::Mat matBinary = TGMTimage::ConvertToGray(matInput);
	
	cv::adaptiveThreshold(matBinary, matBinary, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 21, 0);

	//cv::threshold(matBinary, matBinary, 100, 255, CV_THRESH_BINARY);


	std::vector<Blob> blobs = FindBlobs(matBinary);

	cv::Mat matOut = cv::Mat::zeros(matInput.size(), CV_8UC3);

	DrawBlobs(matOut, blobs, cv::Point(0, 0));

	cv::Point2f center = TGMTblob::GetCenterPoint(blobs[0]);
	cv::circle(matOut, center, 5, RED, -1, 8, 0);
	DEBUG_IMAGE(matOut, "Blobs detected");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<TGMTblob::Blob> TGMTblob::FindBlobs(const cv::Mat &matBinary, cv::Size minSize, cv::Size maxSize)
{
	ASSERT(matBinary.channels() == 1, "Image is not binary");


	std::vector<Blob> blobs;

	// Fill the label_image with the blobs
	// 0  - background
	// 1  - unlabelled foreground
	// 2+ - labelled foreground

	uchar label_count = 2; // starts at 2 because 0,1 are used already

	for (int y = 0; y < matBinary.rows; y++)
	{
		for (int x = 0; x < matBinary.cols; x++)
		{
			uchar val = matBinary.at<uchar>(y, x);
			if (val != 255)
				continue;

			cv::Rect rect;
			cv::floodFill(matBinary, cv::Point(x, y), ++label_count, &rect);
			if (label_count == 255)
				label_count = 2;

			if (minSize.area() > 0 && (rect.width < minSize.width || rect.height < minSize.height))
				continue;

			if(maxSize.area() >0 && (rect.width > maxSize.width || rect.height > maxSize.height))
				continue;

			Blob blob;

			for (int i = rect.y; i < (rect.y + rect.height); i++)
			{
				for (int j = rect.x; j < (rect.x + rect.width); j++)
				{
					if (matBinary.at<uchar>(i, j) == label_count)
						blob.points.push_back(cv::Point2i(j, i));
				}
			}

			if (blob.points.size() == 0)
				continue;

			blob.boundingRect = rect;
			blob.matContainSize = matBinary.size();
			blobs.push_back(blob);
			
		}
	}

	//DEBUG_OUT("Detected %d blobs", blobs.size());
	return blobs;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTblob::DrawBlobs(cv::Mat& mat, std::vector<Blob> blobs, cv::Point startPoint, cv::Scalar color)
{
	ASSERT(mat.data, "Image input is null");

	// Randomy color the blobs
	for (size_t i = 0; i < blobs.size(); i++)
	{
		cv::Scalar clr = color == UNDEFINED_COLOR ? TGMTcolor::GetRandomColor() : color;

		for (size_t j = 0; j < blobs[i].points.size(); j++)
		{
			int x = blobs[i].points[j].x + startPoint.x;
			int y = blobs[i].points[j].y + startPoint.y;

			if(mat.channels() == 1)
				mat.at<uchar>(y, x) = clr[0];
			else
				mat.at<cv::Vec3b>(y, x) = cv::Vec3b(clr[0], clr[1], clr[2]);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTblob::DrawBlob(cv::Mat& mat, Blob blob, cv::Point startPoint, cv::Scalar color)
{
	ASSERT(mat.data, "Image input is null");

	cv::Scalar clr = color == UNDEFINED_COLOR ? TGMTcolor::GetRandomColor() : color;

	for (size_t i = 0; i < blob.points.size(); i++)
	{
		int x = blob.points[i].x + startPoint.x;
		int y = blob.points[i].y + startPoint.y;

		
		if (mat.channels() == 1)
			mat.at<uchar>(y, x) = clr[0];
		else
			mat.at<cv::Vec3b>(y, x) = cv::Vec3b(clr[0], clr[1], clr[2]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTblob::Blob TGMTblob::FindBiggestBlob(std::vector<Blob> blobs)
{
	int size = blobs[0].points.size();
	int index = 0;
	for (uint i = 1; i < blobs.size(); i++)
	{
		if (blobs[i].points.size() > size)
		{
			size = blobs[i].points.size();
			index = i;
		}
	}
	return blobs[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTblob::DrawBlobAndTrim(Blob blob, cv::Scalar color)
{
	cv::Mat mat = cv::Mat::zeros(blob.boundingRect.size(), CV_8UC3);
	DrawBlob(mat, blob, cv::Point(-blob.boundingRect.x, -blob.boundingRect.y), color);
	
	return mat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTblob::Blob TGMTblob::Normalize(Blob& blob)
{
	int x = blob.boundingRect.x;
	int y = blob.boundingRect.y;
	blob.boundingRect.x = 0;
	blob.boundingRect.y = 0;

	for (int i = 0; i < blob.points.size(); i++)
	{
		blob.points[i] = cv::Point(blob.points[i].x - x, blob.points[i].y - y);
	}
	return blob;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<cv::Point> TGMTblob::MergeBlobs(std::vector<TGMTblob::Blob> blobs)
{
	std::vector<cv::Point> points;
	for (uint i = 0; i < blobs.size(); i++)
	{
		for (uint j = 0; j < blobs[i].points.size(); j++)
		{
			points.push_back(blobs[i].points[j]);
		}
	}

	return points;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTblob::Blob TGMTblob::GetBlobBoundary(cv::Mat matInput, TGMTblob::Blob blob)
{
	cv::Rect rect = blob.boundingRect;
	cv::Mat mat(cv::Size(rect.x + rect.width, rect.y + rect.height), CV_8U);
		
	DrawBlob(mat, blob);

	cv::Mat kernel = cv::Mat::ones(3, 3, CV_8U);
	cv::Mat matEroded;
	cv::erode(mat, matEroded, kernel);
	cv::Mat matDst = mat - matEroded;

	Blob result;
	cv::Rect r(mat.cols /2, mat.rows/2, 0, 0);

	for (int y = 0; y < matDst.rows; y++)
	{
		for (int x = 0; x < matDst.cols; x++)
		{
			if (matDst.at<uchar>(y, x) != 0)
			{
				result.points.push_back(cv::Point(x, y));
				if (r.x > x) r.x = x - 1;
				if (r.y > y) r.y = y - 1;
				if (r.x + r.width < x ) r.width = x - r.x + 2;
				if (r.y + r.height < y ) r.height = y - r.y + 2;
			}
		}
	}

	result.boundingRect = r;
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Point2f TGMTblob::GetCenterPoint(Blob blob)
{
	cv::Moments oMoments = cv::moments(blob.ToBinaryMat());

	double area = oMoments.m00;

	float x = oMoments.m10 / area;
	float y = oMoments.m01 / area;

	x += blob.boundingRect.x;
	y += blob.boundingRect.y;

	return cv::Point2f(x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTblob::DrawBoundingRects(cv::Mat matInput, std::vector<Blob> blobs, cv::Point startPoint, cv::Scalar color, int thickness)
{
	if (blobs.size() == 0)
		return matInput;

	cv::Mat matDraw = matInput.clone();
	for (size_t i=0; i<blobs.size(); i++)
	{
		cv::Rect r = blobs[i].boundingRect;
		r.x += startPoint.x;
		r.y += startPoint.y;
		cv::rectangle(matDraw, r, color == UNDEFINED_COLOR ? TGMTcolor::GetRandomColor() : color, thickness);
	}
	return matDraw;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTblob::Blob::ToBinaryMat()
{
	cv::Mat matDraw = cv::Mat(matContainSize, CV_8UC1, BLACK);

	for (size_t i = 0; i < points.size(); i++)
	{
		int x = points[i].x - boundingRect.x;
		int y = points[i].y - boundingRect.y;

		matDraw.at<uchar>(y, x) = 255;
	}
	
	return matDraw;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::RotatedRect TGMTblob::Blob::GetRotatedRect()
{
	cv::RotatedRect rrect = cv::minAreaRect(points);
	return rrect;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTblob::Blob::Blob(Blob* b)
{
	boundingRect = b->boundingRect;
	matContainSize = b->matContainSize;
	points = b->points;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTblob::Blob TGMTblob::Rotate(TGMTblob::Blob blob, int angle)
{
	cv::Mat matBlob = blob.ToBinaryMat();
	cv::Mat matRotate = TGMTtransform::Rotate(matBlob, angle);
	cv::threshold(matRotate, matRotate, 0, 255, CV_THRESH_BINARY);

	//DEBUG_IMAGE(matRotate, "mat blob");
	//cv::waitKey();

	blob.points.clear();
	int minX = matRotate.cols - 1;
	int minY = matRotate.rows - 1;
	int maxX = 0;
	int maxY = 0;
	for (int i = 0; i < matRotate.rows; i++)
	{
		for (int j = 0; j < matRotate.cols; j++)
		{
			uchar a = matRotate.at<uchar>(i, j);
			if (matRotate.at<uchar>(i, j) != 0)
			{
				blob.points.push_back(cv::Point2i(j, i));
				if (minX > j) minX = j;
				if (minY > i) minY = i;
				if (maxX < j) maxX = j;
				if (maxY < i) maxY = i;
			}
		}
	}

	blob.boundingRect.x = minX;
	blob.boundingRect.y = minY;
	blob.boundingRect.width = maxX - minX + 1;
	blob.boundingRect.height = maxY - minY + 1;
	return blob;
}