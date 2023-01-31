#pragma once

#include "Defines.hpp"

struct WindowData
{
#if defined PLATFORM_WINDOWS
	struct HINSTANCE__* instance;
	struct HWND__* window;
#endif
};

class NH_API Platform
{
public:
	static void SetFullscreen(bool fullscreen);
	static void SetWindowSize(bool fullscreen);
	static void SetMousePosition(I32 x, I32 y);
	static void ShowMouse(bool show);
	static void LockMouse(bool lock);

	static const WindowData& GetWindowData();

private:
	static bool Initialize(const W16* applicationName);
	static void Shutdown();
	static bool Update();

	static void SleepFor(U64 ns); //TODO: specify thread

	static void UpdateInput();
	static void UpdateMouse();

	static I32 windowX;
	static I32 windowY;
	static I32 windowWidth;
	static I32 windowHeight;
	static I32 screenWidth;
	static I32 screenHeight;
	static bool running;
	static WindowData windowData;

#if defined PLATFORM_WINDOWS
	static I64 __stdcall WindowsMessageProc(struct HWND__* hwnd, U32 msg, U64 wParam, I64 lParam);
#endif

	STATIC_CLASS(Platform);
	friend class Engine;
};