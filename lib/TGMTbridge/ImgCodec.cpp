#include "ImgCodec.h"
#include "stdafx.h"
#include "TGMTbridge.h"
#include <TGMTimage.h>

using namespace OpenCV;

Bitmap^ ImgCodec::imread(System::String^ filePath)
{
	cv::Mat mat = TGMTimage::imread(Str2wstr(filePath));
	Bitmap^ bmp = TGMTbridge::MatToBitmap(mat);
	return bmp;
}

bool ImgCodec::IsRawImage(System::String^ filePath)
{
	std::wstring wPath = Str2wstr(filePath);
	return TGMTimage::isRawImage(wPath);
}