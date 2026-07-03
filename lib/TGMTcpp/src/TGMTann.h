#ifdef HAVE_OPENCV_ML
#pragma once
#include "stdafx.h"
#include <functional>
#include <set>

#define GetTGMTann TGMTann::GetInstance

using namespace cv::ml;
typedef std::vector<std::string>::const_iterator vec_iter;

class TGMTann
{
	static TGMTann* m_instance;
	cv::Ptr<ANN_MLP> mlp;
	bool m_isTrainSuccess = false;
	std::set<std::string> m_setClass;
	cv::Mat m_vocabulary;
	cv::FlannBasedMatcher m_flann;
	int m_networkInputSize;
	cv::Ptr<cv::KAZE> kaze;

	std::vector<int> m_layerSizes;

	struct ImageData
	{
		std::string classname;
		cv::Mat bowFeatures;
	};

	void ReadImages(vec_iter begin, vec_iter end, std::function<void(const cv::Mat&)> callback);
	cv::Mat ReadImageFeature(std::string filePath);
	cv::Mat ReadImageFeature(cv::Mat matInput);
	
	cv::Mat GetDescriptors(const cv::Mat& img);

	void PrintConfusionMatrix(const std::vector<int>& testOutputPredicted, const std::vector<int>& testOutputExpected);
	bool TrainNeuralNetwork(const cv::Mat& trainSamples, const cv::Mat& trainResponses);

	int GetClassId(const std::string& classname);
	cv::Mat GetClassMat(const std::string& classname);

	cv::Mat GetBOWFeatures( const cv::Mat& descriptors, int vocabularySize);
public:
	TGMTann();
	~TGMTann();

	static TGMTann* GetInstance()
	{
		if (!m_instance)
			m_instance = new TGMTann();
		return m_instance;
	}

	struct PredictData
	{
		std::string classname;
		float probability;
	};

	void LoadSetting();

	bool TrainNeuralNetwork(const std::vector<std::string> fileList, const std::vector<std::string> classes);
	void TestAccuracy(const std::vector<std::string> fileList, const std::vector<std::string> classesExpected);

	bool SaveModel();
	bool LoadModel();

	//return highest probability class name
	PredictData PredictClassess(std::string filePath, std::vector<PredictData> &probabilities);
	PredictData PredictClassess(cv::Mat matInput, std::vector<PredictData> &probabilities);
};

#endif