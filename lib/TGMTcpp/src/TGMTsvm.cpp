#include "TGMTsvm.h"
#include "TGMTdebugger.h"
#include "TGMTfile.h"
#if defined(WIN32) || defined(WIN64)
#include "TGMTConfig.h"
#endif


TGMTsvm* TGMTsvm::m_instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTsvm::TGMTsvm()
{
#if defined(WIN32) || defined(WIN64)
	m_isAutoTuningParam = GetTGMTConfig()->ReadBool("SVM", "auto_train", m_isAutoTuningParam);
#endif

	svm = SVM::create();
	svm->setType(SVM::Types::C_SVC);
	svm->setKernel(SVM::KernelTypes::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 300, 1e-6));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTsvm::~TGMTsvm()
{
	svm.release();	
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTsvm::TrainData(cv::Mat matData, cv::Mat matLabel)
{
	if (!matData.data)
	{
		PrintError("Mat data to train is empty");
		return false;
	}
	if (!matLabel.data)
	{
		PrintError("Mat label to train is empty");
		return false;
	}

	SET_CONSOLE_TITLE("Traning data svm...");
	if (m_isAutoTuningParam)
	{
		cv::Ptr<cv::ml::TrainData> trainData = cv::ml::TrainData::create(matData, cv::ml::ROW_SAMPLE, matLabel);
		m_isInited = svm->trainAuto(trainData);
	}
	else
	{
		m_isInited = svm->train(matData, ROW_SAMPLE, matLabel);
	}
	return m_isInited;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float TGMTsvm::Predict(std::string filePath)
{
	cv::Mat mat = cv::imread(filePath);
	return Predict(mat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float TGMTsvm::Predict(cv::Mat frame)
{
	if (!m_isInited)
	{
		PrintError("You must train SVM before use");
		return -1;
	}
	if (frame.empty())
		return -1;

	cv::Mat matFlattened = frame;
	if (frame.rows > 1)
		matFlattened = Flatten(frame);

	return svm->predict(matFlattened);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTsvm::SaveModel(std::string filePath)
{
	svm->save(filePath.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTsvm::LoadModel(std::string filePath)
{
	if (!TGMTfile::FileExist(filePath))
	{
		PrintError("File svm \'%s\' does not exist", filePath.c_str());
		return false;
	}

	svm = StatModel::load<SVM>(filePath);
	
	m_isInited = true;
	return m_isInited;
}
