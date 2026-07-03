#ifdef XPROFILER
#include "XProfiler.h"
#include <stdio.h>
#include <thread> 
#include <string>
#include <vector>
#include <map>
#include <mutex> 
#include <iomanip>

#ifdef OS_ANDROID
#include <time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#elif defined(_WIN32)
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <thread>

class XProfilerCallbackList
{
  std::function<void(std::string)> endProfileCallback;
};

class XProfilerThread;
class XProfilerFrame;

class XProfilerScopeStored
{
public:
	std::string name;
	timespec start;
	timespec end;
	std::vector<XProfilerScopeStored*> child;
	XProfilerScopeStored* parent;
	XProfilerScopeStored()
	{
		end.tv_sec = 0;
		end.tv_nsec = 0;
		parent = NULL;
	}
protected:
	static unsigned int scopeIDUnique;
};


class XProfilerThread
{
public:
	std::thread::id threadID;
	std::vector<XProfilerScopeStored*> scopes;
	XProfilerScopeStored* currentScope;
	XProfilerThread(std::thread::id id)
	{
		threadID = id;
		currentScope = NULL;
	}
  void CloseScope();
  ~XProfilerThread()
  {
    for (int i = 0; i < scopes.size(); i++)
    {
      delete *(scopes.begin() + i);
    }
  }
};
class XProfilerFrame
{
public:
  timespec start;
  timespec end;
  ~XProfilerFrame()
  {
  }
};

std::mutex xprofiler_mutex;
std::vector<XProfilerFrame*> XProfiler::frames;
bool XProfiler::enabled = false;
bool XProfiler::switchState = false;
std::function<void(std::string)> XProfiler::endProfileCallback;
std::map<std::thread::id, XProfilerThread*> threads;
XProfilerScope::XProfilerScope(const char* name)
{
	if (XProfiler::enabled == false)
	{
		pThread = NULL;
		return;
	}

	std::thread::id threadID = std::this_thread::get_id();
	std::lock_guard<std::mutex> lck(xprofiler_mutex);

	XProfilerThread* thread = NULL;
	if (threads.find(threadID) == threads.end())
	{
		thread = new XProfilerThread(threadID);
		threads[threadID] = thread;
	}
	else
	{
		thread = threads[threadID];
	}
	this->pThread = thread;
	XProfilerScopeStored* scopeStored = new XProfilerScopeStored();
	scopeStored->name = name;
	scopeStored->start = GetCurrentNS();
	scopeStored->parent = thread->currentScope;

	if (thread->currentScope == NULL) //root
	{
		thread->scopes.push_back(scopeStored);
	}
	else
	{
		thread->currentScope->child.push_back(scopeStored);
	}
	thread->currentScope = scopeStored;
}
XProfilerScope::~XProfilerScope()
{
  if (pThread == NULL || XProfiler::enabled == false)
  {
    return;
  }
  //std::lock_guard<std::mutex> lck(xprofiler_mutex);
  pThread->currentScope->end = GetCurrentNS();
  pThread->CloseScope();
}


XProfiler::XProfiler()
{

}

XProfiler::~XProfiler()
{
  for (int i = 0; i < frames.size(); i++)
  {
    delete *(frames.begin() + i);
  }
  frames.clear();
	for (std::map<std::thread::id, XProfilerThread*>::iterator it = threads.begin(); it != threads.end(); it++)
	{
		delete (*it).second;
	}
	threads.clear();
}

