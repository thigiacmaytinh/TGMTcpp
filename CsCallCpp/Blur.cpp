//https://thigiacmaytinh.com

#include "Blur.h"


cv::Mat CBlur::ConvertToBlur(cv::Mat matInput, int value)
{
	if (value < 1)
		return matInput;

	if (value % 2 == 0)
		value++; //vohungvi: value must be odd: 1, 3, 5, 7,...

	cv::Mat matResult;
	cv::blur(matInput, matResult, cv::Size(value, value));
	return matResult;
}