#include "TGMTfile.h"

#if defined(WIN32) || defined(WIN64) 
//#include "TGMTdebugger.h"
#include "TGMTutil.h"
#include <windows.h>
#include <filesystem>
#ifdef UNICODE
#define TGMTSTR LPCWSTR
#else
#define TGMTSTR LPCSTR
#endif

#endif // WIN32

#include <iostream>
#include <fstream>


#ifdef _MANAGED
#include "TGMTbridge.h"
using namespace System::Threading::Tasks;
using namespace System::Threading;
#else
#include <thread>
#include <algorithm>
#endif

#if defined (OS_LINUX)
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>
#endif
//TGMTfile::TGMTfile(void)
//{
//}
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//TGMTfile::~TGMTfile(void)
//{
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::filesystem::path> TGMTfile::GetFilesInDir(std::filesystem::path dirPath, bool searchAllChildDir)
{
	std::vector<std::filesystem::path> files;

	// Check if directory exists
	if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath))
	{
		return files;
	}

	try
	{
		if (searchAllChildDir)
		{
			// Recursive search through all subdirectories
			for (const auto& entry : std::filesystem::recursive_directory_iterator(
				dirPath,
				std::filesystem::directory_options::skip_permission_denied))
			{
				if (entry.is_regular_file())
				{
					files.push_back(entry.path());
				}
			}
		}
		else
		{
			// Non-recursive search in current directory only
			for (const auto& entry : std::filesystem::directory_iterator(
				dirPath,
				std::filesystem::directory_options::skip_permission_denied))
			{
				if (entry.is_regular_file())
				{
					files.push_back(entry.path());
				}
			}
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		// Handle filesystem errors (e.g., permission denied, network issues)
		// You can log the error or handle it based on your application's needs
		// Example: std::cerr << "Filesystem error: " << e.what() << std::endl;
	}
	catch (const std::exception& e)
	{
		// Handle other exceptions
		// Example: std::cerr << "Error: " << e.what() << std::endl;
	}

	return files;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTfile::FileExist(std::filesystem::path filePath)
{
	if (filePath == "")
		return false;
	if (FILE *file = fopen(filePath.string().c_str(), "r"))
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetFileName(std::filesystem::path filePath)
{
	size_t idx = filePath.string().rfind('\\');
	return filePath.string().substr(idx + 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetFileExtension(std::filesystem::path filePath)
{
	std::string ext = filePath.extension().string();
	return ext;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetDirName(std::filesystem::path dirPath)
{
	if (dirPath == "")
		return "";
	if (dirPath.string().substr(dirPath.string().length() - 2) == "\\")
	{
		dirPath = dirPath.string().substr(dirPath.string().length() - 2);
	}

	return GetFileName(dirPath);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetFileNameWithoutExtension(std::filesystem::path filePath)
{
	std::string fileName = GetFileName(filePath);
	size_t idx = fileName.rfind('.');
	return fileName.substr(0, idx);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTfile::DirExist(const std::filesystem::path dirPath)
{
#ifdef WIN32
	DWORD ftyp = GetFileAttributesA(dirPath.string().c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
#elif defined (OS_LINUX)
	if (0 == access(dirName.c_str(), F_OK))
	{
		struct stat statbuf;
		if (0 == stat(dirName.c_str(), &statbuf))
			return (S_IFDIR & statbuf.st_mode) != 0;
	}
#endif
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::MkDir(std::filesystem::path dirName)
{
	CreateDir(dirName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::CreateDir(std::filesystem::path dirName)
{
#if defined(WIN32) || defined (OS_LINUX)
	if (DirExist(dirName))
		return;
#endif

	try
	{
		std::filesystem::create_directories(dirName);
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		// Handle filesystem errors (e.g., permission denied, invalid path)
		// Example: std::cerr << "Failed to create directory: " << e.what() << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::filesystem::path TGMTfile::GetCurrentDir()
{
#ifdef WIN32
#ifdef UNICODE
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	std::wstring wstr(buffer);
	return std::string(wstr.begin(), wstr.end());
#else
	char path[MAX_PATH];
	HMODULE hm = NULL;

	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)&DirExist, &hm) == 0)
	{
		int ret = GetLastError();
		fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
		// Return or however you want to handle an error.
}
	if (GetModuleFileName(hm, path, sizeof(path)) == 0)
	{
		int ret = GetLastError();
		fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
		// Return or however you want to handle an error.
	}

	return GetParentDir(path);

#endif

#elif defined( OS_LINUX)
	char pWorkingDirectory[MAX_PATH];
	getcwd(pWorkingDirectory, MAX_PATH);
	std::string currentDir(pWorkingDirectory);
	return currentDir;
#endif
    return "";
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::filesystem::path> TGMTfile::GetImageFilesInDir(std::filesystem::path dirPath, bool searchAllChildDir)
{
	std::vector<std::filesystem::path> fileNames = GetFilesInDir(dirPath, searchAllChildDir);
	std::vector<std::filesystem::path> result;
	for (size_t i = 0; i < fileNames.size(); i++)
	{
		if (IsImage(fileNames[i]))
		{
			result.push_back(fileNames[i]);
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::filesystem::path> TGMTfile::GetVideoFilesInDir(std::filesystem::path dirPath, bool searchAllChildDir)
{
	std::vector<std::filesystem::path> fileNames = GetFilesInDir(dirPath, searchAllChildDir);
	std::vector<std::filesystem::path> result;
	for (size_t i = 0; i < fileNames.size(); i++)
	{
		if (IsVideo(fileNames[i]))
		{
			result.push_back( fileNames[i]);
		}
	}
	return result;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTfile::IsDir(std::filesystem::path filePath)
{
	return DirExist(filePath);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTfile::IsImage(std::filesystem::path filePath)
{	
	std::string ext = filePath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	return (ext == ".jpg" || ext == ".png" || ext == ".bmp" || ext == ".pgm" || ext == ".webp" || ext == ".gif" || ext == ".jpeg");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTfile::IsVideo(std::filesystem::path filePath)
{
	std::string ext = filePath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	return (ext == ".mp4" || ext == ".avi" || ext == ".wmv" || ext == ".mpg" || ext == ".flv");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTfile::WriteToFile(std::string fileName, std::string text, bool append)
{
	std::ofstream myfile(fileName, append ? std::ios::app : std::ios::out);
	myfile.write(text.c_str(), sizeof(char)*text.size());
	bool result = !myfile.bad();
	myfile.close();
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined(WIN32) ||defined(WIN64)) && !defined(_MANAGED)
void TGMTfile::WriteToFileAsync(std::string fileName, std::string text, bool append)
{
	std::thread([](const std::string _fileName, const std::string _text, bool _append)
	{
		WriteToFile(_fileName, _text, _append);
	}, fileName, text, append)
	.detach();
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetParentDir(std::filesystem::path filePath, bool fullPath)
{
	int lastSlash = filePath.string().rfind('\\');
	std::string dir = filePath.string().substr(0, lastSlash);
	if (!fullPath)
	{
		lastSlash = dir.rfind('\\');
		dir = dir.substr(lastSlash + 1);
	}
	return dir;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::filesystem::path> TGMTfile::GetChildDirInDir(std::filesystem::path dirPath, bool searchAllChildDir)
{
	std::vector<std::filesystem::path> result;
#if defined(WIN32) 
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	std::string fileName;
	bool ret = true;

	TGMTfile::CorrectPath(dirPath);

#ifdef UNICODE
	std::wstring wsForLongFileName = std::wstring(L"\\\\?\\") + TGMTutil::String2WString(dirPath) + L"*";
	
#else
	std::string wsForLongFileName = std::string("\\\\?\\") + dirPath.string() + "*";
	
#endif

	hFind = FindFirstFile((TGMTSTR)(wsForLongFileName).c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
#ifdef UNICODE
			fileName = TGMTutil::WString2String((TGMTSTR)findFileData.cFileName);
#else
			fileName = (TGMTSTR)findFileData.cFileName;
#endif
			if (fileName != "." && fileName != "..")
			{
				fileName.insert(0, dirPath.string());
				if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)   //file is a directory
				{
					result.push_back(fileName);
					if (searchAllChildDir)
					{
						std::vector<std::filesystem::path> temp = GetChildDirInDir(fileName, searchAllChildDir);
						result.insert(result.end(), temp.begin(), temp.end());
					}
				}
			}
		} while (FindNextFile(hFind, &findFileData));

		FindClose(hFind);
	}
#endif
	return result;	
}

//void CopyFile(const std::string& fileNameFrom, const std::string& fileNameTo)
//{
//	assert(fileExists(fileNameFrom));
//	std::ifstream in(fileNameFrom.c_str());
//	std::ofstream out(fileNameTo.c_str());
//	out << in.rdbuf();
//	out.close();
//	in.close();
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::ReadAllText(std::filesystem::path fileName)
{
	std::string content;
	std::ifstream myFile(fileName);
	if (myFile.is_open())
	{
		std::string line;
		while (getline(myFile, line))
		{
			content.append(line + "\n");
		}
		myFile.close();
	}

	return content;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> TGMTfile::ReadAllLine(std::filesystem::path filePath)
{
	std::ifstream infile(filePath);

	std::string line;
	std::vector<std::string> lines;
	while (std::getline(infile, line))
	{
		lines.push_back(line);
	}
	return lines;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined(WIN32) ||defined(WIN64)) && defined(UNICODE)
#include <codecvt>
std::vector<std::wstring> TGMTfile::WReadAllLine(std::string fileName)
{
	std::wifstream infile(fileName);
	infile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstring line;
	std::vector<std::wstring> lines;
	while (std::getline(infile, line))
	{
		lines.push_back(line);
	}
	return lines;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined(WIN32) || defined(_WIN64)) && !defined(_MANAGED)

//void TGMTfile::Move_File(const std::string fileNameFrom, const std::string fileNameTo)
//{
//	MoveFile((TGMTSTR)fileNameFrom.c_str(), (TGMTSTR)fileNameTo.c_str());
//}
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//void TGMTfile::Move_File(const std::wstring fileNameFrom, const std::wstring fileNameTo)
//{
//	MoveFile((TGMTSTR)fileNameFrom.c_str(), (TGMTSTR)fileNameTo.c_str());
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::MoveFileAsync(const std::string fileNameFrom, const std::string fileNameTo)
{
	std::thread([](const std::string from, const std::string to)
	{
		MoveFile((TGMTSTR)from.c_str(), (TGMTSTR)to.c_str());
	}, fileNameFrom, fileNameTo)
	.detach();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::MoveFileAsync(const std::wstring fileNameFrom, const std::wstring fileNameTo)
{
	std::thread([](const std::wstring from, const std::wstring to)
	{
		MoveFile((TGMTSTR)from.c_str(), (TGMTSTR)to.c_str());
	}, fileNameFrom, fileNameTo)
	.detach();
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::WriteResourceFileToDisk(std::string targetFile, int resouceID)
{
#ifdef WIN32
	HRSRC myResource = ::FindResource(NULL, MAKEINTRESOURCE(resouceID), RT_RCDATA);
	unsigned int myResourceSize = ::SizeofResource(NULL, myResource);
	//ASSERT(myResourceSize > 0, "Can not load resouce");
	HGLOBAL myResourceData = ::LoadResource(NULL, myResource);
	void* pMyExecutable = ::LockResource(myResourceData);

	std::ofstream f(targetFile.c_str(), std::ios::out | std::ios::binary);
	f.write((char*)pMyExecutable, myResourceSize);
	f.close();
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetTempChar()
{
#ifdef WIN32
	std::string tempName = std::tmpnam(nullptr);
	int idx = tempName.find_last_of("\\");
	tempName = tempName.substr(idx);
	//remove dot
	tempName.erase(std::remove(tempName.begin(), tempName.end(), '.'), tempName.end());
	tempName.erase(std::remove(tempName.begin(), tempName.end(), '\\'), tempName.end());
	return tempName;
#else
    return "";
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetTempFilePath()
{
	return std::tmpnam(nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetTempDirPath()
{	
	std::string tempDirPath = GetSystemTempDirPath() + GetTempChar();
	CreateDir(tempDirPath);
	return tempDirPath;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTfile::GetSystemTempDirPath()
{
#ifdef WIN32
#ifdef UNICODE
	wchar_t systemTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, systemTempDir);
	std::string path = TGMTutil::WString2String(systemTempDir);
	return path;
#else
	char systemTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, systemTempDir);
	return systemTempDir;
#endif
	
	
#else
    return "";
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::Delete_File(std::filesystem::path fileName)
{
#ifdef WIN32
#ifdef _MANAGED
	DeleteFile((TGMTSTR)fileName.c_str());
#else
	DeleteFile((TGMTSTR)fileName.c_str());
#endif
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::Delete_FileAsync(std::filesystem::path fileName)
{
#ifdef WIN32
#ifdef _MANAGED
	DeleteFile((TGMTSTR)fileName.c_str());
#else
	std::thread([](const std::filesystem::path file)
	{
		DeleteFile((TGMTSTR)file.c_str());
	}, fileName)
	.detach();
#endif
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::Delete_Dir(std::filesystem::path dirName)
{
#ifdef WIN32
	std::system(("rmdir " + dirName.string() + "/s /q").c_str());
#elif defined (OS_LINUX)
	std::string command = "rmdir -p " + dirName;
	const int dir_err = system(command.c_str());
	if (-1 == dir_err)
	{
		printf("Error deleting directory!n");
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::Copy_File(const std::string fileNameFrom, const std::string fileNameTo)
{
#if defined(WIN32) || defined(WIN64)
	bool temp = false;
	CopyFile((TGMTSTR)fileNameFrom.c_str(), (TGMTSTR)fileNameTo.c_str(), temp);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::CopyAndDeleteFile(const std::string fileNameFrom, const std::string fileNameTo)
{
#if defined(WIN32) || defined(WIN64)
	bool temp = false;
	CopyFile((TGMTSTR)fileNameFrom.c_str(), (TGMTSTR)fileNameTo.c_str(), temp);
	DeleteFile((TGMTSTR)fileNameFrom.c_str());
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined(WIN32) ||defined(WIN64)) && !defined(_MANAGED)

void TGMTfile::CopyFileAsync(const std::string fileNameFrom, const std::string fileNameTo)
{
	std::thread([](const std::string from, const std::string to)
	{
		bool temp = false;
		CopyFile((TGMTSTR)from.c_str(), (TGMTSTR)to.c_str(), temp);
	}, fileNameFrom, fileNameTo)
	.detach();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::CopyDir(const std::string dirFrom, const std::string dirTo, bool recursive)
{	
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
	//C++17 specific stuff here
	std::filesystem::copy(dirFrom, dirTo, recursive ? std::filesystem::copy_options::recursive : std::filesystem::copy_options::none);
#else
	//std::experimental::filesystem::copy(dirFrom, dirTo, recursive ? std::experimental::filesystem::copy_options::recursive : std::experimental::filesystem::copy_options::none);
#endif
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::CopyDirAsync(const std::string dirFrom, const std::string dirTo, bool recursive)
{
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
	//C++17 specific stuff here
	std::thread([](const std::string _dirFrom, const std::string _dirTo, bool _recursive)
	{
		std::filesystem::copy(_dirFrom, _dirTo, _recursive ? std::filesystem::copy_options::recursive : std::filesystem::copy_options::none);
	}, dirFrom, dirTo, recursive)
		.detach();
#else
	/*std::thread([](const std::string _dirFrom, const std::string _dirTo, bool _recursive)
	{
		std::experimental::filesystem::copy(_dirFrom, _dirTo, _recursive ? std::experimental::filesystem::copy_options::recursive : std::experimental::filesystem::copy_options::none);
	}, dirFrom, dirTo, recursive)
	.detach();*/
#endif
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::filesystem::path TGMTfile::CorrectPath(std::filesystem::path path)
{
	
	if (path == "")
		return "";

	std::wstring wPath = path.wstring();
	if (wPath.substr(wPath.length() - 1) != L"\\")
	{
		wPath += L"\\";
	}
	if (wPath.substr(1, 1) != L":" && wPath.substr(0, 2) != L"\\\\")
	{
		wPath = GetCurrentDir().wstring() + L"\\" + wPath;
	}
	return std::filesystem::path(wPath);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int TGMTfile::GetFileSize(std::filesystem::path filePath)
{
	std::ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MANAGED
System::String^ TGMTfile::CorrectPath(System::String^ path)
{
	if (path->Substring(path->Length - 2) != "\\")
	{
		path += "\\";
	}
	return path;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::MoveFileAsync(System::String^ fileNameFrom, System::String^ fileNameTo)
{
	ThreadStart^ mThread = gcnew ThreadStart(gcnew TGMTfileManaged(fileNameFrom, fileNameTo), &TGMTfileManaged::MoveFile);
	Thread^ newThread = gcnew Thread(mThread);
	newThread->Start();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTfile::IsImage(System::String^ filePath)
{
	std::string sFilePath = TGMTbridge::SystemStr2stdStr(filePath);
	return IsImage(sFilePath);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTfile::IsVideo(System::String^ filePath)
{
	std::string sFilePath = TGMTbridge::SystemStr2stdStr(filePath);
	return IsVideo(sFilePath);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::OpenFolder(System::String^ folder)
{
	System::Diagnostics::Process::Start(folder);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^ TGMTfile::GetCurrentFolder()
{
	return System::IO::Path::GetDirectoryName(System::Reflection::Assembly::GetEntryAssembly()->Location);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::WriteAllText(System::String^ fileName, System::String^ text)
{
	System::Windows::Forms::SaveFileDialog^ ofd = gcnew System::Windows::Forms::SaveFileDialog();
	ofd->Filter = "Text file|*.txt";
	ofd->FileName = fileName;
	ofd->ShowHelp = true;
	ofd->Title = "Save text file";
	ofd->ShowDialog();
	System::String^ path = ofd->FileName;
	//File::WriteAllText(path, text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::WriteAllText(std::string fileName, std::string text)
{
	System::String^ sfileName = TGMTbridge::stdStrToSystemStr(fileName);
	System::String^ stext = TGMTbridge::stdStrToSystemStr(text);

	WriteAllText(sfileName, stext);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::WriteResouceFileToDisk(System::String^ targetFile, System::String^ resourceFile)
{
	//Stream^ readStream = System::Reflection::Assembly::GetExecutingAssembly()->GetManifestResourceStream(resourceFile);
	//if (readStream != nullptr)
	//{
	//	FileStream^ writeStream = gcnew FileStream(targetFile, FileMode::Create);
	//	readStream->CopyTo(writeStream);
	//	readStream->Close();
	//	writeStream->Close(); // Required to flush the buffer & have non-zero filesize
	//}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTfile::TGMTfileManaged::MoveFile()
{
	if (System::IO::File::Exists(m_fileNameFrom) && !System::IO::File::Exists(m_fileNameTo))
		System::IO::File::Move(m_fileNameFrom, m_fileNameTo);
};
#endif
