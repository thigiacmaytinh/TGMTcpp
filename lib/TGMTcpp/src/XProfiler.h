#ifdef XPROFILER
#ifndef XPROFILER_INCLUDE_H
#define XPROFILER_INCLUDE_H
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#ifdef _WIN32
#include <time.h>
#endif
#ifdef _DEBUG
#define XPROFILER_ENABLE 1
#else
#define XPROFILER_ENABLE 1
#endif
#define COMBINE1(X,Y) X##Y
#define COMBINE(X,Y) COMBINE1(X,Y)
#if XPROFILER_ENABLE
#define XSCOPE(NAME) XProfilerScope COMBINE(xScope, __LINE__)(NAME)
#define XPROFILE_ENDFRAME XProfiler::EndFrame()
#else
#define XSCOPE(NAME)
#define XPROFILE_ENDFRAME
#endif

class DebuggerServer;
class Debugger
{
public:
	static void Init();
	static void Release();
	Debugger();
	virtual ~Debugger();
protected:
	DebuggerServer* debuggerServer;
private:
	static Debugger* instance;
};

class XProfilerThread;
class XProfilerFrame;
class XProfilerScopeStored;
class XProfilerScope
{
public:
  XProfilerThread* pThread;
	XProfilerScopeStored *scopeStored;
  XProfilerScope(const char* name);

  ~XProfilerScope();

  static timespec GetCurrentNS();
};
class XProfiler
{
public:
  static std::vector<XProfilerFrame*> frames;
  static void Start();
  static void End(std::function<void(std::string)> callback);
  static XProfilerFrame* GetCurrentFrame();
  static void EndFrame();
  static bool enabled;
  static bool switchState;
  ~XProfiler();
  XProfiler();
protected:
  static void PrintScope(FILE *f, XProfilerScopeStored * scope);
  static void PrintScope(std::stringstream &stream, XProfilerScopeStored * scope);
  static void WriteToFile();
  static std::string GetProfileXML();
  static std::function<void(std::string)> endProfileCallback;
};
#endif
#else
#define XSCOPE
#define XPROFILE_ENDFRAME
#endif