module;

#include "Defines.hpp"

export module Platform;

export import :Settings;

import Containers;

export struct WindowData
{
#if defined NH_PLATFORM_WINDOWS
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
export class NH_API Platform
{
public:
	static void SetFullscreen(bool fullscreen);
	static void SetWindowSize(U32 width, U32 height);
	static void SetWindowPosition(I32 x, I32 y);
	static void SetConsoleWindowTitle(const StringView& name);

	static bool ExecuteProcess(const StringView& workingDirectory, const StringView& processFullpath, const StringView& arguments, const StringView& searchErrorString);

	static U32 ScreenWidth();
	static U32 ScreenHeight();
	static U32 VirtualScreenWidth();
	static U32 VirtualScreenHeight();
	static bool Focused();
	static bool Minimised();
	static bool Resized();
	static bool MouseConstrained();
	static void ConstrainMouse(bool b);

private:
	static bool Initialize(const StringView& applicationName);
	static void Shutdown();
	static bool Update();

	static const WindowData& GetWindowData();
	static void UpdateMouse();

	static bool running;
	static WindowData windowData;
	static U32 dpi;
	static U32 screenWidth;
	static U32 screenHeight;
	static U32 virtualScreenWidth;
	static U32 virtualScreenHeight;
	static U32 windowWidth;
	static U32 windowHeight;
	static U32 windowWidthSmall;
	static U32 windowHeightSmall;
	static U32 windowPositionX;
	static U32 windowPositionY;
	static U32 windowPositionXSmall;
	static U32 windowPositionYSmall;
	static U32 refreshRate;
	static I32 cursorDisplayCount;
	static bool resized;
	static bool focused;
	static bool minimised;
	static bool fullscreen;
	static bool cursorLocked;
	static bool cursorConstrained;
	static bool cursorShowing;

#if defined NH_PLATFORM_WINDOWS
	static I64 __stdcall WindowsMessageProc(struct HWND__* hwnd, U32 msg, U64 wParam, I64 lParam);
#endif

	STATIC_CLASS(Platform);
	friend class Engine;
	friend class Input;
	friend class Renderer;
	friend struct DropTarget;
	friend struct Swapchain;
};