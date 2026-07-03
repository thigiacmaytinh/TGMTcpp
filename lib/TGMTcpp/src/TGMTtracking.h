#pragma once
#include <string>
#include <opencv2/tracking.hpp>


#define GetTGMTtracking TGMTtracking::GetInstance

class TGMTtracking
{
	static TGMTtracking* m_instance;

	std::string trackerTypes[8] = { "BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "MOSSE", "CSRT" };
	cv::Ptr<cv::Tracker> tracker;
public:
	static enum TrackerTypes
	{
		BOOSTING = 0,
		MIL,
		KCF,
		TLD,
		MEDIANFLOW,
		GOTURN,
		MOSSE,
		CSRT
	} m_trackerType;

	static TGMTtracking* GetInstance()
	{
		if (!m_instance)
			m_instance = new TGMTtracking();
		return m_instance;
	}
	TGMTtracking();
	~TGMTtracking();

	void Init(TrackerTypes trackerType);
	
	void Update(cv::Mat frame, int frameIndex, cv::Rect& bbox);

};

