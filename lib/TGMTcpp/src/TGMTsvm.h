#pragma once
#include "stdafx.h"
#include "TGMTml.h"


#define GetTGMTsvm TGMTsvm::GetInstance

class TGMTsvm : public TGMTml
{
	static TGMTsvm* m_instance;	
	cv::Ptr<SVM> svm;
	bool m_isInited = false;
	bool m_isAutoTuningParam = true;

	
public:
	TGMTsvm();
	~TGMTsvm();
	
	static TGMTsvm* GetInstance()
	{
		if (!m_instance)
			m_instance = new TGMTsvm();
		return m_instance;
	}
	using TGMTml::TrainData;
	bool TrainData(cv::Mat matData, cv::Mat matLabel) override;

	
	float Predict(std::string imgPath) override;
	float Predict(cv::Mat frame) override;

	void SaveModel(std::string filePath) override;
	bool LoadModel(std::string filePath) override;

	bool IsInited() { return m_isInited; }
};