void XProfiler::EndFrame()
{
  
  if (XProfiler::enabled == false && XProfiler::switchState == false)
  {
    return;
  }
  timespec currentTime = XProfilerScope::GetCurrentNS();
  if (frames.size() > 0)
  {
    frames[frames.size() - 1]->end = currentTime;
  }
  if (XProfiler::switchState == true)
  {
    XProfiler::switchState = false;
    if (XProfiler::enabled)
    {
      //WriteToFile();
      endProfileCallback(GetProfileXML());
      for (int i = 0; i < frames.size(); i++)
      {
        delete *(frames.begin() + i);
      }
      frames.clear();
			for (std::map<std::thread::id, XProfilerThread*>::iterator it = threads.begin(); it != threads.end(); it++)
			{
				delete (*it).second;
			}
			threads.clear();
    }
    XProfiler::enabled = !XProfiler::enabled;
  }
  if (XProfiler::enabled)
  {
    XProfilerFrame* frame = new XProfilerFrame();
    frame->start = currentTime;
    frames.push_back(frame);
  }
}
void XProfiler::Start()
{
  if (XProfiler::enabled == false)
  {
    XProfiler::switchState = true;
  }
}
void XProfiler::End(std::function<void(std::string)> callback)
{
  if (XProfiler::enabled == true)
  {
    endProfileCallback = callback;
    XProfiler::switchState = true;
  }
}
/*
float XProfiler::GetLastFrameTimeMS()
{
  return lastFrameTimeMS;
}
*/
std::string XProfiler::GetProfileXML()
{
  std::stringstream stream;

  stream << "<XProfile>\r\n";
  stream << "<FrameList number=\"" << frames.size() << "\">\r\n";
  for (int i = 0; i < frames.size(); i++)
  {
    XProfilerFrame* pFrame = *(frames.begin() + i);
    stream << "<F s=\"" << (long long)pFrame->start.tv_sec << "." << std::setfill('0') << std::setw(9) << pFrame->start.tv_nsec 
          << "\" e=\"" << (long long)pFrame->end.tv_sec << "." << std::setw(9) << (long long)pFrame->end.tv_nsec << "\">\r\n";
    stream << "</F>\r\n";
  }
  stream << "</FrameList>\r\n";
  stream << "<ThreadList number=\"" << threads.size() << "\">\r\n";
	for (std::map<std::thread::id, XProfilerThread*>::iterator it = threads.begin(); it != threads.end(); it++)
	{
		XProfilerThread* pThread = (*it).second;
		stream << "<T id=\"" << pThread->threadID << "\">\r\n";
		for (int k = 0; k < pThread->scopes.size(); k++)
		{
			PrintScope(stream, pThread->scopes[k]);
		}
		stream << "</T>\r\n";
	}
  stream << "</ThreadList>\r\n";
  stream << "</XProfile>\r\n";
  return stream.str();
}

void XProfiler::WriteToFile()
{
	/*
  time_t     now = time(0);
  struct tm  tstruct;
  char       buf[100];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
#ifdef _WIN32
  strftime(buf, sizeof(buf), "%Y-%m-%d.%H-%M-%S.xprofile", &tstruct);
#else
  strftime(buf, sizeof(buf), "/sdcard/%Y-%m-%d.%H-%M-%S.xprofile", &tstruct);
#endif

  FILE* f = fopen(buf, "wt");
  if (f)
  {
    fprintf(f, "<XProfiler frame=\"%d\">\r\n", frames.size());
    for (int i = 0; i < frames.size(); i++)
    {
      XProfilerFrame* pFrame = *(frames.begin() + i);
      fprintf(f, "<Frame id=\"%d\" start=\"%lld.%.9ld\" end=\"%lld.%.9ld\">\r\n", i, (long long)pFrame->start.tv_sec, pFrame->start.tv_nsec, (long long)pFrame->end.tv_sec, (long long)pFrame->end.tv_nsec);
      for (int j = 0; j < pFrame->threads.size(); j++)
      {
        XProfilerThread* pThread = *(pFrame->threads.begin() + j);
        fprintf(f, "<Thread id=\"%d\">\r\n", pThread->threadID);
        for (int k = 0; k < pThread->scopes.size(); k++)
        {
          PrintScope(f, pThread->scopes[k]);
        }
        fprintf(f, "</Thread>\r\n");
      }
      fprintf(f, "</Frame>\r\n");
    }
    fprintf(f, "</XProfiler>\r\n\0");
    fclose(f);
  }
	*/
}

void XProfiler::PrintScope(std::stringstream &stream, XProfilerScopeStored * scope)
{
  stream << "<S n=\"" << scope->name.c_str() << "\" s=\"" << (long long)scope->start.tv_sec << "." << std::setfill('0') 
        << std::setw(9) << scope->start.tv_nsec << "\" e=\"" << (long long)scope->end.tv_sec << "." << std::setw(9) << scope->end.tv_nsec << "\">\r\n";
  for (int i = 0; i < scope->child.size(); i++)
  {
    PrintScope(stream, scope->child[i]);
  }
  stream << "</S>\r\n";
}

