#pragma once
#include <string>

class TGMTsound
{
public:
	//TGMTsound();
	static void beep(int duration=200);
	static void beepAsync(int duration = 200);
	static void PlayFile(std::string soundFile);
};

