#include "TGMTshirtDetector.h"

#include "TGMTshirtDetector.h"
#include "TGMTdebugger.h"
#include "TGMTcolor.h"
#include "TGMTobjDetect.h"

TGMTshirtDetector* TGMTshirtDetector::instance = nullptr;


uchar CTHue[] = { 0, 0, 0, 0, 20, 30, 55, 85, 115, 138, 161 };
uchar CTSat[] = { 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255 };
uchar CTVal[] = { 0, 255, 120, 255, 255, 255, 255, 255, 255, 255, 255 };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTshirtDetector::TGMTshirtDetector()
{
	GetTGMTobjDetect()->Init("Cascade\\haarcascade_frontalface_alt.xml");
}

TGMTshirtDetector::~TGMTshirtDetector()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTshirtDetector::DetectShirtColor(char* imgPath)
{
	cv::Mat img = cv::imread(imgPath);
	return DetectShirtColor(img);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTshirtDetector::DetectShirtColor(cv::Mat imgInput)
{
	ASSERT(imgInput.data, "image is null");

	ASSERT(imgInput.channels() == 3, "Input image isn't a color RGB image! ");

	std::cout << "got a " << imgInput.cols << "x" << imgInput.rows << " color image." << "\n";


	// If trying to debug the color detector code, enable this:
#if SHOW_DEBUG_IMAGE
	// Create a HSV image showing the color types of the whole image, for debugging.
	cv::Mat imageInHSV;
	cv::cvtColor(imgInput, imageInHSV, CV_BGR2HSV);	// (note that OpenCV stores RGB images in B,G,R order.
	cv::Mat imageDisplayHSV = imgInput.clone();	// Create an empty HSV image

	int rowSizeIn = imageDisplayHSV.step;		// Size of row in bytes, including extra padding
	uchar *imOfsDisp = imageDisplayHSV.data;	// Pointer to the start of the image HSV pixels.


	for (int y = 0; y<imageDisplayHSV.rows; y++)
	{
		for (int x = 0; x<imageDisplayHSV.cols; x++)
		{
			cv::Vec3b pixel = imageInHSV.at<cv::Vec3b>(y, x);

			// Determine what type of color the HSV pixel is.
			int ctype = TGMTcolor::GetPixelColorType(pixel);

			// Show the color type on the displayed image, for debugging.
			*(uchar*)(imOfsDisp + (y)*rowSizeIn + (x)* 3 + 0) = CTHue[ctype];	// Hue
			*(uchar*)(imOfsDisp + (y)*rowSizeIn + (x)* 3 + 1) = CTSat[ctype];	// Full Saturation (except for black & white)
			*(uchar*)(imOfsDisp + (y)*rowSizeIn + (x)* 3 + 2) = CTVal[ctype];		// Full Brightness
		}
	}
	// Display the HSV debugging image
	cv::Mat imageDisplayHSV_RGB;
	cv::cvtColor(imageDisplayHSV, imageDisplayHSV_RGB, CV_HSV2BGR);	// (note that OpenCV stores RGB images in B,G,R order.

	cv::imshow("Colors", imageDisplayHSV_RGB);
#endif	// SHOW_DEBUG_IMAGE

	std::vector<cv::Rect> rectFaces = GetTGMTobjDetect()->Detect(imgInput);
	
	return DetectShirtColor(imgInput, rectFaces);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTshirtDetector::DetectShirtColor(cv::Mat imgInput, std::vector<cv::Rect> rectFaces)
{
	START_COUNT_TIME("detect_shirt_color");
	
	char percent[256];
	const char* color = "";
#if SHOW_DEBUG_IMAGE
	cv::Mat imageDisplay = imgInput.clone();
#endif
	// Process each detected face

	for (int r = 0; r<rectFaces.size(); r++)
	{
		float initialConfidence = 1.0f;
		cv::Rect rectFace = rectFaces[r];
#if SHOW_DEBUG_IMAGE
		cv::rectangle(imageDisplay, rectFace, cv::Scalar(255, 0, 0));
#endif
		// Create the shirt region, to be below the detected face and of similar size.
		float SHIRT_DY = 1.4f;	// Distance from top of face to top of shirt region, based on detected face height.
		float SHIRT_SCALE_X = 0.6f;	// Width of shirt region compared to the detected face
		float SHIRT_SCALE_Y = 0.6f;	// Height of shirt region compared to the detected face
		cv::Rect rectShirt;
		rectShirt.x = rectFace.x + (int)(0.5f * (1.0f - SHIRT_SCALE_X) * (float)rectFace.width);
		rectShirt.y = rectFace.y + (int)(SHIRT_DY * (float)rectFace.height) + (int)(0.5f * (1.0f - SHIRT_SCALE_Y) * (float)rectFace.height);
		rectShirt.width = (int)(SHIRT_SCALE_X * rectFace.width);
		rectShirt.height = (int)(SHIRT_SCALE_Y * rectFace.height);
#if DEBUG
		std::cout << "Shirt region is from " << rectShirt.x << ", " << rectShirt.y << " to " << rectShirt.x + rectShirt.width - 1 << ", " << rectShirt.y + rectShirt.height - 1 << "\n";
#endif
		// If the shirt region goes partly below the image, try just a little below the face
		int bottom = rectShirt.y + rectShirt.height - 1;
		if (bottom > imgInput.rows - 1)
		{
			SHIRT_DY = 0.95f;	// Distance from top of face to top of shirt region, based on detected face height.
			SHIRT_SCALE_Y = 0.3f;	// Height of shirt region compared to the detected face
			// Use a higher shirt region
			rectShirt.y = rectFace.y + (int)(SHIRT_DY * (float)rectFace.height) + (int)(0.5f * (1.0f - SHIRT_SCALE_Y) * (float)rectFace.height);
			rectShirt.height = (int)(SHIRT_SCALE_Y * rectFace.height);
			initialConfidence = initialConfidence * 0.5f;	// Since we are using a smaller region, we are less confident about the results now.
#if DEBUG
			std::cout << "Warning: Shirt region goes past the end of the image. Trying to reduce the shirt region position to " << rectShirt.y << " with a height of " << rectShirt.height << "\n";
#endif
		}

		// Try once again if it is partly below the image.
		bottom = rectShirt.y + rectShirt.height - 1;
		if (bottom > imgInput.rows - 1)
		{
			bottom = imgInput.rows - 1;	// Limit the bottom
			rectShirt.height = bottom - (rectShirt.y - 1);	// Adjust the height to use the new bottom
			initialConfidence = initialConfidence * 0.7f;	// Since we are using a smaller region, we are less confident about the results now.
#if DEBUG
			std::cout << "Warning: Shirt region still goes past the end of the image. Trying to reduce the shirt region height to " << rectShirt.height << "\n";
#endif
		}

		// Make sure the shirt region is in the image		
		if (rectShirt.height > 1)
		{
#if SHOW_DEBUG_IMAGE
			// Show the shirt region
			cv::rectangle(imageDisplay, rectShirt, CV_RGB(255, 255, 255));
#endif
			// Convert the shirt region from RGB colors to HSV colors
			//std::cout << "Converting shirt region to HSV" << endl;
			cv::Mat imageShirt = imgInput(rectShirt);
			cv::Mat imageShirtHSV;
			cv::cvtColor(imageShirt, imageShirtHSV, CV_BGR2HSV);	// (note that OpenCV stores RGB images in B,G,R order.
			ASSERT(imageShirtHSV.data, "ERROR: Couldn't convert Shirt image from BGR2HSV.")

				//std::cout << "Determining color type of the shirt" << endl;
				int h = imageShirtHSV.rows;				// Pixel height
			int w = imageShirtHSV.cols;				// Pixel width
			int rowSize = imageShirtHSV.step;		// Size of row in bytes, including extra padding

			// Create an empty tally of pixel counts for each color type
			int tallyColors[TGMTcolor::NUM_COLOR_TYPES];
			for (int i = 0; i < TGMTcolor::NUM_COLOR_TYPES; i++)
			{
				tallyColors[i] = 0;
			}

			// Scan the shirt image to find the tally of pixel colors
			for (int y = 0; y<h; y++)
			{
				for (int x = 0; x<w; x++)
				{
					cv::Vec3b pixel = imageShirtHSV.at<cv::Vec3b>(y, x);

					// Determine what type of color the HSV pixel is.
					int ctype = TGMTcolor::GetPixelColorType(pixel);
					// Keep count of these colors.
					tallyColors[ctype]++;
				}
			}

			// Print a report about color types, and find the max tally
			int tallyMaxIndex = 0;
			int tallyMaxCount = -1;
			int pixels = w * h;
			for (int i = 0; i<TGMTcolor::NUM_COLOR_TYPES; i++)
			{
				int v = tallyColors[i];
#ifdef _DEBUG
				std::cout << TGMTcolor::ColorNames[i] << " " << (v * 100 / pixels) << "%, ";
#endif
				if (v > tallyMaxCount)
				{
					tallyMaxCount = tallyColors[i];
					tallyMaxIndex = i;
				}
			}

			int percentage = initialConfidence * (tallyMaxCount * 100 / pixels);
			color = TGMTcolor::ColorNames[tallyMaxIndex].c_str();
			PrintMessage("Detected color: %s with %d percent", color, percentage);
#if SHOW_DEBUG_IMAGE
			std::cout << "Color of shirt: " << TGMTcolor::ColorNames[tallyMaxIndex] << " (" << percentage << "% confidence)." << "\n" << "\n";
			// Display the color type over the shirt in the image.

			sprintf_s(percent, sizeof(percent) - 1, "%d%%", percentage);

			cv::putText(imageDisplay, color, cvPoint(rectShirt.x, rectShirt.y + rectShirt.height + 12), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0), 1.5);
			cv::putText(imageDisplay, percent, cvPoint(rectShirt.x, rectShirt.y + rectShirt.height + 24), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 0, 0), 1.5);
#endif			


		}//end if valid height
	}//end for loop

#if SHOW_DEBUG_IMAGE
	// Display the RGB debugging image
	cv::imshow("Shirt", imageDisplay);
#endif

	STOP_AND_PRINT_COUNT_TIME("detect_shirt_color");

	return color;
}