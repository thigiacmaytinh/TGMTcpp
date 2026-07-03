#pragma once
#include <vector>
#include <string>
#include <filesystem>

class TGMTfile
{	
	static std::string GetTempChar();
public:
	//TGMTfile(void);
	//~TGMTfile(void);

	
	static bool FileExist(std::filesystem::path filePath);
	static bool DirExist(std::filesystem::path dirName);

	static std::string GetFileName(std::filesystem::path filePath);
	static std::string GetFileExtension(std::filesystem::path filePath);
	static std::string GetDirName(std::filesystem::path dirPath);
	static std::string GetFileNameWithoutExtension(std::filesystem::path filePath);
	static std::string GetTempFilePath();
	static std::string GetTempDirPath();
	static std::string GetSystemTempDirPath();

	static int GetFileSize(std::filesystem::path filePath);
	static void MkDir(std::filesystem::path dirName);
	static void CreateDir(std::filesystem::path dirName);
	static std::filesystem::path GetCurrentDir();
	
	static std::string GetParentDir(std::filesystem::path filePath, bool fullPath = true);
	static std::vector<std::filesystem::path> GetImageFilesInDir(std::filesystem::path dirPath, bool searchAllChildDir = false);
	static std::vector<std::filesystem::path> GetVideoFilesInDir(std::filesystem::path dirPath, bool searchAllChildDir = false);
	static std::vector<std::filesystem::path> GetFilesInDir(std::filesystem::path dirPath, bool searchAllChildDir = false);
	static std::vector<std::filesystem::path> GetChildDirInDir(std::filesystem::path dirPath, bool searchAllChildDir = false);

	static bool IsDir(std::filesystem::path filePath);
	static bool IsImage(std::filesystem::path filePath);
	static bool IsVideo(std::filesystem::path filePath);

	static void MoveFileAsync(const std::string fileNameFrom, const std::string fileNameTo);
	static void MoveFileAsync(const std::wstring fileNameFrom, const std::wstring fileNameTo);
	static void Copy_File(const std::string fileNameFrom, const std::string fileNameTo);
	static void CopyAndDeleteFile(const std::string fileNameFrom, const std::string fileNameTo);
	
	static void CopyDir(const std::string dirFrom, const std::string dirTo, bool recursive = false);
	static bool WriteToFile(std::string fileName, std::string text, bool append = true);
#if (defined(WIN32) ||defined(WIN64)) && !defined(_MANAGED)
	static void WriteToFileAsync(std::string fileName, std::string text, bool append = true);
	static void CopyFileAsync(const std::string fileNameFrom, const std::string fileNameTo);
	static void CopyDirAsync(const std::string dirFrom, const std::string dirTo, bool recursive = false);
#endif
	static void WriteResourceFileToDisk(std::string targetFile, int resouceID);
	static void Delete_File(std::filesystem::path fileName);
	static void Delete_FileAsync(std::filesystem::path fileName);
	static void Delete_Dir(std::filesystem::path dirName);

	static std::string ReadAllText(std::filesystem::path fileName);
	static std::vector<std::string> ReadAllLine(std::filesystem::path fileName);
#if (defined(WIN32) ||defined(WIN64)) && defined(UNICODE)
	static std::vector<std::wstring> WReadAllLine(std::filesystem::path fileName);
#endif

	static std::filesystem::path CorrectPath(std::filesystem::path path);

//#ifdef _MANAGED
//	ref class TGMTfileManaged
//	{
//		System::String^ m_fileNameFrom;
//		System::String^ m_fileNameTo;
//	public: 
//		TGMTfileManaged( System::String^ fileNameFrom, System::String^ fileNameTo)
//		{
//			m_fileNameFrom = fileNameFrom;
//			m_fileNameTo = fileNameTo;
//		};
//		void MoveFile();
//	};
//	static System::String^ CorrectPath(System::String^ path);
//	
//	static void MoveFileAsync(System::String^ fileNameFrom, System::String^ fileNameTo);
//
//	static bool IsImage(System::String^ filePath);
//	static bool IsVideo(System::String^ filePath);
//	static void OpenFolder(System::String^ folder);
//	static System::String^ GetCurrentFolder();
//	static void WriteAllText(System::String^ fileName, System::String^ text);
//	static void WriteAllText(std::string fileName, std::string text);
//	static void WriteResouceFileToDisk(System::String^ targetFile, System::String^ resourceFile);
//#else	
//	static void Move_File(const std::string fileNameFrom, const std::string fileNameTo);
//	static void Move_File(const std::wstring fileNameFrom, const std::wstring fileNameTo);
//#endif
};

