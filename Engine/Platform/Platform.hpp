#pragma once

#include "Defines.hpp"

#include "Core/Events.hpp"
#include "Containers/String.hpp"

#ifdef NH_PLATFORM_WINDOWS

struct WindowInfo
{
	struct HINSTANCE__* instance;
	struct HWND__* window;
};

#endif

class NH_API Platform
{
public:
	static void SetFullscreen(bool fullscreen);
	static void SetWindowSize(U32 width, U32 height);
	static void SetWindowPosition(I32 x, I32 y);
	static void SetConsoleWindowTitle(const char* name);

	static U32 ScreenWidth();
	static U32 ScreenHeight();
	static U32 VirtualScreenWidth();
	static U32 VirtualScreenHeight();
	static bool Focused();
	static bool Minimised();
	static bool Resized();
	static bool MouseConstrained();
	static void ConstrainMouse(bool b);

	static Event<bool> OnFocused;
	static Event<String> OnDragDrop;

private:
	static bool Initialize(const StringView& title);
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
	static WindowInfo GetWindowInfo();
#endif

	friend class Input;
	friend class Engine;

	STATIC_CLASS(Platform);
};