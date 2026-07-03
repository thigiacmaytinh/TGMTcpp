#include "TGMTsound.h"
#include <thread>
#include <Windows.h>

//TGMTsound::TGMTsound()
//{
//}

//function name is lower case to avoid ambigous
void TGMTsound::beep(int duration)
{
	//Beep(523, duration);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTsound::beepAsync(int duration)
{
	//std::cout << '\a';
	std::thread threadSendRequest = std::thread(&TGMTsound::beep, duration);
	threadSendRequest.detach();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TGMTsound::PlayFile(std::string soundFile)
{

}