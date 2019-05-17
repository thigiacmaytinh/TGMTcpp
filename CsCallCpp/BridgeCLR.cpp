//https://thigiacmaytinh.com
#include "BridgeCLR.h"
#include "Blur.h"
#include "TGMTbridge.h"

using namespace TGMT;

Bitmap^ BridgeCLR:: ConvertToBlur(Bitmap^ bmp, int value)
{
	cv::Mat mat = TGMTbridge::BitmapToMat(bmp);
	mat = CBlur::ConvertToBlur(mat, value);
	return TGMTbridge::MatToBitmap(mat);
}
