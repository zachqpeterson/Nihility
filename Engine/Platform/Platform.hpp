#pragma once

#include "Defines.hpp"

struct WindowData
{
#if defined PLATFORM_WINDOWS
	struct HINSTANCE__* instance;
	struct HWND__* window;
#endif
};

/*
* TODO: Change cursor image (maybe define regions where the cursor changes)
* TODO: Load cursors and icons from a folder: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadimagew
* TODO: Handle copy and pasting: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclipboarddata?redirectedfrom=MSDN, https://stackoverflow.com/questions/14762456/getclipboarddatacf-text
* TODO: Don't freeze the program when resizing
*/
class NH_API Platform
{
public:
	static void SetFullscreen(bool fullscreen);
	static void SetWindowSize(U32 width, U32 height);
	static void SetWindowPosition(I32 x, I32 y);
	static void SetMousePosition(I32 x, I32 y);
	static void HideCursor(bool hide);
	static void LockCursor(bool lock);
	static void SetConsoleWindowTitle(CSTR name);

	static const WindowData& GetWindowData();

	static bool ExecuteProcess(CSTR workingDirectory, CSTR processFullpath, CSTR arguments, CSTR searchErrorString);

private:
	static bool Initialize(CSTR applicationName);
	static void Shutdown();
	static bool Update();

	static void UpdateMouse();

	static bool running;
	static WindowData windowData;

#if defined PLATFORM_WINDOWS
	static I64 __stdcall WindowsMessageProc(struct HWND__* hwnd, U32 msg, U64 wParam, I64 lParam);
#endif

	STATIC_CLASS(Platform);
	friend class Engine;
};