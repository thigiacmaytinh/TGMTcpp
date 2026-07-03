#include "TGMTdnn.h"
#include "TGMTdebugger.h"
#include "TGMTfile.h"
#if defined(WIN32) || defined(WIN64)
#include "TGMTConfig.h"
#endif

TGMTdnn* TGMTdnn::m_instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTdnn::TGMTdnn()
{
#if CV_MAJOR_VERSION == 3

	m_isAutoTrain = GetTGMTConfig()->ReadValueBool("SVM", "auto_train");

	svm = SVM::create();
	svm->setType(SVM::C_SVC);
	svm->setKernel(SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 300, 1e-6));

	if (!m_isAutoTrain)
	{		
		
	}
#else
	CvSVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::LINEAR;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);
#endif

	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTdnn::~TGMTdnn()
{
#if CV_MAJOR_VERSION == 3
	svm.release();
#else
	svm.~CvSVM();
#endif
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTdnn::TrainData(cv::Mat matData, cv::Mat matLabel)
{
	ASSERT(matData.data, "Mat data to train is empty");
	ASSERT(matLabel.data, "Mat label to train is empty");

	SET_CONSOLE_TITLE("Traning data svm...");
	if (m_isAutoTrain)
	{
		cv::Ptr<cv::ml::TrainData> trainData = cv::ml::TrainData::create(matData, cv::ml::ROW_SAMPLE, matLabel);
		m_isTrained = svm->trainAuto(trainData);
	}
	else
	{
		m_isTrained = svm->train(matData, ROW_SAMPLE, matLabel);
	}
	return m_isTrained;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float TGMTdnn::Predict(std::string filePath)
{
	cv::Mat mat = cv::imread(filePath, CV_LOAD_IMAGE_GRAYSCALE);
	return Predict(mat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float TGMTdnn::Predict(cv::Mat matInput)
{
	ASSERT(m_isTrained, "You must train SVM before use");

	cv::Mat matData = PrepareMatData(matInput);

#if CV_MAJOR_VERSION == 3
	return svm->predict(matData);
#else
	return svm.predict(matData);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTdnn::SaveModel(std::string filePath)
{
#if CV_MAJOR_VERSION == 3
	svm->save(filePath.c_str());
#else
	svm.save(filePath.c_str());
#endif	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTdnn::LoadModel(std::string filePath)
{
	ASSERT(TGMTfile::FileExist(filePath), "File svm \'%s\' does not exist", filePath.c_str());
#if CV_MAJOR_VERSION == 3
	svm = StatModel::load<SVM>(filePath);
#else
	svm.load(filePath.c_str());
#endif
	
	m_isTrained = true;
	return m_isTrained;
}