void XProfiler::PrintScope(FILE *f, XProfilerScopeStored * scope)
{
  fprintf(f, "<Scope name=\"%s\" start=\"%lld.%.9ld\" end=\"%lld.%.9ld\">\r\n", scope->name.c_str(), (long long)scope->start.tv_sec, scope->start.tv_nsec, (long long)scope->end.tv_sec, scope->end.tv_nsec);
  for (int i = 0; i < scope->child.size(); i++)
  {
    PrintScope(f, scope->child[i]);
  }
  fprintf(f, "</Scope>\r\n");
}
void XProfilerThread::CloseScope()
{
  if (currentScope)
  {
    currentScope = currentScope->parent;
  }
}
timespec XProfilerScope::GetCurrentNS()
{
#ifdef OS_ANDROID
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now;
#elif defined(_WIN32)
	struct timespec now;
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
  static double toNano = (double(1000000000LL) / double(s_frequency.QuadPart));
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
  __int64 nano = double(value.QuadPart) * toNano;
	now.tv_sec = nano / 1000000000LL;
	now.tv_nsec = nano % 1000000000LL;
	return now;
#endif
}

//---------------------------------------------------------------------------------------------------------


#define DEFAULT_BUFLEN 128
#define DEFAULT_PORT_ANDROID "27015"
#define DEFAULT_PORT_WINDOWS "27016"
#ifdef _WIN32
#define DEBUGGER_INVALID_SOCKET INVALID_SOCKET
#define DEBUGGER_SOCKET_ERROR SOCKET_ERROR
#else
#define DEBUGGER_INVALID_SOCKET -1
#define DEBUGGER_SOCKET_ERROR -1
#endif

class DebuggerServer
{
protected:
	char *pendingBuffer;
	int32_t pendingBufferSize;
	int32_t pendingCurrentPostion;
	void ResizePendingBuffer(const int32_t &newSize) //new size must be larger
	{
		char* newBuffer = new char[newSize];
		memcpy(newBuffer, pendingBuffer, pendingBufferSize);
		delete[] pendingBuffer;
		pendingBuffer = newBuffer;
		pendingBufferSize = newSize;
	}
	void AppendDataToPendingBuffer(const char *data, const int32_t &size)
	{
		if (pendingCurrentPostion + size > pendingBufferSize)
		{
			ResizePendingBuffer(pendingCurrentPostion + size);
		}
		memcpy(pendingBuffer + pendingCurrentPostion, data, size);
		pendingCurrentPostion += size;
	}
	void TrimFrontData(const int32_t &byteToTrim)
	{
		pendingCurrentPostion -= byteToTrim;
		memmove(pendingBuffer, pendingBuffer + byteToTrim, pendingCurrentPostion);
	}
public:
	std::thread serverThread;
#ifdef _WIN32
	SOCKET ListenSocket;
	SOCKET ClientSocket;
#else
	int ListenSocket;
	int ClientSocket;
#endif
	DebuggerServer()
	{
		pendingBuffer = new char[DEFAULT_BUFLEN];
		pendingBufferSize = DEFAULT_BUFLEN;
		pendingCurrentPostion = 0;
	}
	~DebuggerServer()
	{
		delete[] pendingBuffer;
		pendingBuffer = NULL;
	}
	bool m_bStop;
	void Run();
	void Stop();
	void Update();
	int ProcessRequest(const char *request, const char *data, const int32_t &dataSize);
	void SendProfile(std::string s);
	void SendPacket(const char* message, const char* data, const int32_t dataSize);
	int ProcessReceivedData(const char *data, const int32_t &size);
};



Debugger* Debugger::instance = NULL;

Debugger::Debugger()
{
	debuggerServer = new DebuggerServer();
	debuggerServer->Run();
}

Debugger::~Debugger()
{
	debuggerServer->Stop();
	delete debuggerServer;
	debuggerServer = NULL;
}

void Debugger::Init()
{
	if (instance == NULL)
	{
		instance = new Debugger();
	}
}

void Debugger::Release()
{
	delete instance;
	instance = NULL;
}


void DebuggerServer::Run()
{
	int iResult;

#ifdef _WIN32
	WSADATA wsaData;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}
#endif


	struct addrinfo *result = NULL, *ptr = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	//ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
#ifdef OS_ANDROID
	iResult = getaddrinfo(NULL, DEFAULT_PORT_ANDROID, &hints, &result);
#else
	iResult = getaddrinfo(NULL, DEFAULT_PORT_WINDOWS, &hints, &result);
#endif
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
#ifdef _WIN32
		WSACleanup();
