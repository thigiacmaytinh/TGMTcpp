#pragma once
#include "stdafx.h"
#include "yolo_v2_class.hpp"

#define GetTGMTdarknetCpp TGMTdarknetCpp::GetInstance

class TGMTdarknetCpp
{
	static TGMTdarknetCpp* m_instance;
	

#if CV_MAJOR_VERSION == 3

#else

#endif
	bool m_isTrained = false;
	bool m_isAutoTrain = false;
	Detector* detector;
	std::vector<std::string> obj_names;
	
public:
	TGMTdarknetCpp();
	~TGMTdarknetCpp();
	
	static TGMTdarknetCpp* GetInstance()
	{
		if (!m_instance)
			m_instance = new TGMTdarknetCpp();
		return m_instance;
	}

	struct DarknetObj
	{
		cv::Rect rect;
		int classID;
		std::string className;
		float probability;
	};

	//using TGMTml::TrainData;
	//bool TrainData(cv::Mat matData, cv::Mat matLabel) override;

	//float Predict(cv::Mat matInput) override;
	bool IsLoaded() { return m_isTrained; }
	std::vector<TGMTdarknetCpp::DarknetObj> Detect(std::string filename, float probability);
	std::vector<TGMTdarknetCpp::DarknetObj> Detect(cv::Mat matInput, float probability);


	bool LoadModel(std::string classessFile, std::string cfgFile, std::string weightFile);
	void show_console_result(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names, int frame_id = -1);
	std::vector<DarknetObj> ConvertResult(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names, float probability);
};

