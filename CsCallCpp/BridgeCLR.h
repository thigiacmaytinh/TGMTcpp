//https://thigiacmaytinh.com
#pragma once

using namespace System::Drawing;

namespace TGMT
{
	public ref class BridgeCLR
	{
	public:
		static Bitmap^ ConvertToBlur(Bitmap^ bmp, int value);
	};

}