#include "handGesture.hpp"
#include "TGMTutil.h"
#include "TGMTdebugger.h"


HandGesture::HandGesture(int cameraId)
{
	cap = cv::VideoCapture(cameraId);
	ASSERT(cap.isOpened(), "Can not get image from camera");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

HandGesture::HandGesture()
{
	HandGesture(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

HandGesture::~HandGesture()
{
	cap.release();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandGesture::InitVectors()
{
	hullI = std::vector<std::vector<int> >(contours.size());
	hullP = std::vector<std::vector<cv::Point> >(contours.size());
	defects = std::vector<std::vector<cv::Vec4i> >(contours.size());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandGesture::PrintGestureInfo(cv::Mat src)
{
	if (isHand && nrOfDefects < 5)
	{
		//cout << "Number of fingers: " << nrOfDefects + 1 << "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HandGesture::IsHand()
{
	double h = bRect.height;
	double w = bRect.width;
	if (fingerTips.size() > 5) {
		return false;
	}
	else if (h == 0 || w == 0) {
		return false;
	}
	else if (h / w > 4 || w / h > 4) {
		return false;
	}
	else if (bRect.x < 20) {
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

float HandGesture::DistanceP2P(cv::Point a, cv::Point b)
{
	float d = sqrt(fabs(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)));
	return d;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// remove fingertips that are too close to 
// eachother
void HandGesture::removeRedundantFingerTips()
{
	std::vector<cv::Point> newFingers;
	for (int i = 0; i < fingerTips.size(); i++) 
	{
		for (int j = i; j < fingerTips.size(); j++)
		{
			if (DistanceP2P(fingerTips[i], fingerTips[j]) < 10 && i != j)
			{
			}
			else 
			{
				newFingers.push_back(fingerTips[i]);
				break;
			}
		}
	}
	fingerTips.swap(newFingers);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandGesture::ComputeFingerNumber() 
{
	std::sort(fingerNumbers.begin(), fingerNumbers.end());
	int frequentNr;
	int thisNumberFreq = 1;
	int highestFreq = 1;
	if (fingerNumbers.size() == 0)
		return;
	frequentNr = fingerNumbers[0];
	for (int i = 1; i < fingerNumbers.size(); i++) 
	{
		if (fingerNumbers[i - 1] != fingerNumbers[i]) 
		{
			if (thisNumberFreq > highestFreq) 
			{
				frequentNr = fingerNumbers[i - 1];
				highestFreq = thisNumberFreq;
			}
			thisNumberFreq = 0;
		}
		thisNumberFreq++;
	}
	if (thisNumberFreq > highestFreq) {
		frequentNr = fingerNumbers[fingerNumbers.size() - 1];
	}
	mostFrequentFingerNumber = frequentNr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandGesture::AddFingerNumberToVector()
{
	int i = fingerTips.size();
	if (i > 5)
	{
		fingerTips.clear();
		return;
	}
	fingerNumbers.push_back(i);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// calculate most frequent numbers of fingers 
// over 20 frames
void HandGesture::GetFingerNumber()
{
	removeRedundantFingerTips();
	if (bRect.height > src.rows / 2)
	{
		AddFingerNumberToVector();
		//if(frameNumber>12)
		{
			//nrNoFinger=0;
			//frameNumber=0;	
			ComputeFingerNumber();
			numbers2Display.push_back(mostFrequentFingerNumber);
			fingerNumbers.clear();
		}
		//else{
			//frameNumber++;
		//}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

float HandGesture::getAngle(cv::Point s, cv::Point f, cv::Point e)
{
	float l1 = DistanceP2P(f, s);
	float l2 = DistanceP2P(f, e);
	float dot = (s.x - f.x)*(e.x - f.x) + (s.y - f.y)*(e.y - f.y);
	float angle = acos(dot / (l1*l2));
	angle = angle * 180 / CV_PI;
	return angle;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandGesture::EleminateDefects()
{
	int tolerance = bRect_height / 5;

	std::vector<cv::Vec4i> newDefects;
	int startidx, endidx, faridx;
	std::vector<cv::Vec4i>::iterator d = defects[cIdx].begin();
	while (d != defects[cIdx].end()) 
	{
		cv::Vec4i& v = (*d);
		startidx = v[0]; cv::Point ptStart(contours[cIdx][startidx]);
		endidx = v[1]; cv::Point ptEnd(contours[cIdx][endidx]);
		faridx = v[2]; cv::Point ptFar(contours[cIdx][faridx]);
		if (DistanceP2P(ptStart, ptFar) > tolerance && DistanceP2P(ptEnd, ptFar) > tolerance && getAngle(ptStart, ptFar, ptEnd) < 95)
		{
			if (ptEnd.y > (bRect.y + bRect.height - bRect.height / 4)) {
			}
			else if (ptStart.y > (bRect.y + bRect.height - bRect.height / 4)) {
			}
			else {
				newDefects.push_back(v);
			}
		}
		d++;
	}
	nrOfDefects = newDefects.size();
	defects[cIdx].swap(newDefects);
	RemoveRedundantEndPoints(defects[cIdx]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// remove endpoint of convexity defects if they are at the same fingertip
void HandGesture::RemoveRedundantEndPoints(std::vector<cv::Vec4i> newDefects)
{
	cv::Vec4i temp;
	float avgX, avgY;
	float tolerance = bRect_width / 6;
	int startidx, endidx, faridx;
	int startidx2, endidx2;
	for (int i = 0; i < newDefects.size(); i++) {
		for (int j = i; j < newDefects.size(); j++) 
		{
			startidx = newDefects[i][0]; cv::Point ptStart(contours[cIdx][startidx]);
			endidx = newDefects[i][1]; cv::Point ptEnd(contours[cIdx][endidx]);
			startidx2 = newDefects[j][0]; cv::Point ptStart2(contours[cIdx][startidx2]);
			endidx2 = newDefects[j][1]; cv::Point ptEnd2(contours[cIdx][endidx2]);
			if (DistanceP2P(ptStart, ptEnd2) < tolerance)
			{
				contours[cIdx][startidx] = ptEnd2;
				break;
			}
			if (DistanceP2P(ptEnd, ptStart2) < tolerance)
			{
				contours[cIdx][startidx2] = ptEnd;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// convexity defects does not check for one finger
// so another method has to check when there are no
// convexity defects
void HandGesture::CheckForOneFinger()
{
	int yTol = bRect.height / 6;
	cv::Point highestP;
	highestP.y = src.rows;
	std::vector<cv::Point>::iterator d = contours[cIdx].begin();
	while (d != contours[cIdx].end()) {
		cv::Point v = (*d);
		if (v.y < highestP.y) 
		{
			highestP = v;
			//cout<<highestP.y<<endl;
		}
		d++;
	}int n = 0;
	d = hullP[cIdx].begin();
	while (d != hullP[cIdx].end())
	{
		cv::Point v = (*d);
		//cout<<"x " << v.x << " y "<<  v.y << " highestpY " << highestP.y<< "ytol "<<yTol<<endl;
		if (v.y < highestP.y + yTol && v.y != highestP.y && v.x != highestP.x)
		{
			n++;
		}
		d++;
	}if (n == 0) {
		fingerTips.push_back(highestP);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandGesture::DrawFingerTips()
{
	cv::Point p;
	for (int i = 0; i < fingerTips.size(); i++)
	{
		p = fingerTips[i];
		putText(src, TGMTutil::IntToString(i), p - cv::Point(0, 30), cv::FONT_HERSHEY_PLAIN, 1.2f, cv::Scalar(200, 0, 0), 2);
		circle(src, p, 5, cv::Scalar(100, 255, 100), 8);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandGesture::GetFingerTips()
{
	fingerTips.clear();
	int i = 0;
	std::vector<cv::Vec4i>::iterator d = defects[cIdx].begin();
	while (d != defects[cIdx].end()) 
	{
		cv::Vec4i& v = (*d);
		int startidx = v[0]; cv::Point ptStart(contours[cIdx][startidx]);
		int endidx = v[1]; cv::Point ptEnd(contours[cIdx][endidx]);
		int faridx = v[2]; cv::Point ptFar(contours[cIdx][faridx]);
		if (i == 0)
		{
			fingerTips.push_back(ptStart);
			i++;
		}
		fingerTips.push_back(ptEnd);
		d++;
		i++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HandGesture::GetImageFromCamera()
{
	cap >> src;
}