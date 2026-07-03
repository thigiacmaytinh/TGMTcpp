
#include "TGMTbgs.h"
#include "TGMTutil.h"
#include "TGMTdebugger.h"
#include "TGMTfile.h"
#include "TGMTutil.h"
#include "TGMTvideo.h"
#include "TGMTConfig.h"
#include "TGMTcamera.h"
#include "TGMTbrightness.h"
#include "TGMTmorphology.h"
#include "TGMTdraw.h"
#include "TGMTblob.h"

#ifdef _MANAGED
#include "TGMTbridge.h"

using namespace TGMT;

#endif

TGMTbgs* TGMTbgs::instance = NULL;

#define INI_SECTION "TGMTbgs"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTbgs::TGMTbgs()
{
	m_pMOG2 = cv::createBackgroundSubtractorMOG2();

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTbgs::~TGMTbgs()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTbgs::LoadConfig()
{
	m_minWidth = GetTGMTConfig()->ReadValueInt(INI_SECTION, "min_width");
	m_minHeight = GetTGMTConfig()->ReadValueInt(INI_SECTION, "min_height");
	m_blurSize = GetTGMTConfig()->ReadValueInt(INI_SECTION, "blur_size");
	m_minPoints = GetTGMTConfig()->ReadValueInt(INI_SECTION, "min_points");
	m_maxPoints = GetTGMTConfig()->ReadValueInt(INI_SECTION, "max_points");
	m_debug = GetTGMTConfig()->ReadValueBool(INI_SECTION, "debug");
	m_dilate = GetTGMTConfig()->ReadValueInt(INI_SECTION, "dilate");
	m_erode = GetTGMTConfig()->ReadValueInt(INI_SECTION, "erode");
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTbgs::Process(cv::Mat matInput)
{
	cv::Mat matfgMaskMOG2;
	m_pMOG2->apply(matInput, matfgMaskMOG2);
	if (m_debug)
	{
		ShowImage(matfgMaskMOG2, "after bgs");
	}


	//remove noise
	cv::medianBlur(matfgMaskMOG2, matfgMaskMOG2, 11);


	if (m_blurSize > 0 && m_blurSize % 2 == 1)
	{
		cv::GaussianBlur(matfgMaskMOG2, matfgMaskMOG2, cv::Size(m_blurSize, m_blurSize * 9), 10);
	}

	if (m_debug)
	{
		ShowImage(matfgMaskMOG2, "blurred");
	}


	if (m_erode > 0 && m_erode % 2 == 1)
	{
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(m_erode * 3, m_erode));
		cv::morphologyEx(matfgMaskMOG2, matfgMaskMOG2, cv::MORPH_ERODE, element);
	}

	cv::threshold(matfgMaskMOG2, matfgMaskMOG2, 200, 255, cv::THRESH_BINARY);


	if (m_dilate > 0 && m_dilate % 2 == 1)
	{
		matfgMaskMOG2 = TGMTmorphology::Dilate(matfgMaskMOG2, cv::MORPH_ELLIPSE, m_dilate);
	}

	

	
	auto blobs = TGMTblob::FindBlobs(matfgMaskMOG2.clone(), cv::Size(m_minWidth, m_minHeight));
	cv::cvtColor(matfgMaskMOG2, matfgMaskMOG2, CV_GRAY2BGR);
	for (int i = 0; i < blobs.size(); i++)
	{
		TGMTblob::Blob blob = blobs[i];
		
		if (blob.points.size() < m_minPoints)
			continue;
		if (blob.points.size() > m_maxPoints)
			continue;

		cv::Point2f p = TGMTblob::GetCenterPoint(blob);
		cv::circle(matfgMaskMOG2, p, 10, RED, -1, 8, 0);
		cv::circle(matInput, p, 10, RED, -1, 8, 0);

		TGMTdraw::PutText(matfgMaskMOG2, blob.boundingRect.tl(), BLUE, "%d", blob.points.size());
	}

	return matfgMaskMOG2;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MANAGED

void TGMTbgsBridge::Process(Bitmap^ bmp)
{
	cv::Mat mat = TGMTbridge::BitmapToMat(bmp);
	GetTGMTbgs()->Process(mat);
	cv::waitKey(1);
}

#endif