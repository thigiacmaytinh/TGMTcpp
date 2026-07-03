// This is the main DLL file.

#include "TGMTbridge.h"
#include <msclr/marshal.h>


using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::Threading;
using namespace System::Runtime::InteropServices;
//using namespace System::Windows::Forms;

typedef unsigned char byte;

Bitmap^ TGMTbridge::MatToBitmap(cv::Mat img)
{
	if (img.data == nullptr)
		return nullptr;

	if (img.type() != CV_8UC3 && img.type() != CV_8UC4)
	{
		std::cout<< "Only images of type CV_8UC3 are supported for conversion to Bitmap";
		return nullptr;
	}

	//create the bitmap and get the pointer to the data
	Bitmap ^bmpimg = gcnew Bitmap(img.cols, img.rows, PixelFormat::Format24bppRgb);

	BitmapData ^data;
	if(img.type() == CV_8UC3)
		data = bmpimg->LockBits(System::Drawing::Rectangle(0, 0, img.cols, img.rows), ImageLockMode::WriteOnly, PixelFormat::Format24bppRgb);
	else if(img.type() == CV_8UC4)
		data = bmpimg->LockBits(System::Drawing::Rectangle(0, 0, img.cols, img.rows), ImageLockMode::WriteOnly, PixelFormat::Format32bppRgb);

	byte *dstData = reinterpret_cast<byte*>(data->Scan0.ToPointer());

	unsigned char *srcData = img.data;

	for (int row = 0; row < data->Height; ++row)
	{
		memcpy(reinterpret_cast<void*>(&dstData[row*data->Stride]), reinterpret_cast<void*>(&srcData[row*img.step]), img.cols*img.channels());
	}

	bmpimg->UnlockBits(data);
	delete(data);
	img.release();
	return bmpimg;	
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTbridge::BitmapToMat(Bitmap^ bitmap)
{
    if (bitmap == nullptr)
        return cv::Mat();

    Drawing::Rectangle rect(0, 0, bitmap->Width, bitmap->Height);

    BitmapData^ bmpData = nullptr;

    try
    {
        bmpData = bitmap->LockBits(
            rect,
            ImageLockMode::ReadOnly,
            bitmap->PixelFormat
        );

        int width = bitmap->Width;
        int height = bitmap->Height;
        int stride = bmpData->Stride;

        uchar* src = static_cast<uchar*>(bmpData->Scan0.ToPointer());

        cv::Mat owned;

        if (bitmap->PixelFormat == PixelFormat::Format24bppRgb)
        {
            owned = cv::Mat(height, width, CV_8UC3);

            int rowBytes = width * 3;

            for (int y = 0; y < height; y++)
            {
                uchar* srcRow = src + y * stride;
                uchar* dstRow = owned.ptr<uchar>(y);
                memcpy(dstRow, srcRow, rowBytes);
            }
        }
        else if (bitmap->PixelFormat == PixelFormat::Format8bppIndexed)
        {
            cv::Mat gray(height, width, CV_8UC1);

            int rowBytes = width;

            for (int y = 0; y < height; y++)
            {
                uchar* srcRow = src + y * stride;
                uchar* dstRow = gray.ptr<uchar>(y);
                memcpy(dstRow, srcRow, rowBytes);
            }

            cv::cvtColor(gray, owned, cv::COLOR_GRAY2BGR);
        }
        else if (bitmap->PixelFormat == PixelFormat::Format32bppArgb ||
            bitmap->PixelFormat == PixelFormat::Format32bppRgb ||
            bitmap->PixelFormat == PixelFormat::Format32bppPArgb)
        {
            cv::Mat bgra(height, width, CV_8UC4);

            int rowBytes = width * 4;

            for (int y = 0; y < height; y++)
            {
                uchar* srcRow = src + y * stride;
                uchar* dstRow = bgra.ptr<uchar>(y);
                memcpy(dstRow, srcRow, rowBytes);
            }

            cv::cvtColor(bgra, owned, cv::COLOR_BGRA2BGR);
        }
        else
        {
            //MessageBox::Show("Does not support input image because don't know format", "TGMT error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            return cv::Mat();
        }

        return owned;
    }
    finally
    {
        if (bmpData != nullptr)
            bitmap->UnlockBits(bmpData);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Point2f TGMTbridge::Point2cvPoint(System::Drawing::Point^ p)
{
	return cv::Point2f(p->X, p->Y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Point TGMTbridge::CvPoint2Point(cv::Point2f p)
{
	return Point(p.x, p.y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Rect TGMTbridge::RectangleToCvRect(Drawing::Rectangle^ rect)
{
	return cv::Rect(rect->X, rect->Y, rect->Width, rect->Height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Drawing::Rectangle TGMTbridge::CvRectToRectangle(cv::Rect rect)
{
	return Drawing::Rectangle(rect.x, rect.y, rect.width, rect.height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//array<System::Drawing::Rectangle^>^ TGMTbridge::CvRectsToRectangles(std::vector<cv::Rect> rects)
//{
//	array<System::Drawing::Rectangle^>^ result = gcnew array<System::Drawing::Rectangle^ >(rects.size());
//	for (int i = 0; i < rects.size(); i++)
//	{
//		result[i] = CvRectToRectangle(rects[i]);
//	}
//	return result;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTbridge::StringToCharPointer(System::String^ string, char* output, int size)
{
	//pin_ptr<const __wchar_t> p = PtrToStringChars(string);
	//wcstombs_s(NULL, output, size, p, size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^ TGMTbridge::stdStrToSystemStr(std::string str)
{
	return gcnew System::String(str.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^ TGMTbridge::stdWstrToSystemStr(std::wstring str)
{
	return gcnew System::String(str.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTbridge::SystemStr2stdStr(System::String^ str)
{

	const char* chars = (const char*)(Marshal::StringToHGlobalAnsi(str)).ToPointer();
	std::string result = chars;
	Marshal::FreeHGlobal(System::IntPtr((void*)chars));
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//return UTF-16 string
std::wstring TGMTbridge::SystemStr2stdWstr(System::String^ str)
{
	pin_ptr<const wchar_t> wch = PtrToStringChars(str);
	return std::wstring(wch, str->Length);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* TGMTbridge::SystemStringToChar(System::String^ str)
{
	return (char*)(void*)Marshal::StringToHGlobalAnsi(str);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTbridge::SystemStringToInt(System::String^ str)
{
	return int::Parse(str);
}