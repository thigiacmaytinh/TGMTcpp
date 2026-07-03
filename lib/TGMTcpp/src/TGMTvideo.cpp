#include "TGMTvideo.h"

#ifdef HAVE_OPENCV_VIDEOIO
#include "TGMTdebugger.h"
#include "TGMTfile.h"
#ifdef WIN32
#include <Windows.h>
#include <time.h>
#endif
#include "TGMTutil.h"


TGMTvideo* TGMTvideo::instance = NULL;

std::string m_outDir;

TGMTvideo::TGMTvideo()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTvideo::~TGMTvideo()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTvideo::LoadVideoFile(std::string videoFilePath)
{

	if (!TGMTfile::FileExist(videoFilePath))
		return false;

#ifdef WIN32
#if defined(OPENCV_320)
	ASSERT(TGMTfile::FileExist(TGMTfile::GetCurrentDir() + "\\opencv_ffmpeg320.dll"), "File opencv_ffmpeg320.dll not found");
#elif defined(OPENCV_342)
#if defined(WIN64)
	ASSERT(TGMTfile::FileExist(TGMTfile::GetCurrentDir() + "\\opencv_ffmpeg342_64.dll"), "File opencv_ffmpeg342_64.dll not found");
#else
	ASSERT(TGMTfile::FileExist(TGMTfile::GetCurrentDir() + "\\opencv_ffmpeg342.dll"), "File opencv_ffmpeg342.dll not found");
#endif
#endif
#endif
	mCap = cv::VideoCapture(videoFilePath);
	if (!mCap.isOpened())
	{		
		PrintError("Can not load video %s", videoFilePath.c_str());

		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTvideo::PlayVideo()
{
	PlayVideo(m_videoPath, cv::Size(m_inputWidth, m_inputHeight), m_fps, m_startIndex, m_playTotalFrame);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTvideo::PlayVideo(std::string videoFilePath, cv::Size maxSize, int fps, int startIndex, int total)
{
	if (!OnNewFrame)
		return;
	
	if (!LoadVideoFile(videoFilePath))
		return;
	m_stopVideo = false;

	uint amount = mCap.get(cv::CAP_PROP_FRAME_COUNT);
	if (startIndex < 0 || startIndex >= amount)
	{
		PrintError("start index (%d) must be less than amount frame (%d)", startIndex, amount);
		return;
	}

	if (startIndex != 0 && startIndex > 0 && startIndex < amount)
	{
		mCap.set(cv::CAP_PROP_POS_FRAMES, startIndex);
	}
	cv::Mat frame;
	int duration = 0;
	if (fps != 0)
	{
		duration = 1000 / fps;
	}

	m_frameCount = startIndex;
	int key = -1;
#if defined(_CONSOLE)
	while (key != VK_ESCAPE && !m_stopVideo)
#else //TODO: need to check which value (keycode) is return for orther GUI (QT, GTK ..)
	while (!m_stopVideo)
#endif
	{
		clock_t startTime = clock();
		mCap.read(frame);
		if (!frame.data)
		{
			break;
		}
		if ((maxSize.width != 0 && maxSize.height != 0) && (frame.cols > maxSize.width || frame.rows > maxSize.height))
		{
			cv::resize(frame, frame, maxSize);
		}
		OnNewFrame(frame);

		if (fps != 0)
		{

			int elapsedTime = clock() - startTime;
			if (elapsedTime < duration)
			{
				key = cv::waitKey(duration - elapsedTime);
			}
		}

		m_frameCount++;
		if (total > 0 && m_frameCount - startIndex >= total )
		{
			break;
		}
	}

	m_stopVideo = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SaveFrame(cv::Mat frame)
{
	WRITE_IMAGE_ASYNC(frame, "%s\\frame_%d.jpg", m_outDir.c_str(), GetTGMTvideo()->m_frameCount);
	std::cout << "\r" << "Writing frame " << GetTGMTvideo()->m_frameCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTvideo::ExtractFrame(std::string videoFilePath, std::string outputDir, cv::Size outputSize, int startIndex, int total)
{
	PrintMessage("Total frame: %d", CountTotalFrame(videoFilePath));
	OnNewFrame = SaveFrame;
	m_outDir = outputDir;	
	TGMTfile::CreateDir(outputDir);	
	PlayVideo(videoFilePath, outputSize, 0, startIndex, total);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

cv::Mat TGMTvideo::GetFrameAt(std::string videoFilePath, int frameIndex)
{
	//in function GetAmountFrame() has loaded video
	if (frameIndex <= 0 || frameIndex >= CountTotalFrame(videoFilePath))
	{
		PrintError("Frame index is invalid: %d", frameIndex);
		return cv::Mat();
	}

	mCap.set(cv::CAP_PROP_POS_FRAMES, frameIndex);
	cv::Mat frame;
	mCap.read(frame);
	return frame;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTvideo::CountTotalFrame(std::string videoFilePath)
{
	cv::VideoCapture cap = cv::VideoCapture(videoFilePath);
	if (!cap.isOpened())
	{
		PrintError("Can not load video %s", videoFilePath.c_str());

		return -1;
	}


	int amountOfFrame = cap.get(cv::CAP_PROP_FRAME_COUNT);
	return amountOfFrame;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTvideo::LoadConfig()
{
	PrintMessage("Loading video in config.ini");
	m_loadConfigSuccess = false;

	////read config
	//m_videoPath = GetTGMTConfig()->ReadString("TGMTvideo", "video");
	//ASSERT(!m_videoPath.empty(), "Video path is empty");

	//
	//m_inputWidth = GetTGMTConfig()->ReadInt("TGMTvideo", "input_width");
	//m_inputHeight = GetTGMTConfig()->ReadInt("TGMTvideo", "input_height");

	//m_startIndex = GetTGMTConfig()->ReadInt("TGMTvideo", "start_video_at_frame");
	//m_playTotalFrame = GetTGMTConfig()->ReadInt("TGMTvideo", "play_total_frame");

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTvideo::StopVideo()
{
	m_stopVideo = true; 
}

#endif