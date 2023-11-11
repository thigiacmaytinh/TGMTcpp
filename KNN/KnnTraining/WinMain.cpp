// TGMTtemplate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <tchar.h>
#include "TGMTfile.h"
#include "TGMTdebugger.h"
#include "TGMTknn.h"
#include "TGMTutil.h"
#include "TGMTConfig.h"

void PrintHelp()
{
	std::cout << "Using with syntax: \n";
	debug_out(3, "KnnTraining.exe -in <directory> -out <file> -w <width> -h <height>\n");
		
	std::cout << "-in is directory contain sub directories, with each sub directory contain images same class, name of each sub directory only 1 character\n"
		<< "-out is file name you choose\n";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 1)
	{
		PrintHelp();
		getchar();
		return;
	}

	std::string dir = R"(E:\PROJECT\CV\KnnOpenCV\DataSample\Trainset)";
	std::string fileOuput = "char.knn";

	if (!TGMTfile::DirExist(dir))
	{
		PrintError("Directory \"%s\" does not exist", dir.c_str());
		return;
	}

	GetTGMTConfig()->LoadConfig("config.ini");
	
	START_COUNT_TIME("training_knn");

	SET_CONSOLE_TITLE("Traning data...");
	GetTGMTknn()->TrainData(dir);
	GetTGMTknn()->SaveData(fileOuput);

	std::cout << "\n";
	STOP_AND_PRINT_COUNT_TIME("training_knn");
#ifdef _DEBUG
	getchar();
#endif
}

