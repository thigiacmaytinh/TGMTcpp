#pragma once
#include "stdafx.h"
#include "opencv2/face.hpp"

#define GetTGMTface TGMTface::GetInstance
#define INI_TGMTFACE "TGMTface"

#define FACE_MODEL_EIGEN "eigen"
#define FACE_MODEL_FISHER "fisher"
#define FACE_MODEL_LBPH "LBPH"

using namespace cv::face;

class TGMTface
{
public:
	struct Person
	{
		bool isEmpty = true;
		std::string name = "Guest";
		int age = 0;
		int gender = -1;
		int confident = -1;
		cv::Rect rect;
		int numImgs = 0;
		int label = -1;	
		bool exactly = false;

		Person(std::string _name, int _age, int _gender, int _confident, int _label)
		{
			name = _name;
			age = _age;
			gender = _gender;
			confident = _confident;
			label = _label;
			isEmpty = false;
		}

		Person()
		{
		}
	};
private:
	static TGMTface* m_instance;

	std::vector<cv::Rect> DetectEyes(cv::Mat imgInput);
	cv::CascadeClassifier mCascadeFace;
	cv::CascadeClassifier mCascadeEyes;
	cv::Ptr<FaceRecognizer> m_pModel;

	bool m_trained;
	int m_newPersonID = 1;
	int m_minFaceSize;
	int m_maxFaceSize;
	int m_minNeighbors;

	bool m_shouldDetectEye;
	bool m_shouldDetectSkin;
	bool m_correctFaceAngle;
	bool m_autoLuminance;
	bool m_autoRetrain;
	bool m_enableEqualizeHist;

	float m_scaleFactor;
	float m_expandFaceRatio;

	int m_blurryFactor;

	std::vector<Person> m_Persons;
	std::vector<Person> m_lastResult;

	std::string m_facialDir;
	std::string m_faceDir;
	std::string m_originalDir;

	Person FindPerson(std::string name);
	Person FindPerson(int label);

	double GetFaceAngle(cv::Rect eyeLeft, cv::Rect eyeRight);
public:
	enum FaceModel { Eigen, Fisher, LBPH, undefine };
	FaceModel m_model = undefine;


	//property
	int m_faceSize;
	int m_accuracy = -1;
	bool m_resizeInputImage = false;



	TGMTface();
	~TGMTface();

	static TGMTface* GetInstance()
	{
		if (!m_instance)
			m_instance = new TGMTface();
		return m_instance;
	}

	

	void TGMTface::Init(std::string cascadeFace = "", std::string cascadeEye = "", FaceModel model = undefine);

	std::vector<cv::Rect> DetectFaces(std::string fileInput);
	std::vector<cv::Rect> DetectFaces(cv::Mat imgInput);

	int DetectAndDrawFaces(cv::Mat& imgInput);
	int DetectAndDrawFaces(std::string imgPath);

	Person PredictPerson(cv::Mat faceCropped);
	std::vector<Person> DetectPersons(cv::Mat matInput, std::string &errMsg, bool faceCropped = false);
	std::vector<Person> DetectPersons(cv::Mat matInput, std::vector<cv::Rect> &rects, std::string &errMsg, bool faceCropped = false);
	int PredictLabel(cv::Mat face, double &confident);

	bool SaveModel();
	bool SaveModel(std::string modelFileName);

	bool LoadModel();
	bool LoadModel(std::string modelFileName);

	void SetModel(FaceModel model);

	//Face always is quad, so we only need to set 1 edge of quad
	void SetFaceSize(int faceSize);

	bool Training();
	bool Training(std::string dir);

	void AddNewImage(cv::Mat matFace, std::string name, bool saveToIndividualDir = true);
	bool DetectAndWriteFace(cv::Mat matInput, std::string name);
	void WriteFace(cv::Mat matFace, std::string dirName, std::string fileName = "");

	static void ImproveContrast(cv::Mat src, cv::Mat &dest, int threshold);
	bool ImproveFaceOrientation(cv::Mat src, cv::Mat &dest);

	void Reload();

	std::string GetFacialDir() { return m_facialDir; }
	std::string GetFaceDir() { return m_faceDir; }
	std::string GetOriginalDir() { return m_originalDir; }
};

