#include "TGMTtracking.h"
#include <curl/curl.h>
#include "TGMTonline.h"
#include "TGMTConfig.h"
#include "TGMTdebugger.h"
#include "TGMTfile.h"
#include "TGMThardware.h"


#include <thread>
#ifdef _MANAGED
#else
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")
#endif
TGMTtracking* TGMTtracking::m_instance = nullptr;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTtracking::TGMTtracking()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTtracking::~TGMTtracking()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTtracking::Init(TrackerTypes trackerType)
{
	m_trackerType = trackerType;

	if (trackerType == TrackerTypes::BOOSTING)
		tracker = cv::TrackerBoosting::create();
	if (trackerType == TrackerTypes::MIL)
		tracker = cv::TrackerMIL::create();
	if (trackerType == TrackerTypes::KCF)
		tracker = cv::TrackerKCF::create();
	if (trackerType == TrackerTypes::TLD)
		tracker = cv::TrackerTLD::create();
	if (trackerType == TrackerTypes::MEDIANFLOW)
		tracker = cv::TrackerMedianFlow::create();
	if (trackerType == TrackerTypes::GOTURN)
		tracker = cv::TrackerGOTURN::create();
	if (trackerType == TrackerTypes::MOSSE)
		tracker = cv::TrackerMOSSE::create();
	if (trackerType == TrackerTypes::CSRT)
		tracker = cv::TrackerCSRT::create();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTtracking::Update(cv::Mat frame, int frameIndex, cv::Rect& bbox)
{
	cv::Rect2d box;
	bool ok = tracker->update(frame, box);
}