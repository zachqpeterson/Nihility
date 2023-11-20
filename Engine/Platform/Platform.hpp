#pragma once

#include "Defines.hpp"

struct WindowData
{
#if defined PLATFORM_WINDOWS
	struct HINSTANCE__* instance;
	struct HWND__* window;
#endif
};

struct IDataObject;
struct _POINTL;

/*
* TODO: Don't freeze the program when resizing
* TODO: Change cursor image (maybe define regions where the cursor changes)
* TODO: Load cursors and icons from a folder: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadimagew
* TODO: Handle copy and pasting: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclipboarddata?redirectedfrom=MSDN, https://stackoverflow.com/questions/14762456/getclipboarddatacf-text
* TODO: Handle drag and drop: https://learn.microsoft.com/en-us/windows/win32/com/drag-and-drop
* TODO: Custom file icons for nihility assets: https://learn.microsoft.com/en-us/windows/win32/shell/how-to-assign-a-custom-icon-to-a-file-type
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

	static L32 DragEnter(IDataObject* pDataObj, UL32 grfKeyState, _POINTL pt, UL32* pdwEffect);
	static L32 DragOver(UL32 grfKeyState, _POINTL pt, UL32* pdwEffect);
	static L32 DragLeave();
	static L32 Drop(IDataObject* pDataObj, UL32 grfKeyState, _POINTL pt, UL32* pdwEffect);
#endif

	STATIC_CLASS(Platform);
	friend class Engine;
	friend struct DropTarget;
};