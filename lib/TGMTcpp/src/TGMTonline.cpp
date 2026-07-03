#include "TGMTonline.h"
#include <curl/curl.h>
//#include "TGMTdebugger.h"
#include "TGMTutil.h"
#include "TGMTfile.h"
#include "TGMTHardware.h"

#include <string.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")

//TGMTonline::TGMTonline()
//{
//}
//
//
//TGMTonline::~TGMTonline()
//{
//}

struct MemoryStruct {
	char *memory;
	size_t size;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t write_memory_callback(char *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL)
	{
#ifdef SHOW_TIZEN_IAP_LOG
		dlog_print(DLOG_DEBUG, "_GET_FEED_", "not enough memory (realloc returned NULL)");
#endif //SHOW_TIZEN_IAP_LOG
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::string TGMTonline::ReadOnlineFile(std::string url)
{
	CURL *curl_handle;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	if (curl_handle)
	{
		curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE); //add to fix can not connect to free SSL
		res = curl_easy_perform(curl_handle);
		if (res != CURLE_OK)
		{
#ifdef SHOW_TIZEN_IAP_LOG
			dlog_print(DLOG_DEBUG, "_GET_FEED_", "URL: %s", url);
			dlog_print(DLOG_DEBUG, "_GET_FEED_", "curl_easy_perform() failed: %s", curl_easy_strerror((CURLcode)res));
#endif //SHOW_TIZEN_IAP_LOG
		}
	}
	std::string resultString = (chunk.memory);
	
	//UnlockProfiles::getInstance()->set_feed_length(chunk.size);
	//UnlockProfiles::getInstance()->set_feed_data(strdup(chunk.memory));
	curl_easy_cleanup(curl_handle);
	free(chunk.memory);
	curl_global_cleanup();
	return resultString;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTonline::IsOnlineFileExist(std::string url)
{
	CURL *curl;
	CURLcode res;

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if (curl) {
		/* First set the URL that is about to receive our POST. This URL can
		just as well be a https:// URL if that is what should receive the
		data. */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "8.8.8.8,8.8.4.4");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); //add to fix can not connect to free SSL

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
	if (res == CURLE_REMOTE_FILE_NOT_FOUND)
		return false;
	return res != CURLE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTonline::SendGETRequestAsync(std::string url, std::string user, std::string pass)
{
	std::thread([](std::string _url, std::string _user, std::string _pass)
	{
		SendGETRequest(_url, _user, _pass);
	}, url, user, pass)
	.detach();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTonline::SendGETRequest(std::string url, std::string user, std::string pass)
{
	try
	{
		CURL* curl;
		curl = curl_easy_init();
		if (!curl)
		{
			//PrintError("Can not init");
			return false;
		}
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "8.8.8.8,8.8.4.4");

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); //add to fix can not connect to free SSL

		if (user != "" && pass != "")
		{
			curl_easy_setopt(curl, CURLOPT_USERPWD, (user + ":" + pass).c_str());
		}

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		return res == CURLE_OK;
	}
	catch (std::exception ex)
	{
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool TGMTonline::IsInternetAvailable(std::string urlToCheck)
{
	CURL *curl;
	CURLcode res;

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if (curl) {
		/* First set the URL that is about to receive our POST. This URL can
		just as well be a https:// URL if that is what should receive the
		data. */
		curl_easy_setopt(curl, CURLOPT_URL, urlToCheck.c_str());
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
	return res == CURLE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t write_to_string(void *ptr, size_t size, size_t count, void *stream) {
	((std::string*)stream)->append((char*)ptr, 0, size*count);
	return size * count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//param="username=admin&password=admin
bool TGMTonline::SendPOSTRequest(std::string url, std::string param, long& http_code, std::string& response)
{
	CURL *curl;
	CURLcode res;

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if (curl) 
	{
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		/* Now specify the POST data */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, param.c_str());


		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); //add to fix can not connect to free SSL
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	curl_global_cleanup();

	return res == CURLE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//param="username=admin&password=admin
bool TGMTonline::SendPOSTRequest(std::string url, std::wstring param, long& http_code, std::wstring& response)
{
	CURL *curl;
	CURLcode res;

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if (curl)
	{
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		/* Now specify the POST data */



		std::string paramUtf8 = std::string(param.begin(), param.end());
		curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, paramUtf8.c_str());


		std::string originResponse;
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE); //add to fix can not connect to free SSL
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &originResponse);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);

		response = TGMTutil::String2WString(originResponse);
	}
	curl_global_cleanup();
	curl_global_cleanup();

	return res == CURLE_OK;
}

#ifndef _DEBUG
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	return size * nmemb;
}
#endif

bool TGMTonline::UploadFile(std::string filePath, std::string _childDir, std::string customFileName, std::string targetURL)
{
	//ASSERT(m_user != "" & m_pass != "", "username and password are empty");
	//ASSERT(targetURL != "", "host url is empty");
	if (targetURL == "" || !TGMTfile::FileExist(filePath))
		return false;

	CURL *curl;
	CURLcode res;
	bool result = false;

	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Expect:";

	curl_global_init(CURL_GLOBAL_ALL);

	/* Fill in the file upload field */
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "sendfile",
		CURLFORM_FILE, filePath.c_str(),
		CURLFORM_END);

	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "childfolder",
		CURLFORM_COPYCONTENTS, _childDir.c_str(),
		CURLFORM_END);

	/* Fill in the custom filename field */
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "customname",
		CURLFORM_COPYCONTENTS, customFileName.c_str(),
		CURLFORM_END);



	curl = curl_easy_init();
	/* initialize custom header list (stating that Expect: 100-continue is not
	wanted */
	headerlist = curl_slist_append(headerlist, buf);
	if (curl)
	{
		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, targetURL.c_str());
		//curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/upfile/dosavefile.php");

		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
