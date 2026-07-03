#pragma once
#include <string>
class TGMTonline
{
public:
	//TGMTonline();
	//~TGMTonline();
	
	static bool IsOnlineFileExist(std::string url);
	static std::string ReadOnlineFile(std::string url);
	static bool SendGETRequest(std::string url, std::string user = "", std::string pass = "");
	static void SendGETRequestAsync(std::string url, std::string user = "", std::string pass = "");

	static bool SendPOSTRequest(std::string url, std::string param, long& http_code, std::string& response);
	static bool SendPOSTRequest(std::string url, std::wstring param, long& http_code, std::wstring& response);

	static bool IsInternetAvailable(std::string urlToCheck = "http://google.com");

	static bool UploadFile(std::string filePath, std::string childDir, std::string customFileName, std::string targetURL);

	static void SendUserInfoAsync(std::string trackingHost, std::string version);

#ifdef _MANAGED
	Concurrency::task<void> UploadFileAsync(std::string filePath, std::string childDir, std::string customFileName, std::string targetURL);
#else
	static void UploadFileAsync(std::string filePath, std::string childDir, std::string customFileName, std::string targetURL);
#endif


#ifdef _MANAGED
	ref class TGMTtrackingManaged
	{

		void UploadFile(std::string filePath, std::string childDir, std::string customFileName, std::string targetURL)
		{
			GetTGMTtracking()->UploadFile(filePath, childDir, customFileName, targetURL);

		}
	};
#endif
};

