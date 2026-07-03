#pragma once
using namespace System::Drawing;

namespace OpenCV
{
	public ref class ImgCodec
	{
	public:
		static Bitmap^ imread(System::String^ filePath);
		static bool IsRawImage(System::String^ filePath);
	};
}