#ifndef _DEBUG
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
#endif
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);//add to fix can not connect to free SSL

		/* set user name and password for the authentication */
		//curl_easy_setopt(curl, CURLOPT_USERPWD, (m_user + ":" + m_pass).c_str());
		//MinhDPT rem 
		//if ((argc == 2) && (!strcmp(argv[1], "noexpectheader")))
		//	/* only disable 100-continue header if explicitly requested */
		//	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		//END MinhDPT rem

		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			result = false;
		}
		else
		{
			result = true;
		}

		/* always cleanup */
		curl_easy_cleanup(curl);

		/* then cleanup the formpost chain */
		curl_formfree(formpost);
		/* free slist */
		curl_slist_free_all(headerlist);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTonline::SendUserInfoAsync(std::string trackingHost, std::string version)
{
	std::thread([](const std::string trackingHost, const std::string version)
	{
		//tracking user
		std::string url = trackingHost;

		url += "?user=" + GetTGMThardware()->GetCurrentUser();
		url += "&pc=" + GetTGMThardware()->GetPCname();
		url += "&udid=" + GetTGMThardware()->GetUDID();
		url += "&version=" + version;

		TGMTonline::SendGETRequest(url);
	}, trackingHost, version)
		.detach();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef _MANAGED
Concurrency::task<void>
#else
void
#endif
TGMTonline::UploadFileAsync(std::string filePath, std::string childDir, std::string customFileName, std::string targetURL)
{
#ifdef _MANAGED

	return Concurrency::create_task([filePath, childDir, customFileName, targetURL]()
	{
		GetTGMTtracking()->UploadFile(filePath, childDir, customFileName, targetURL);
	});

#else
	std::thread([&](std::string _filePath, std::string _childDir, std::string _customFileName, std::string _targetURL)
	{
		return UploadFile(_filePath, _childDir, _customFileName, _targetURL);
	}, filePath, childDir, customFileName, targetURL)
		.detach();
#endif
}