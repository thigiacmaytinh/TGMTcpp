#pragma once
#include "stdafx.h"
#include <opencv2/video.hpp>


#define GetTGMTbgs TGMTbgs::GetInstance

#ifdef _MANAGED
using namespace System::Drawing;

namespace TGMT
{
	public ref class TGMTbgsBridge
	{
	public:
		void Init(int type);
		void Process(Bitmap^ bmp);
		//void Process(System::String^ videoPath);
	};
}
#endif

class TGMTbgs
{
	static TGMTbgs* instance;
	int m_blurSize = 0;
	int m_minWidth = 0;
	int m_minHeight = 0;
	int m_minPoints = 0;
	int m_maxPoints = 0;
	int m_dilate = 0;
	int m_erode = 0;

	cv::Ptr<cv::BackgroundSubtractor> m_pMOG2;

	bool m_debug = false;
public:

	TGMTbgs();
	~TGMTbgs();

	static TGMTbgs* GetInstance()
	{
		if (!instance)
			instance = new TGMTbgs();
		return instance;
	}

	cv::Mat Process(cv::Mat matInput);
	bool LoadConfig();
};
