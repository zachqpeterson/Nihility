#include "Platform.hpp"

#include "Input.hpp"
#include "Resources\Settings.hpp"
#include "Core\Logger.hpp"

#ifdef PLATFORM_WINDOWS

#include <Windows.h>

bool Platform::running;
WindowData Platform::windowData;

U32 style;
RECT border;

HICON arrow;
HICON hand;
HICON sizeNS;
HICON sizeWE;
HICON sizeNESW;
HICON sizeNWSE;

static constexpr const C8* MENU_NAME = "Nihility Menu";
static constexpr const C8* CLASS_NAME = "Nihility Class";

bool Platform::Initialize(const C8* applicationName)
{
	Logger::Trace("Initializing Platform...");

	windowData.instance = GetModuleHandleA(0);
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	running = true;


	//Load cursor images
	//arrow = LoadCursorW(nullptr, IDC_ARROW);
	//hand = LoadCursorW(nullptr, IDC_HAND);
	//sizeNS = LoadCursorW(nullptr, IDC_SIZENS);
	//sizeWE = LoadCursorW(nullptr, IDC_SIZEWE);
	//sizeNESW = LoadCursorW(nullptr, IDC_SIZENESW);
	//sizeNWSE = LoadCursorW(nullptr, IDC_SIZENWSE);

	//Setup and register window class.
	WNDCLASSEXA wc{};
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = WindowsMessageProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = windowData.instance;
	wc.hIcon = LoadIconA(nullptr, ""); //MAKEINTRESOURCEW(IDI_ICON)
	wc.hCursor = arrow;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = MENU_NAME;
	wc.lpszClassName = CLASS_NAME;
	wc.hIconSm = LoadIconA(nullptr, ""); //MAKEINTRESOURCEW(IDI_ICON)

	if (!RegisterClassExA(&wc))
	{
		MessageBoxA(nullptr, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_VISIBLE;

	windowData.window = CreateWindowExA(0, CLASS_NAME, applicationName, style, 0, 0, 0, 0, nullptr, nullptr, windowData.instance, nullptr);

	if (!windowData.window)
	{
		MessageBoxA(nullptr, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	U32 dpi = Settings::Dpi();

	Settings::data.dpi = GetDpiForWindow(windowData.window);

	Settings::data.screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::Dpi());
	Settings::data.screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::Dpi());

	if (dpi)
	{
		Settings::data.windowPositionXSmall = MulDiv(Settings::WindowPositionXSmall(), Settings::Dpi(), dpi);
		Settings::data.windowPositionYSmall = MulDiv(Settings::WindowPositionYSmall(), Settings::Dpi(), dpi);
		Settings::data.windowWidthSmall = MulDiv(Settings::WindowWidthSmall(), Settings::Dpi(), dpi);
		Settings::data.windowHeightSmall = MulDiv(Settings::WindowHeightSmall(), Settings::Dpi(), dpi);
	}

	if (Settings::Fullscreen())
	{
		Settings::data.windowPositionX = 0;
		Settings::data.windowPositionY = 0;
		Settings::data.windowWidth = Settings::ScreenWidth();
		Settings::data.windowHeight = Settings::ScreenHeight();
	}
	else
	{
		Settings::data.windowPositionX = Settings::WindowPositionXSmall();
		Settings::data.windowPositionY = Settings::WindowPositionYSmall();
		Settings::data.windowWidth = Settings::WindowWidthSmall();
		Settings::data.windowHeight = Settings::WindowHeightSmall();
	}

	AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi());
	SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX() + border.left, Settings::WindowPositionY() + border.top,
		Settings::WindowWidth() + border.right - border.left, Settings::WindowHeight() + border.bottom - border.top, SWP_NOZORDER | SWP_NOACTIVATE);

	DEVMODEA monitorInfo{};
	EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &monitorInfo);
	if (Settings::TargetFrametime() == 0.0) { Settings::data.targetFrametime = 1.0 / monitorInfo.dmDisplayFrequency; }
	Settings::data.monitorHz = monitorInfo.dmDisplayFrequency;

	ShowWindow(windowData.window, Settings::Fullscreen() ? SW_SHOWMAXIMIZED : SW_SHOW);

	return true;
}

void Platform::Shutdown()
{
	if (windowData.window)
	{
		DestroyWindow(windowData.window);
		windowData.window = nullptr;

		UnregisterClassA(CLASS_NAME, windowData.instance);
	}
}

