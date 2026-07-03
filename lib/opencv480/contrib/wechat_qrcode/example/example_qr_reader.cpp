////////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other cv::Materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
////////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************************************
USAGE:
./opencv_annotation -images <folder location> -annotations <output file>

Created by: Puttemans Steven - February 2015
Adapted by: Puttemans Steven - April 2016 - Vectorize the process to enable better processing
                                               + early leave and store by pressing an ESC key
                                               + enable delete `d` button, to remove last annotation
*****************************************************************************************************/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include <fstream>
#include <iostream>
#include <map>

using namespace std;


#include <opencv2/wechat_qrcode.hpp>
#include <codecvt>
#include <windows.h>

void PrintUnicode(const wchar_t s[])
{
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
    char* m = new char[bufferSize];
    WideCharToMultiByte(CP_UTF8, 0, s, -1, m, bufferSize, NULL, NULL);
    wprintf(L"%S", m);
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);

    cout << endl << argv[0] << endl << endl;
    cout << "A demo program of WeChat QRCode Detector: " << endl;

    cv::Mat img;
    img = cv::imread("qr/z5768967815624.jpg");
        
    // The model is downloaded to ${CMAKE_BINARY_DIR}/downloads/wechat_qrcode if cmake runs without warnings,
    // otherwise you can download them from https://github.com/WeChatCV/opencv_3rdparty/tree/wechat_qrcode.
    cv::Ptr<cv::wechat_qrcode::WeChatQRCode> detector;

    try {
        detector = cv::makePtr<cv::wechat_qrcode::WeChatQRCode>("qr/detect.prototxt", "qr/detect.caffemodel",
            "qr/sr.prototxt", "qr/sr.caffemodel");
    }
    catch (const std::exception& e) {
        cout <<
            "\n---------------------------------------------------------------\n"
            "Failed to initialize WeChatQRCode.\n"
            "Please, download 'detector.*' and 'sr.*' from\n"
            "https://github.com/WeChatCV/opencv_3rdparty/tree/wechat_qrcode\n"
            "and put them into the current directory.\n"
            "---------------------------------------------------------------\n";
        cout << e.what() << endl;
        return 0;
    }
    string prevstr = "";
    vector<cv::Mat> points;

    auto res = detector->detectAndDecode(img, points);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::vector<std::wstring> wres;

    for (const auto& t : res)
    {
        //cout << t << endl;
        //wres.push_back(converter.from_bytes(t));
        PrintUnicode(converter.from_bytes(t).c_str());

    }
    //for (const auto t : wres) wcout << L"unicode: " << t.c_str() << endl;
    
    return 0;
}