#endif
		return;
	}

	ListenSocket = DEBUGGER_INVALID_SOCKET;
	ClientSocket = DEBUGGER_INVALID_SOCKET;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == DEBUGGER_INVALID_SOCKET) {
#ifdef _WIN32
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
#else
		//LogDebug("Error at socket()");
		freeaddrinfo(result);
#endif
		return;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == DEBUGGER_SOCKET_ERROR) {
#ifdef _WIN32
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
#else
		//LogDebug("bind failed");
		freeaddrinfo(result);
#endif
		return;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == DEBUGGER_SOCKET_ERROR) {
#ifdef _WIN32
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
#else
		//LogDebug("Listen failed");
		close(ListenSocket);
#endif
		return;
	}

	m_bStop = false;
	serverThread = std::thread(&DebuggerServer::Update, this);
	//LogDebug("Server thread start");
}
void DebuggerServer::Stop()
{
	m_bStop = true;
#ifdef _WIN32
	closesocket(ClientSocket);
	WSACleanup();
#else
	close(ClientSocket);
#endif
	if (serverThread.joinable())
	{
		serverThread.join();
	}
}
void DebuggerServer::Update()
{
	while (m_bStop == false)
	{
		// Accept a client socket
		if (ClientSocket == DEBUGGER_INVALID_SOCKET)
		{
			ClientSocket = accept(ListenSocket, NULL, NULL);
			if (ClientSocket == DEBUGGER_INVALID_SOCKET)
			{



#ifdef _WIN32
				//LogVars("accept failed: %d\n", WSAGetLastError());
				closesocket(ListenSocket);
				WSACleanup();
#else
				close(ListenSocket);
#endif
				break;
			}
		}
		else
		{
			char recvbuf[DEFAULT_BUFLEN];
			int iResult, iSendResult;
			int recvbuflen = DEFAULT_BUFLEN;
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0)
			{
				//LogVars("Bytes received: %d\n", iResult);
				ProcessReceivedData(recvbuf, iResult);
			}
			else if (iResult == 0)
			{
				//LogDebug("Connection closing...\n");
				ClientSocket = DEBUGGER_INVALID_SOCKET;
			}
			else
			{

#ifdef _WIN32
				//LogVars("recv failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
#else
				close(ClientSocket);
#endif
				break;
			}
		}
	}
}


int DebuggerServer::ProcessRequest(const char *request, const char *data, const int32_t &dataSize)
{
	if (strcmp(request, "StartProfiling") == 0)
	{
		XProfiler::Start();
	}
	else if (strcmp(request, "StopProfiling") == 0)
	{
		std::function<void(std::string)> f = std::bind(&DebuggerServer::SendProfile, this, std::placeholders::_1);
		XProfiler::End(f);
	}
	return 0;
}

void DebuggerServer::SendProfile(std::string s)
{
	if (ClientSocket != DEBUGGER_INVALID_SOCKET)
	{
		/*
		int32_t size = s.length() + 4;
		char* buffer = new char[size];
		memcpy(buffer, &size, 4);
		memcpy(buffer + 4, s.c_str(), s.length());
		send(ClientSocket, buffer, size, 0);
		SafeDeleteArray(buffer);
		*/
		SendPacket("Profile", s.c_str(), s.length());
	}
}

void DebuggerServer::SendPacket(const char* message, const char* data, const int32_t dataSize)
{
	char messageLength = strlen(message);
	int32_t packetSize = dataSize + messageLength + 6;
	char *packet = new char[packetSize];
	char *packetPtr = packet;
	packetPtr[0] = 0x55;
	packetPtr += 1;
	memcpy(packetPtr, &packetSize, 4);
	packetPtr += 4;
	memcpy(packetPtr, &messageLength, 1);
	packetPtr += 1;
	memcpy(packetPtr, message, messageLength);
	packetPtr += messageLength;
	memcpy(packetPtr, data, dataSize);
	send(ClientSocket, packet, packetSize, 0);
	delete[] packet;
	packet = NULL;
}

int DebuggerServer::ProcessReceivedData(const char *data, const int32_t &size)
{
	static int32_t packetSize = -1;
	AppendDataToPendingBuffer(data, size);
	if (packetSize == -1) //new packet is comming
	{
		if (pendingBuffer[0] != 0x55) //out of sync
		{
			//LogDebug("Pending buffer is out of sync");
			return 1;
		}
		if (pendingCurrentPostion > 4)
		{
			memcpy(&packetSize, pendingBuffer + 1, 4);
		}
	}
	if (pendingCurrentPostion >= packetSize) //enough data for packet
	{
		int32_t messageLength = pendingBuffer[5];
		char *message = new char[messageLength + 1];
		message[messageLength] = 0;
		strncpy(message, pendingBuffer + 6, messageLength);
		char *packetData = message + messageLength;
		int32_t packetDataSize = packetSize - 5 - messageLength;
		ProcessRequest(message, packetData, packetDataSize);
		delete[] message;
		message = NULL;
		TrimFrontData(packetSize);
		packetSize = -1;
	}
	return 0;
}

#endif