#ifdef HAVE_OPENCV_ML
#include "TGMTann.h"
#include "TGMTdebugger.h"
#include "TGMTfile.h"
#include <fstream>
#include "TGMTConfig.h"
#include "TGMTimage.h"

//const int TGMTann::numCharacters = 30;
TGMTann* TGMTann::m_instance = nullptr;

#define ANN_INI "TGMTann"

////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTann::TGMTann()
{
	kaze = cv::KAZE::create();	
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTann::~TGMTann()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTann::LoadSetting()
{
	m_networkInputSize = GetTGMTConfig()->ReadValueInt(ANN_INI, "network_input_size", 8);

	if (m_networkInputSize < 2)
		m_networkInputSize = 2;

	int w = GetTGMTConfig()->ReadValueInt(ANN_INI, "desire_width");
	int h = GetTGMTConfig()->ReadValueInt(ANN_INI, "desire_height");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTann::ReadImages(vec_iter begin, vec_iter end, std::function<void(const cv::Mat&)> callback)
{
	uint i = 0;
	for (auto it = begin; it != end; ++it)
	{
		std::string filename = *it;
		std::cout << "Reading image " << filename << std::endl;
		cv::Mat img = cv::imread(filename, 0);
		if (img.empty())
		{
			std::cerr << "WARNING: Could not read image." << std::endl;
			continue;
		}

		cv::Mat descriptors = GetDescriptors(img);
		callback(descriptors);

		SET_CONSOLE_TITLE("%d", i++);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTann::ReadImageFeature(std::string filePath)
{
	cv::Mat mat = cv::imread(filePath, CV_LOAD_IMAGE_GRAYSCALE);
	return ReadImageFeature(mat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTann::ReadImageFeature(cv::Mat matInput)
{
	ASSERT(!matInput.empty(), "Can not read image");
	matInput = TGMTimage::ConvertToGray(matInput);

	cv::Mat descriptors = GetDescriptors(matInput);
	cv::Mat bowFeatures = GetBOWFeatures(descriptors, m_networkInputSize);
	cv::normalize(bowFeatures, bowFeatures, 0, bowFeatures.rows, cv::NORM_MINMAX, -1, cv::Mat());

	return bowFeatures;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTann::TrainNeuralNetwork(const std::vector<std::string> files, const std::vector < std::string > classes)
{
	ASSERT(files.size() == classes.size(), "Num of files must same classes");
	ASSERT(m_networkInputSize > 0, "m_networkInputSize must bigger than 2");
	

	cv::Mat descriptorsSet;
	std::vector<ImageData*> descriptorsMetadata;
	m_setClass.clear();

	int idx = 0;
	

	ReadImages(files.begin(), files.end(), [&](const cv::Mat& descriptors)
	{

		// Append to the list of descriptors
		descriptorsSet.push_back(descriptors);
		// Append metadata to each extracted feature
		ImageData* data = new ImageData;
		data->classname = classes[idx++];
		data->bowFeatures = cv::Mat::zeros(cv::Size(m_networkInputSize, 1), CV_32F);
		for (int j = 0; j < descriptors.rows; j++)
		{
			descriptorsMetadata.push_back(data);
		}

		m_setClass.insert(data->classname);
	});


	std::cout << "Creating vocabulary..." << std::endl;

	cv::Mat labels;

	// Use k-means to find k centroids (the words of our vocabulary)
	cv::kmeans(descriptorsSet, m_networkInputSize, labels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 0.01), 1, cv::KMEANS_PP_CENTERS, m_vocabulary);
	// No need to keep it on memory anymore
	descriptorsSet.release();



	// Convert a set of local features for each image in a single descriptors
	// using the bag of words technique
	std::cout << "Getting histograms of visual words..." << std::endl;
	int* ptrLabels = (int*)(labels.data);
	int size = labels.rows * labels.cols;
	for (int i = 0; i < size; i++)
	{
		int label = *ptrLabels++;
		ImageData* data = descriptorsMetadata[i];
		data->bowFeatures.at<float>(label)++;
	}

	// Filling matrixes to be used by the neural network
	std::cout << "Preparing neural network..." << std::endl;
	cv::Mat arrayTrainSamples;
	cv::Mat arrayTrainResponses;
	std::set<ImageData*> uniqueMetadata(descriptorsMetadata.begin(), descriptorsMetadata.end());
	for (auto it = uniqueMetadata.begin(); it != uniqueMetadata.end(); )
	{
		ImageData* data = *it;
		cv::Mat normalizedHist;
		cv::normalize(data->bowFeatures, normalizedHist, 0, data->bowFeatures.rows, cv::NORM_MINMAX, -1, cv::Mat());
		arrayTrainSamples.push_back(normalizedHist);
		arrayTrainResponses.push_back(GetClassMat(data->classname));
		delete *it; // clear memory
		it++;
	}
	descriptorsMetadata.clear();

	// Training neural network
	std::cout << "Training neural network..." << std::endl;
	TrainNeuralNetwork(arrayTrainSamples, arrayTrainResponses);


	// We can clear memory now 
	arrayTrainSamples.release();
	arrayTrainResponses.release();


	// Train FLANN 
	std::cout << "Training FLANN..." << std::endl;


	m_flann.add(m_vocabulary);
	m_flann.train();


	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTann::TrainNeuralNetwork(const cv::Mat& trainSamples, const cv::Mat& trainResponses)
{
	ASSERT(trainSamples.rows == trainResponses.rows, "train set not match with class");

	int networkOutputSize = trainResponses.cols;
	mlp = cv::ml::ANN_MLP::create();
	m_layerSizes = { trainSamples.cols, m_networkInputSize,	networkOutputSize };
	mlp->setLayerSizes(m_layerSizes);
	mlp->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM);
	m_isTrainSuccess = mlp->train(trainSamples, cv::ml::ROW_SAMPLE, trainResponses);
	return m_isTrainSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTann::PrintConfusionMatrix(const std::vector<int>& testOutputPredicted, const std::vector<int>& testOutputExpected)
{
	std::cout << "Confusion matrix: " << std::endl;
	std::vector<std::vector<int> > confusionMatrix(2, std::vector<int>(2));

	std::vector<std::string> classes(m_setClass.begin(), m_setClass.end());


	for (int i = 0; i < testOutputPredicted.size(); i++)
	{
		int expectedClass = testOutputExpected[i];
		int predictedClass = testOutputPredicted[i];
		confusionMatrix[expectedClass][predictedClass]++;

		if (predictedClass == expectedClass)
		{
			PrintMessageGreen("%d %s: %s", i, classes[expectedClass].c_str(), classes[predictedClass].c_str());
		}
		else
		{
			PrintError("%d %s: %s", i, classes[expectedClass].c_str(), classes[predictedClass].c_str());
		}
	}

	for (auto it = m_setClass.begin(); it != m_setClass.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	for (size_t i = 0; i < confusionMatrix.size(); i++)
	{
		for (size_t j = 0; j < confusionMatrix[i].size(); j++)
		{
			std::cout << confusionMatrix[i][j] << " ";
		}
		std::cout << std::endl;
	}

	//print accuracy of confusion matrix
	int hits = 0;
	int total = 0;
	for (size_t i = 0; i < confusionMatrix.size(); i++)
	{
		for (size_t j = 0; j < confusionMatrix.at(i).size(); j++)
		{
			if (i == j) hits += confusionMatrix.at(i).at(j);
			total += confusionMatrix.at(i).at(j);
		}
	}
	std::cout << "Accuracy: " << hits / (float)total <<"\n";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTann::SaveModel()
{
	mlp->save("ann.yaml");

	//save vocabulary
	cv::FileStorage fs("vocabulary.yaml", cv::FileStorage::WRITE);
	fs << "vocabulary" << m_vocabulary;
	fs.release();	


	//save classes
	std::string classes;
	for (auto it = m_setClass.begin(); it != m_setClass.end(); ++it)
	{
		classes += *it + "\n";		
	}
	classes = classes.substr(0, classes.length() - 1);
	TGMTfile::WriteToFile("ann_classes.txt", classes, false);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTann::LoadModel()
{
	LoadSetting();

	//load ann
	ASSERT(TGMTfile::FileExist("ann.yaml"), "Can not find file ann.yaml");
	ASSERT(TGMTfile::FileExist("vocabulary.yaml"), "Can not find file vocabulary.yaml");
	ASSERT(TGMTfile::FileExist("ann_classes.txt"), "Can not find file ann_classes.txt");

	mlp = StatModel::load<ANN_MLP>("ann.yaml");
	if (mlp.empty())
	{
		PrintError("Can not load ANN");
		return false;
	}

	//load vocabulary
	cv::FileStorage fs("vocabulary.yaml", cv::FileStorage::READ);
	fs["vocabulary"] >> m_vocabulary;
	fs.release();

	//train flann
	m_flann.clear();
	m_flann.add(m_vocabulary);
	m_flann.train();


	//load class
	std::vector<std::string> classes = TGMTfile::ReadAllLine("ann_classes.txt");
	if (classes.size() == 0)
		return false;

	m_setClass.clear();
	m_setClass = std::set<std::string>(classes.begin(), classes.end());

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTann::GetBOWFeatures(const cv::Mat& descriptors, int vocabularySize)
{
	cv::Mat outputArray = cv::Mat::zeros(cv::Size(vocabularySize, 1), CV_32F);
	std::vector<cv::DMatch> matches;
	m_flann.match(descriptors, matches);
	for (size_t j = 0; j < matches.size(); j++)
	{
		int visualWord = matches[j].trainIdx;
		outputArray.at<float>(visualWord)++;
	}
	return outputArray;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTann::GetClassMat(const std::string& classname)
{

	cv::Mat code = cv::Mat::zeros(cv::Size((int)m_setClass.size(), 1), CV_32F);
	int index = GetClassId(classname);
	code.at<float>(index) = 1;
	return code;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTann::GetClassId(const std::string& classname)
{
	int index = 0;
	for (auto it = m_setClass.begin(); it != m_setClass.end(); ++it)
	{
		if (*it == classname) break;
		++index;
	}
	return index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTann::GetDescriptors(const cv::Mat& img)
{
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
	kaze->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
	//ShowImage(descriptors, "decriptors");
	//cv::waitKey(1);
	return descriptors;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTann::TestAccuracy(const std::vector<std::string> files, const std::vector<std::string> classesExpected)
{
	// Reading test set 
	std::cout << "Reading test set..." << std::endl;
	std::vector<int> testOutputExpected;
	std::vector<int> testOutputPredicted;
	int idx = 0;

	for (int i = 0; i < files.size(); i++)
	{
		std::vector<TGMTann::PredictData> probabilities;
		
		TGMTann::PredictData highestData = PredictClassess(files[i], probabilities);
		testOutputPredicted.push_back(GetClassId(highestData.classname));

		std::string classname = classesExpected[idx++];
		testOutputExpected.push_back(GetClassId(classname));
	}
	
	PrintConfusionMatrix(testOutputPredicted, testOutputExpected);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

TGMTann::PredictData TGMTann::PredictClassess(std::string filePath, std::vector<PredictData> &probabilities)
{
	cv::Mat mat = cv::imread(filePath, CV_LOAD_IMAGE_GRAYSCALE);
	return PredictClassess(mat, probabilities);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

TGMTann::PredictData TGMTann::PredictClassess(cv::Mat matInput, std::vector<PredictData> &probabilities)
{
	cv::Mat matProcessed = ReadImageFeature(matInput);
	cv::Mat predictions;
	mlp->predict(matProcessed, predictions);

	std::vector<std::string> classes(m_setClass.begin(), m_setClass.end());

	const float* ptrPredictions = predictions.row(0).ptr<float>(0);
	float highestProbability = 0.f;
	int highestPredictedClass;

	for (int i = 0; i < predictions.cols; i++)
	{
		float prediction = *ptrPredictions++;

		prediction = (prediction / 3.4318) + 0.5;

#ifdef _DEBUG
		ASSERT(prediction > 0, "negative result");
		ASSERT(prediction < 1, "bigger than 1");
#endif

		PredictData predictData;
		predictData.classname = classes[i];
		predictData.probability = prediction;

		probabilities.push_back(predictData);

		if (highestProbability < prediction)
		{
			highestProbability = prediction;
			highestPredictedClass = i;
		}
	}

	PredictData highestPredictData;
	highestPredictData.classname = classes[highestPredictedClass];
	highestPredictData.probability = highestProbability;
	return highestPredictData;
}
#endif