bool Platform::Update()
{
	MSG message;

	while (PeekMessageA(&message, windowData.window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	UpdateMouse();

	return running;
}

void Platform::SetFullscreen(bool fullscreen)
{
	Settings::data.fullscreen = fullscreen;

	AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi());

	if (fullscreen)
	{
		SetWindowPos(windowData.window, nullptr, border.left, border.top,
			Settings::ScreenWidth() + border.right - border.left, Settings::ScreenHeight() + border.bottom - border.top, SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos(windowData.window, nullptr, Settings::WindowPositionXSmall() + border.left, Settings::WindowPositionYSmall() + border.top,
			Settings::WindowWidthSmall() + border.right - border.left, Settings::WindowHeightSmall() + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowSize(U32 width, U32 height)
{
	if (Settings::Fullscreen())
	{
		//TODO: Log, can't change size in fullscreen
	}
	else
	{
		SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX() + border.left, Settings::WindowPositionY() + border.top,
			width + border.right - border.left, height + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowPosition(I32 x, I32 y)
{
	if (Settings::Fullscreen())
	{
		//TODO: Log, can't change position in fullscreen
	}
	else
	{
		SetWindowPos(windowData.window, nullptr, x + border.left, y + border.top,
			Settings::WindowWidth() + border.right - border.left, Settings::WindowHeight() + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetMousePosition(I32 x, I32 y)
{
	SetCursorPos(x, y);
}

void Platform::HideCursor(bool hide)
{
	//ShowCursor(hide);
	Settings::data.hideCursor = hide;
}

void Platform::LockCursor(bool lock)
{
	Settings::data.lockCursor = lock;
}

void Platform::SetConsoleWindowTitle(const C8* name)
{
	SetConsoleTitleA(name);
}

const WindowData& Platform::GetWindowData()
{
	return windowData;
}

void Platform::UpdateMouse()
{
	if (Settings::Focused())
	{
		if (Settings::LockCursor())
		{
			RECT clip{};
			clip.left = Settings::WindowPositionX() + Settings::WindowWidth() / 2;
			clip.right = clip.left;
			clip.top = Settings::WindowPositionY() + Settings::WindowHeight() / 2;
			clip.bottom = clip.top;

			ClipCursor(&clip);
		}
		else if (Settings::ConstrainCursor())
		{
			RECT clip{};
			if (Settings::Fullscreen())
			{
				clip.left = Settings::WindowPositionX();
				clip.top = Settings::WindowPositionY();
				clip.right = Settings::WindowPositionX() + Settings::WindowWidth();
				clip.bottom = Settings::WindowPositionY() + Settings::WindowHeight();
				ClipCursor(&clip);
			}
			else
			{
				GetWindowRect(windowData.window, &clip);
				ClipCursor(&clip);
			}
		}

		//int i = ShowCursor(!Settings::HideCursor()); //TODO: Doesn't work after unfocusing and focusing the window
		//TODO: Log the return to see the display count
	}
	else
	{
		ShowCursor(true);
		ClipCursor(nullptr);
	}
}

I64 __stdcall Platform::WindowsMessageProc(HWND hwnd, U32 msg, U64 wParam, I64 lParam)
{
	switch (msg)
	{
	case WM_CREATE: { } return 0;
	case WM_SETFOCUS: { Settings::data.focused = true; } return 0;
	case WM_KILLFOCUS: { Settings::data.focused = false; } return 0;
	case WM_QUIT: { Settings::data.focused = false; running = false; } return 0;
	case WM_CLOSE: { Settings::data.focused = false; running = false; } return 0;
	case WM_DESTROY: { Settings::data.focused = false; running = false; } return 0;
	case WM_ERASEBKGND: {} return 1;
	case WM_DPICHANGED: {
		Settings::data.dpi = HIWORD(wParam);
		AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi());
		RECT* rect = (RECT*)lParam;

		Settings::data.windowPositionXSmall = rect->left - border.left;
		Settings::data.windowPositionYSmall = rect->top - border.top;
		Settings::data.windowWidthSmall = rect->right - rect->left - border.right + border.left;
		Settings::data.windowHeightSmall = rect->bottom - rect->top - border.bottom + border.top;

		if (!Settings::Fullscreen())
		{
			Settings::data.windowPositionX = Settings::WindowPositionXSmall();
			Settings::data.windowPositionY = Settings::WindowPositionYSmall();
			Settings::data.windowWidth = Settings::WindowWidthSmall();
			Settings::data.windowHeight = Settings::WindowHeightSmall();

			SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX() + border.left, Settings::WindowPositionY() + border.top,
				Settings::WindowWidth() + border.right - border.left, Settings::WindowHeight() + border.bottom - border.top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	} return 0;
	case WM_SIZE: {
		switch (wParam)
		{
		case SIZE_MINIMIZED: { Settings::data.focused = false; Settings::data.minimised = true; } break;
		case SIZE_RESTORED: { Settings::data.minimised = false; } break;
		}

		RECT rect{};
		GetWindowRect(hwnd, &rect);

		Settings::data.windowPositionX = rect.left - border.left;
		Settings::data.windowPositionY = rect.top - border.top;
		Settings::data.windowWidth = rect.right - rect.left - border.right + border.left;
		Settings::data.windowHeight = rect.bottom - rect.top - border.bottom + border.top;

		if (!Settings::Fullscreen())
		{
			Settings::data.windowPositionXSmall = Settings::WindowPositionX();
			Settings::data.windowPositionYSmall = Settings::WindowPositionY();
			Settings::data.windowWidthSmall = Settings::WindowWidth();
			Settings::data.windowHeightSmall = Settings::WindowHeight();
		}

		Settings::data.screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::Dpi());
		Settings::data.screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::Dpi());

		//RendererFrontend::OnResize();
	} return 0;
	case WM_MOVE: {
		Settings::data.windowPositionX = LOWORD(lParam);
		Settings::data.windowPositionY = HIWORD(lParam);

		if (!Settings::Fullscreen())
		{
			Settings::data.windowPositionXSmall = Settings::WindowPositionX();
			Settings::data.windowPositionYSmall = Settings::WindowPositionY();
		}

		Settings::data.screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::Dpi());
		Settings::data.screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::Dpi());
	} return 0;

	case WM_INPUT_DEVICE_CHANGE: {
		if (wParam == GIDC_ARRIVAL) { Input::AddDevice((void*)lParam); }
		else { Input::RemoveDevice((void*)lParam); }
	} return 0;
	case WM_INPUT: {
		if (GET_RAWINPUT_CODE_WPARAM(wParam) == 0) { Input::ReceiveInput((HRAWINPUT)lParam); }
		else { Input::InputSink((HRAWINPUT)lParam); }
	}
	}

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#endif