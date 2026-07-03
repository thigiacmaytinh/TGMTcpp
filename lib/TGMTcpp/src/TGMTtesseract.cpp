#include "TGMTtesseract.h"
#include "TGMTdebugger.h"
#include "TGMTfile.h"
#include "TGMTConfig.h"
#include <memory>


TGMTtesseract* TGMTtesseract::instance = nullptr;

TGMTtesseract::TGMTtesseract()
{
	// setup
	m_tesseract.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
	m_tesseract.SetVariable("save_best_choices", "T");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TGMTtesseract::~TGMTtesseract()
{
	m_tesseract.Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTtesseract::LoadConfig()
{
	std::string data = "";
	m_lang = GetTGMTConfig()->ReadString("TGMTtesseract", "lang", m_lang);
	ASSERT(!m_tesseract.Init((TGMTfile::GetCurrentDir() + "\\data").c_str(), m_lang.c_str()), "OCRTesseract: Could not initialize tesseract.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTtesseract::ReadText(std::string filePath)
{
	// read image
	auto pixs = pixRead(filePath.c_str());
	if (!pixs)
	{
		std::cout << "Cannot open input file: " << filePath << std::endl;
		return "";
	}

	// recognize
	m_tesseract.SetImage(pixs);
	m_tesseract.Recognize(0);

	// get result and delete[] returned char* string
	std::string result =  std::unique_ptr<char[]>(m_tesseract.GetUTF8Text()).get();
	result = ReplaceChar(result);
	// cleanup
	
	pixDestroy(&pixs);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTtesseract::ReadText(cv::Mat matInput)
{
	if (matInput.data == nullptr)
	{
		DEBUG_OUT("%s", "Mat input is null");
		return "";
	}
		

	// recognize
	m_tesseract.SetImage(matInput.data, matInput.cols, matInput.rows, matInput.channels(), matInput.step1());
	m_tesseract.Recognize(0);

	// get result and delete[] returned char* string
	std::string result = std::unique_ptr<char[]>(m_tesseract.GetUTF8Text()).get();
	result = ReplaceChar(result);


	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTtesseract::SetLang(Lang lang)
{
	switch (lang)
	{
	case Lang::Deu:
		m_lang = "deu";
		break;
	case Lang::Vie:
		m_lang = "vie";
		break;
	default:
		m_lang = "eng";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTtesseract::ReplaceChar(std::string input)
{
	std::string output;
	for (int i = 0; i < input.length(); i++)
	{
		if (input[i] == 'Â' && input[i + 1] == 'º')
		{
			output += "o";
			i++;
		}
		else
		{
			output += input[i];
		}
	}
	return output;
}