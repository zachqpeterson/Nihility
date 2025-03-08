#pragma once

#include "Defines.hpp"

class NH_API Platform
{
public:

private:
	static bool Initialize();
	static void Shutdown();

	static bool Update();

	static U32 screenWidth;
	static U32 screenHeight;
	static U32 virtualScreenWidth;
	static U32 virtualScreenHeight;
	static U32 refreshRate;
	static bool resized;
	static bool focused;
	static bool minimised;

	static bool running;

#ifdef NH_PLATFORM_WINDOWS
	static I64 __stdcall WindowsMessageProc(struct HWND__* hwnd, U32 msg, U64 wParam, I64 lParam);
#endif

	friend class Engine;

	STATIC_CLASS(Platform);
};