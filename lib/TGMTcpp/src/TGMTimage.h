#pragma once
#include "stdafx.h"
#include <filesystem>

//----------------------------------------
// TGMT mean: ThiGiacMayTinh
//----------------------------------------
class TGMTimage
{
public:
	//TGMTimage(void);
	//~TGMTimage(void);
	static cv::Mat ConvertToGray(const cv::Mat matInput);

	static std::string GetImageType(cv::Mat);
	static bool Compare(cv::Mat mat1, cv::Mat mat2);

	//return percent of Blurriness
	//lower is more blurry

	static int GetBitDepth(cv::Mat);
	static int GetNumChannels(cv::Mat matInput);
	static cv::Mat Blend(cv::Mat mat1, cv::Mat mat2, float alpha);
	static cv::Mat SelectRoi(cv::Mat matInput, cv::Rect roi);
	static std::string MatToBase64(cv::Mat matInput);
	static cv::Mat Base64ToMat(std::string base64);

	static cv::Mat imread(std::filesystem::path path, int flags = cv::IMREAD_COLOR);
	static cv::Mat imwrite(std::filesystem::path path, cv::Mat frame);

	static bool IsMatHeaderValid(const cv::Mat& m);

#ifdef HAVE_LIBRAW
	static cv::Mat imreadRaw(std::wstring path);
	static bool isRawImage(std::wstring path);
#endif // HAVE_LIBRAW
};

