// TGMTtemplate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <tchar.h>
#include <map>
#include "TGMTfile.h"
#include "TGMTdebugger.h"
#include "TGMTknn.h"
#include "TGMTutil.h"
#include "TGMTConfig.h"

void PrintHelp()
{
	PrintMessage("This program auto classify and move file to each folder");
	std::cout << "Using with syntax: \n";
	debug_out(3, "KnnClassifier.exe -file <knn_file> -in <directory> -out <directory> -w <width> -h <height>\n");
	std::cout << "with <knn_file> is KNN trained file\n";
	std::cout << "and <directory> is directory contain images to predict\n";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CreateEmptyFolder(std::string dir)
{
	for (int i = 0; i < 10; i++)
	{
		TGMTfile::CreateDir(TGMTutil::FormatString("%s%d", dir.c_str(), i));
	}

	for (char i = 'A'; i <= 'Z'; i++)
	{
		TGMTfile::CreateDir(TGMTutil::FormatString("%s%c", dir.c_str(), i));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 1)
	{
		PrintHelp();
		getchar();
	}

	std::string knnFile = "char.knn";;
	std::string inDir = R"(E:\PROJECT\CV\KnnOpenCV\DataSample\Testset)";

	std::string outDir = TGMTutil::GetParameter(argc, argv, "-out");
	if (outDir.substr(outDir.length() - 2) != "\\")
		outDir += "\\";	
	CreateEmptyFolder(outDir);

	GetTGMTConfig()->LoadConfig("config.ini");
	PrintMessage("Loading data...");
	GetTGMTknn()->LoadData(knnFile);

	PrintMessage("Loading image...");
	std::vector<std::string> files = TGMTfile::GetImageFilesInDir(inDir, false);
	if (files.size() == 0)
	{
		PrintMessage("Can not load any image");
#ifdef _DEBUG
		getchar();
#endif
		return 0;
	}
	PrintMessage("Loaded %d images", files.size());

	std::map<int, int> maps;

	START_COUNT_TIME("classify");
	for (size_t i = 0; i < files.size(); i++)
	{
		SET_CONSOLE_TITLE("%d / %d", i+ 1, files.size());

		int result = GetTGMTknn()->Predict(files[i]);
		maps[result]++;

		std::string fileName = TGMTfile::GetFileName(files[i]);
		PrintMessage("%s: %c", fileName.c_str(), (char)result);
		std::string targetFile = TGMTutil::FormatString("%s%c\\%s", outDir.c_str(), result, fileName.c_str());
		
		TGMTfile::MoveFileAsync(files[i], targetFile);
	}
		
	STOP_AND_PRINT_COUNT_TIME("classify");

	PrintSuccess("Classify complete and sum up:");

	//Print Result
	std::map<int, int>::iterator iter = maps.begin();
	while (iter != maps.end())
	{		
		PrintMessage("Class %c: %d", (char)iter->first, iter->second);
		iter++;
	}
#ifdef _DEBUG
	getchar();
#endif
	return 0;
}