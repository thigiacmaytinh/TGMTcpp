#include "TGMTface.h"
#include "TGMTdebugger.h"
#include "TGMTConfig.h"
#include "TGMTutil.h"
#include "TGMTdebugger.h"
#include "TGMTfile.h"
#include "TGMTimage.h"
#include "TGMTdraw.h"
#include "TGMTutil.h"
#include "TGMTbrightness.h"
#include "TGMTshape.h"

#include <time.h>
#include "XProfiler.h"
TGMTface* TGMTface::m_instance = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTface::TGMTface()
{
	m_facialDir = "facial";
	m_faceDir = "face";
	m_originalDir = "original";

#ifdef LIB_CS
	GetTGMTConfig()->LoadSettingFromFile();	
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTface::~TGMTface()
{
	mCascadeFace.~CascadeClassifier();
	mCascadeEyes.~CascadeClassifier();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTface::Init(std::string cascadeFace, std::string cascadeEye, FaceModel model)
{
	if (cascadeFace == "")
	{
		cascadeFace = GetTGMTConfig()->ReadValueString(INI_TGMTFACE, "face_cascade_name");
	}
	
	ASSERT(!cascadeFace.empty(), "Face cascade name is null");	
	mCascadeFace = cv::CascadeClassifier(cascadeFace);
	ASSERT(!mCascadeFace.empty(), "Can not load face cascade: " + cascadeFace);

	m_shouldDetectEye = GetTGMTConfig()->ReadValueBool(INI_TGMTFACE, "should_detect_eye");
	if (m_shouldDetectEye)
	{
		if (cascadeEye == "")
		{
			cascadeEye = GetTGMTConfig()->ReadValueString(INI_TGMTFACE, "eye_cascade_name");
		}
		ASSERT(!cascadeEye.empty(), "Eye cascade name is null");
		mCascadeEyes = cv::CascadeClassifier(cascadeEye);
		ASSERT(!mCascadeEyes.empty(), "Can not load eye cascade: " + cascadeEye);
	}
	
	

	m_faceSize = GetTGMTConfig()->ReadValueInt(INI_TGMTFACE, "face_size");
	m_shouldDetectSkin = GetTGMTConfig()->ReadValueBool(INI_TGMTFACE, "should_detect_skin");
	m_minFaceSize = GetTGMTConfig()->ReadValueInt(INI_TGMTFACE, "min_face_size");
	if (m_minFaceSize < 100)
		m_minFaceSize = 100;
	m_maxFaceSize = GetTGMTConfig()->ReadValueInt(INI_TGMTFACE, "max_face_size");
	m_correctFaceAngle = GetTGMTConfig()->ReadValueBool(INI_TGMTFACE, "correct_face_angle");
	m_autoLuminance = GetTGMTConfig()->ReadValueBool(INI_TGMTFACE, "auto_luminance");
	m_autoRetrain = GetTGMTConfig()->ReadValueBool(INI_TGMTFACE, "auto_retrain");
	m_enableEqualizeHist = GetTGMTConfig()->ReadValueBool(INI_TGMTFACE, "enable_equalizeHist");
	m_scaleFactor = GetTGMTConfig()->ReadValueDouble(INI_TGMTFACE,"scale_factor", 1.1);
	m_minNeighbors = GetTGMTConfig()->ReadValueInt(INI_TGMTFACE, "min_neighbors", 3);
	m_expandFaceRatio = GetTGMTConfig()->ReadValueDouble(INI_TGMTFACE, "expand_face_ratio", 1.f);
	m_blurryFactor = GetTGMTConfig()->ReadValueInt(INI_TGMTFACE, "blurry_factor", 50);
	
	m_facialDir = GetTGMTConfig()->ReadValueString(INI_TGMTFACE, "facial_dir", m_facialDir);
	m_faceDir = GetTGMTConfig()->ReadValueString(INI_TGMTFACE, "face_dir", m_faceDir);
	m_originalDir = GetTGMTConfig()->ReadValueString(INI_TGMTFACE, "original_dir", m_originalDir);


	TGMTfile::CreateDir(m_facialDir);
	TGMTfile::CreateDir(m_faceDir);
	TGMTfile::CreateDir(m_originalDir);

	if (model == undefine)
	{
		std::string modelName = GetTGMTConfig()->ReadValueString(INI_TGMTFACE, "model");

		if (modelName == FACE_MODEL_EIGEN)
			model = Eigen;
		else if (modelName == FACE_MODEL_FISHER)
			model = Fisher;
		else if (modelName == FACE_MODEL_LBPH)
			model = LBPH;
		else
			ASSERT(false, "Model name not defined");
	}
	
	//if (!m_autoRetrain)
	//{
	//	TGMTfile::CreateDir(FACIAL_DIR + "\\new");
	//}

	SetModel(model);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<cv::Rect> TGMTface::DetectFaces(std::string fileInput)
{
	return DetectFaces(cv::imread(fileInput, cv::IMREAD_GRAYSCALE));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<cv::Rect> TGMTface::DetectFaces(cv::Mat matInput)
{
	std::vector<cv::Rect> rects;
	if (matInput.data == nullptr)
	{
		PrintError("Input image is null");
		return rects;
	}
	//START_COUNT_TIME("detect_face");


	//scale to 1000 pixels
	float scaleRatio = 1.f;
	if (m_resizeInputImage)
	{
		if (matInput.cols > 1000)
		{
			scaleRatio = 1000.f / matInput.cols;
			cv::resize(matInput, matInput, cv::Size(1000, scaleRatio * matInput.rows));
		}
		else if (matInput.rows > 1000)
		{
			scaleRatio = 1000.f / matInput.rows;
			cv::resize(matInput, matInput, cv::Size(scaleRatio * matInput.cols, 1000));
		}
	}

	cv::Mat matGray = matInput.clone();
	TGMTimage::ConvertToGray(matGray);
	if (m_enableEqualizeHist)
	{
		TGMTbrightness::EqualizeHist(matGray);
	}
	
	mCascadeFace.detectMultiScale(matGray, rects, m_scaleFactor, m_minNeighbors, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(m_minFaceSize, m_minFaceSize), cv::Size(m_maxFaceSize, m_maxFaceSize) );
	if (rects.size() == 0)
		return rects;



	//detect eye to improve accuracy
	if (m_shouldDetectEye)
	{
		std::vector<cv::Rect> temp = rects;
		rects.clear();
		for (int i = 0; i < temp.size(); i++)
		{
			std::vector<cv::Rect> eyes = DetectEyes(matGray(temp[i]));
			if (eyes.size() >= 2)
			{
				rects.push_back(temp[i]);
#if _DEBUG
				cv::rectangle(matInput, cv::Rect(eyes[0].tl() + temp[0].tl(), eyes[0].br() + temp[0].tl()), cv::Scalar(0, 255, 0));
				cv::rectangle(matInput, cv::Rect(eyes[1].tl() + temp[0].tl(), eyes[1].br() + temp[0].tl()), cv::Scalar(0, 255, 0));
#endif
				if (m_correctFaceAngle)
				{
					TODO("implement face angle correction later");
					DEBUG_OUT("Angle %f", GetFaceAngle(eyes[0], eyes[1]));
				}
			}
		}
	}


	//detect people skin to improve accuracy
	if (m_shouldDetectSkin)
	{
		if (matInput.channels() == 3 && TGMTbrightness::GetLuxValue(matInput) > 60)
		{
			//skin color in range
			cv::Scalar lower(0, 48, 80);
			cv::Scalar upper(20, 255, 255);


			std::vector<cv::Rect> temp = rects;
			rects.clear();
			for (int i = 0; i < temp.size(); i++)
			{
				cv::Mat matHsv;
				cv::cvtColor(matInput(temp[i]), matHsv, cv::COLOR_BGR2HSV);

				cv::inRange(matHsv, lower, upper, matHsv);

				std::vector<cv::Mat> channels(3);

				cv::split(matHsv, channels);

				cv::Scalar mean = cv::mean(channels[0]);
				if (mean[0] > 100)
					rects.push_back(temp[i]);

				//DEBUG_OUT("Avg face %d: %f", i, mean[0]);
				DEBUG_IMAGE(matHsv, "HSV %d", i);
			}
		}
		else
		{
			PrintError("Image input is not colored image, so we can detect skin face color");
		}

	}


	//return true size
	if (m_resizeInputImage)
	{
		if (scaleRatio != 1.f)
		{
			for (int i = 0; i < rects.size(); i++)
			{
				rects[i] = cv::Rect(rects[i].x / scaleRatio, rects[i].y / scaleRatio, rects[i].width / scaleRatio, rects[i].height / scaleRatio);
			}
		}
	}

	rects = TGMTshape::ExpandRects(rects, m_expandFaceRatio, m_expandFaceRatio, matInput);
	

	//DEBUG_IMAGE(matInput, "Mat input");

	//DEBUG_OUT("Detected %d faces", rects.size());
	//STOP_AND_PRINT_COUNT_TIME("detect_face");
	return rects;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTface::DetectAndDrawFaces(cv::Mat& imgInput)
{
	std::vector<cv::Rect> rects = DetectFaces(imgInput);
#ifndef LIB_CS
	TGMTdraw::PutText(imgInput, cv::Point(10, 30), cv::Scalar(0,0,255), "Detected %d face", rects.size());
#endif

	for (int i = 0; i < rects.size(); i++)
	{
		cv::rectangle(imgInput, rects[i], cv::Scalar(255, 0, 0));
	}

	return rects.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<cv::Rect> TGMTface::DetectEyes(cv::Mat imgInput)
{
	ASSERT(imgInput.data, "TGMTobjDetect::DetectEyes: image input error");

	std::vector<cv::Rect> rects;
	mCascadeEyes.detectMultiScale(imgInput.clone(), rects, 1.1, 3, 0 | cv::CASCADE_SCALE_IMAGE);

	return rects;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTface::DetectAndDrawFaces(std::string imgPath)
{
	cv::Mat mat = cv::imread(imgPath);
	return DetectAndDrawFaces(mat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<TGMTface::Person> TGMTface::DetectPersons(cv::Mat matInput, std::string &errMsg, bool faceCropped)
{
	
	std::vector<cv::Rect> rects;

	return DetectPersons(matInput, rects, errMsg, faceCropped);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<TGMTface::Person> TGMTface::DetectPersons(cv::Mat matInput, std::vector<cv::Rect> &rects, std::string &errMsg, bool faceCropped)
{
	ASSERT(matInput.data != nullptr, "mat input is null");
	std::string name = "";
	std::vector<Person> result;

	if (faceCropped)
	{
		rects.push_back(cv::Rect(0, 0, matInput.cols, matInput.rows));
	}
	else
	{
		rects = DetectFaces(matInput);
	}

	if (rects.size() == 0)
		errMsg = "Can not detect face";

	for (int i = 0; i < rects.size(); i++)
	{
		Person p = PredictPerson(matInput(rects[i]));
		p.rect = rects[i];
		result.push_back(p);
	}	

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TGMTface::Person TGMTface::PredictPerson(cv::Mat faceCropped)
{
	ASSERT(faceCropped.data, "faceInput error");

	if (!m_trained || m_Persons.size() == 0)
	{
		return Person();
	}
	else if (m_model == undefine || m_pModel.empty())
	{
		return Person();
	}
	double confident;
		
	Person p = FindPerson(PredictLabel(faceCropped, confident));
	p.confident = confident;
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTface::PredictLabel(cv::Mat matFace, double &confident)
{
	cv::resize(matFace, matFace, cv::Size(m_faceSize, m_faceSize));

	ImproveContrast(matFace, matFace, 4);
	ImproveFaceOrientation(matFace, matFace);

#ifdef _DEBUG
	ShowImage(matFace, "facial");
#endif

	int label;
	m_pModel->predict(matFace, label, confident);

	return label;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTface::Training()
{
	return Training(m_facialDir);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTface::Training(std::string dir)
{
	XSCOPE("TGMTface::Training");
	std::vector<std::string> subDirs = TGMTfile::GetChildDirInDir(dir);
	if (subDirs.size() == 0)
	{
#ifdef _MANAGED
		ShowMessageBox("Notice", "Training model fail: Folder is empty");
#else
		PrintError("Training model fail: Folder %s is empty", dir.c_str());
#endif
		return false;
	}
	else if (m_model == Fisher && subDirs.size() <= 2)
	{
#ifdef _MANAGED
		ShowMessageBox("Notice", "Fisher need least 2 persons");
#else
		PrintError("Fisher need least 2 persons");
#endif
		return false;
	}

	PrintMessage("Start training face in directory: %s", dir.c_str());

	clock_t startTime = clock();

	m_Persons.clear();
	std::vector<cv::Mat> imgs;
	std::vector<int> labels;
	int count = 0;
	for (int i = 0; i < subDirs.size(); i++)
	{
		XSCOPE("Train dir");
		std::string currentDir = subDirs[i];
		std::string dirName = TGMTfile::GetDirName(currentDir);
		if (dirName == ".svn")
			continue;
	
		Person p;
		p.name = TGMTfile::GetFileName(currentDir);
		p.label = count++;
		p.isEmpty = false;

		PrintMessage("%d: %s", p.label + 1, currentDir.c_str());
		
		std::vector<std::string> imgList = TGMTfile::GetImageFilesInDir(currentDir);
		for (int j = 0; j < imgList.size(); j++)
		{
			XSCOPE("Train img");
			cv::Mat img = cv::imread(imgList[j], cv::IMREAD_GRAYSCALE);

			if (img.cols < m_faceSize || img.rows < m_faceSize)
			{
				PrintError("Can not train %s because size smaller than %d", imgList[j].c_str(), m_faceSize);
				continue;
			}				
			else if (img.cols > m_faceSize || img.rows > m_faceSize)
			{
				cv::resize(img, img, cv::Size(m_faceSize, m_faceSize));
			}				

			if (img.data)
			{
				imgs.push_back(img);
				labels.push_back(p.label);
				p.numImgs++;
			}
		}

		m_Persons.push_back(p);
	}
	if (imgs.size() < 2)
	{
		PrintError("Load images fail: not enough images to train, must bigger than 2");
		return false;
	}

	if (m_model == Eigen || m_model == Fisher || m_model == LBPH)
	{
		XSCOPE("m_pModel->train");
		m_pModel->train(imgs, labels);
	}
	else
	{
		ASSERT(m_model != undefine, "Model must different undefine")
	}

	clock_t elapsed =  clock() - startTime;
	PrintSuccess("Train %d face success (%d ms)", imgs.size(), (int)elapsed);


	m_trained = true;
	return m_trained;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTface::SaveModel()
{
	std::string modelName;
	if (m_model == Eigen)
	{
		modelName = "face_eigen.yaml";
	}
	else if (m_model == LBPH)
	{
		modelName = "face_LBPH.yaml";
	}
	else if (m_model == Fisher)
	{
		modelName = "face_fisher.yaml";
	}
	return SaveModel(modelName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTface::SaveModel(std::string modelFileName)
{
	m_pModel->save(modelFileName);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTface::LoadModel()
{
	std::string modelName;
	if (m_model == Eigen)
	{
		modelName = "face_eigen.yaml";
	}
	else if (m_model == LBPH)
	{
		modelName = "face_LBPH.yaml";
	}
	else if (m_model == Fisher)
	{
		modelName = "face_fisher.yaml";
	}
	return LoadModel(modelName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTface::LoadModel(std::string modelFileName)
{
	clock_t startTime = clock();
	if (!TGMTfile::FileExist(modelFileName))
	{
		PrintError("File %s does not exist", modelFileName.c_str());
		return false;
	}
	
#if defined(OPENCV_320)
	m_pModel->load(modelFileName);
#elif defined(OPENCV_342)
	if (m_model == Eigen)
	{		
		m_pModel = cv::Algorithm::load<EigenFaceRecognizer>(modelFileName);
	}
	else if (m_model == Fisher)
	{
		m_pModel = cv::Algorithm::load<FisherFaceRecognizer>(modelFileName);
	}
	else if (m_model == LBPH)
	{
		m_pModel = cv::Algorithm::load<LBPHFaceRecognizer>(modelFileName);
	}
#endif
	clock_t elapsed = clock() - startTime;
	PrintMessage("Elapsed time %d (ms)", (int)elapsed);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TGMTface::Person TGMTface::FindPerson(std::string name)
{
	for (int i = 0; i < m_Persons.size(); i++)
	{
		if (m_Persons[i].name == name)
			return m_Persons[i];
	}
	return Person();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

TGMTface::Person TGMTface::FindPerson(int label)
{
	for (int i = 0; i < m_Persons.size(); i++)
	{
		if (m_Persons[i].label == label)
			return m_Persons[i];
	}
	return Person();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTface::AddNewImage(cv::Mat matFace, std::string name, bool saveToIndividualDir)
{
	int label = FindPerson(name).label;
	if (label == -1)
	{
		label = m_Persons.size();
	}
	

	matFace = TGMTimage::ConvertToGray(matFace);
	cv::resize(matFace, matFace, cv::Size(m_faceSize, m_faceSize));
	if (m_enableEqualizeHist)
	{
		TGMTbrightness::EqualizeHist(matFace);
	}

	if (m_autoRetrain)
	{
		if (saveToIndividualDir)
		{
			TGMTfile::CreateDir(m_facialDir + "\\" + name);
			WriteImage(matFace, (m_facialDir + "\\" + name + "\\" + GetCurrentDateTime(true) + ".png").c_str());
		}
		else
		{
			WriteImage(matFace, (m_facialDir + "\\" + name + GetCurrentDateTime(true) + ".png").c_str());
		}

		std::vector<int> labels;
		labels.push_back(label);

		std::vector<cv::Mat> matFaces;
		matFaces.push_back(matFace);

		if (m_model == LBPH)
		{
			m_pModel->update(matFaces, labels);
		}
		else if (m_model == Eigen || m_model == Fisher)
		{
			Training(m_facialDir);
		}
	}	
	else
	{
		WriteImage(matFace, (m_facialDir + "\\new\\" + name + GetCurrentDateTime(true) + ".png").c_str());
	}

#ifndef _MANAGED
	PrintMessage("Added user %s", name.c_str());
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTface::SetModel(FaceModel model)
{
	if (m_model == model)
		return;

	if (model == Eigen)
	{
#if defined(OPENCV_320)
		m_pModel = createEigenFaceRecognizer();
#elif defined(OPENCV_342)
		m_pModel = EigenFaceRecognizer::create();
#endif
	}	
	else if (model == Fisher)
	{
#if defined(OPENCV_320)
		m_pModel = createFisherFaceRecognizer();	
#elif defined(OPENCV_342)
		m_pModel = FisherFaceRecognizer::create();
#endif
	}
	else if (model == LBPH)
	{
#if defined(OPENCV_320)
		m_pModel = createLBPHFaceRecognizer();
#elif defined(OPENCV_342)
		m_pModel = LBPHFaceRecognizer::create();
#endif
	}
	else
	{
		ASSERT(false, "Model must different undefine");
	}
	m_model = model;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTface::SetFaceSize(int faceSize)
{
	if (faceSize < m_minFaceSize || faceSize > m_maxFaceSize)
		return;

	m_faceSize = faceSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

double TGMTface::GetFaceAngle(cv::Rect eyeLeft, cv::Rect eyeRight)
{
	cv::Point centerLeft = cv::Point(eyeLeft.x + eyeLeft.width / 2, eyeLeft.y + eyeLeft.height / 2);
	cv::Point centerRight = cv::Point(eyeRight.x + eyeRight.width / 2, eyeRight.y + eyeRight.height / 2);
	double eyeXdis = centerRight.x - centerLeft.x;
	double eyeYdis = centerRight.y - centerLeft.y;
	double angle = atan(eyeYdis / eyeXdis);
	double degree = angle * 180 / CV_PI;
	return degree;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTface::ImproveContrast(cv::Mat src, cv::Mat &dest, int threshold)
{
	cv::Mat lab_image;
	cv::cvtColor(src, lab_image, cv::COLOR_BGR2Lab);

	std::vector<cv::Mat> lab_planes(3);
	cv::split(lab_image, lab_planes);

	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(threshold);
	cv::Mat dst;
	clahe->apply(lab_planes[0], dst);

	dst.copyTo(lab_planes[0]);
	cv::merge(lab_planes, lab_image);

	cv::cvtColor(lab_image, dest, cv::COLOR_Lab2BGR);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTface::ImproveFaceOrientation(cv::Mat src, cv::Mat &dest)
{
	dest = src.clone();

	cv::Mat gray = TGMTimage::ConvertToGray(dest);
	

	if (!m_shouldDetectEye && mCascadeEyes.empty())
	{
		std::string cascadeEye = GetTGMTConfig()->ReadValueString(INI_TGMTFACE, "eye_cascade_name");
		mCascadeEyes = cv::CascadeClassifier(cascadeEye);
	}
	dest = gray;
	std::vector<cv::Rect> eyes = DetectEyes(gray);
	if (eyes.size() < 2)
	{
		PrintError("Only detect 1 eye");
		return false;
	}

	if (eyes.size() > 2)
	{
		//remove eye below nose
		for (int i = eyes.size() - 1; i >= 0 ; i--)
		{
			if (eyes[i].y > src.rows / 2)
			{
				eyes[i] = eyes[eyes.size() - 1];
				eyes.pop_back();
			}				
		}
	}

	if (eyes.size() > 2)
	{
		//remove overlapped
		for (int i = eyes.size() - 1; i > 0; i--)
		{
			if (TGMTshape::IsOverlap(eyes[i], eyes[i - 1]))
			{
				eyes[i] = eyes[eyes.size() - 1];
				eyes.pop_back();
			}
		}
	}

	if (eyes.size() > 2)
	{
#ifdef _DEBUG
		TGMTdraw::DrawRectangles(src, eyes, 1, RED);
		ShowImage(src, "Eye");
#endif
		PrintError("Detected %d eyes", eyes.size());
		//TODO: retrain eyes
		return false;
	}

	if (eyes.size() < 2)
	{
		return false;
	}

	cv::Rect leftEye, rightEye;
	if (eyes[0].x < eyes[1].x)
	{
		leftEye = eyes[0];
		rightEye = eyes[1];
	}
	else
	{
		leftEye = eyes[1];
		rightEye = eyes[0];
	}

	cv::Point2f centerLeft = TGMTshape::GetCenterPoint(leftEye);
	cv::Point2f centerRight = TGMTshape::GetCenterPoint(rightEye);

	cv::Point2f eyesCenter = cv::Point2f((centerLeft.x + centerRight.x) * 0.5f, (centerLeft.y + centerRight.y) * 0.5f);
	
	const double DESIRED_LEFT_EYE_X = 0.2;
	const double DESIRED_LEFT_EYE_Y = 0.18;

	// Hand measurements shown that the left eye center should ideally be at roughly (0.16, 0.14) of a scaled face image.
	const double DESIRED_RIGHT_EYE_X = (1.0f - DESIRED_LEFT_EYE_X);

	double dy = (centerRight.y - centerLeft.y);
	double dx = (centerRight.x - centerLeft.x);
	double len = sqrt(dx*dx + dy*dy);
	double angle = atan2(dy, dx) * 180.0 / CV_PI;

	if (abs(angle) > 45)
		return false;

	// Get the amount we need to scale the image to be the desired fixed size we want.
	double desiredLen = (DESIRED_RIGHT_EYE_X - DESIRED_LEFT_EYE_X) * 200;
	double scale = desiredLen / len;
		
	cv::Mat rot_mat = cv::getRotationMatrix2D(eyesCenter, angle, 1);


	// Rotate and scale and translate the image to the desired angle & size & position!
	cv::Mat warped = dest.clone();
	cv::warpAffine(gray, warped, rot_mat, warped.size());

	cv::Mat filtered = cv::Mat(warped.size(), CV_8U);
	cv::bilateralFilter(warped, filtered, 0, 20.0, 2.0);

	cv::Mat mask = cv::Mat(warped.size(), CV_8U, cv::Scalar(0)); // Start with an empty mask.
	cv::Point faceCenter = cv::Point(warped.size().width / 2, warped.size().height/2);
	cv::Size size = cv::Size(cvRound(warped.size().width * 0.38), cvRound(warped.size().height * 0.5));
	cv::ellipse(mask, faceCenter, size, 0, 0, 360, cv::Scalar(255), cv::FILLED);

	cv::Mat dstImg = cv::Mat(warped.size(), CV_8U, cv::Scalar(128));

	filtered.copyTo(dstImg, mask);

	dest = dstImg.clone();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTface::DetectAndWriteFace(cv::Mat matInput, std::string name)
{
	std::vector<cv::Rect> rects = DetectFaces(matInput);
	if (rects.size() == 0)
	{
		PrintError("Not found any face");
		return false;
	}
	

	TGMTfile::CreateDir(TGMTfile::CorrectPath(m_originalDir) + name);
	WriteImageAsync(matInput, (TGMTfile::CorrectPath(m_originalDir) + name + "\\" + name + GetCurrentDateTime(true) + ".png" ).c_str());

	for (int i = 0; i < rects.size(); i++)
	{
		WriteFace(matInput(rects[i]), name, name + TGMTutil::FormatString("%d", i));
	}

	PrintSuccess("Written %d face", rects.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTface::WriteFace(cv::Mat matFace, std::string dirName, std::string fileName)
{
	//if (TGMTimage::IsBlurryImage(matFace, m_blurryFactor))
	//{
	//	PrintError("Image too blurry");
	//	return;
	//}
	std::string datetime = GetCurrentDateTime(true);

	if (matFace.cols > 200 || matFace.rows > 200)
		cv::resize(matFace, matFace, cv::Size(200, 200));

	if (dirName.substr(1, 1) == ":")
	{
		dirName = TGMTfile::GetDirName(dirName);
	}

	if (fileName == "")
		fileName = datetime + ".png";
	else if (fileName.substr(fileName.length() - 4) != ".png")
		fileName += datetime + ".png";

	TGMTfile::CreateDir(TGMTfile::CorrectPath(m_faceDir) + dirName);
	WriteImageAsync(matFace, (TGMTfile::CorrectPath(m_faceDir) + dirName + "\\" + fileName).c_str());

	ImproveContrast(matFace, matFace, 4);	
	ImproveFaceOrientation(matFace, matFace);
	
	TGMTfile::CreateDir(TGMTfile::CorrectPath(m_facialDir) + dirName);
	std::string filePath = TGMTfile::CorrectPath(m_facialDir) + dirName + "\\" + fileName;
	WriteImage(matFace, filePath.c_str());	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTface::Reload()
{
	Init();
	Training();
}