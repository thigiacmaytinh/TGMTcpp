#include "TGMTdarknetCpp.h"
#include "TGMTdebugger.h"
#include "TGMTfile.h"
//#if defined(WIN32) || defined(WIN64)
//#include "TGMTConfig.h"
//#endif

TGMTdarknetCpp* TGMTdarknetCpp::m_instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTdarknetCpp::TGMTdarknetCpp()
{
#if CV_MAJOR_VERSION == 3

	//m_isAutoTrain = GetTGMTConfig()->ReadValueBool("SVM", "auto_train");

#else
#endif

	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTdarknetCpp::~TGMTdarknetCpp()
{
#if CV_MAJOR_VERSION == 3

#else
#endif
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//bool TGMTdarknetCpp::TrainData(cv::Mat matData, cv::Mat matLabel)
//{
//	ASSERT(matData.data, "Mat data to train is empty");
//	ASSERT(matLabel.data, "Mat label to train is empty");
//
//	SET_CONSOLE_TITLE("Traning data svm...");
//	if (m_isAutoTrain)
//	{
//		cv::Ptr<cv::ml::TrainData> trainData = cv::ml::TrainData::create(matData, cv::ml::ROW_SAMPLE, matLabel);
//		m_isTrained = svm->trainAuto(trainData);
//	}
//	else
//	{
//		m_isTrained = svm->train(matData, ROW_SAMPLE, matLabel);
//	}
//	return m_isTrained;
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//float TGMTdarknetCpp::Predict(std::string filePath)
//{
//	cv::Mat mat = cv::imread(filePath, CV_LOAD_IMAGE_GRAYSCALE);
//	return Predict(mat);
//}
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<TGMTdarknetCpp::DarknetObj> TGMTdarknetCpp::Detect(std::string filename, float probability)
{
	std::vector<TGMTdarknetCpp::DarknetObj> result;
	try {

		image_t img = detector->load_image(filename);
		std::vector<bbox_t> result_vec = detector->detect(img);
		detector->free_image(img);

		result = ConvertResult(result_vec, obj_names, probability);
	}
	catch (std::exception &e) { std::cerr << "exception: " << e.what() << "\n"; getchar(); }
	catch (...) { std::cerr << "unknown exception \n"; getchar(); }
	filename.clear();

	
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<TGMTdarknetCpp::DarknetObj> TGMTdarknetCpp::Detect(cv::Mat matInput, float probability)
{
	//ASSERT(m_isTrained, "You must train SVM before use");
	std::vector<TGMTdarknetCpp::DarknetObj> result;
	try {
		int numChannels = matInput.channels();
		if (numChannels == 4) cv::cvtColor(matInput, matInput, cv::COLOR_RGBA2BGR);
		else if (numChannels == 3) cv::cvtColor(matInput, matInput, cv::COLOR_RGB2BGR);
		else if (numChannels == 1) cv::cvtColor(matInput, matInput, cv::COLOR_GRAY2BGR);

		image_t img = Detector::mat_to_image_custom(matInput);
		std::vector<bbox_t> result_vec = detector->detect(img);
		detector->free_image(img);

		result = ConvertResult(result_vec, obj_names, probability);
	}
	catch (std::exception &e) { std::cerr << "exception: " << e.what() << "\n"; getchar(); }
	catch (...) { std::cerr << "unknown exception \n"; getchar(); }


	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> objects_names_from_file(std::string filename)
{
	std::vector<std::string> file_lines = TGMTfile::ReadAllLine(filename);
	return file_lines;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTdarknetCpp::LoadModel(std::string classessFile, std::string cfgFile, std::string weightFile)
{

#ifdef LIB_CS
	ASSERT(TGMTfile::FileExist(classessFile), "File \'%s\' does not exist", classessFile.c_str());
	ASSERT(TGMTfile::FileExist(cfgFile), "File \'%s\' does not exist", cfgFile.c_str());
	ASSERT(TGMTfile::FileExist(weightFile), "File \'%s\' does not exist", weightFile.c_str());
#endif

	detector = new Detector(cfgFile, weightFile);
	obj_names = objects_names_from_file(classessFile);
	m_isTrained = true;
	return m_isTrained;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTdarknetCpp::show_console_result(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names, int frame_id)
{
	if (frame_id >= 0) std::cout << " Frame: " << frame_id << std::endl;
	for (auto &i : result_vec) 
	{
		if (obj_names.size() > i.obj_id) std::cout << obj_names[i.obj_id] << " - ";
		std::cout << "obj_id = " << i.obj_id << ",  x = " << i.x << ", y = " << i.y
			<< ", w = " << i.w << ", h = " << i.h
			<< std::setprecision(3) << ", prob = " << i.prob << std::endl;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<TGMTdarknetCpp::DarknetObj> TGMTdarknetCpp::ConvertResult(std::vector<bbox_t> const result_vec, std::vector<std::string> const obj_names, float probability)
{
	std::vector<TGMTdarknetCpp::DarknetObj> result;
	for (auto &i : result_vec)
	{		
		if (obj_names.size() > i.obj_id)
		{
			if (i.prob < probability)
				continue;

			TGMTdarknetCpp::DarknetObj obj;
			obj.className = obj_names[i.obj_id];
			obj.rect = cv::Rect(i.x, i.y, i.w, i.h);
			obj.classID = i.obj_id;
			obj.probability = i.prob;
			result.push_back(obj);
		}
	}

	return result;
}
