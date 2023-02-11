#include "Platform.hpp"

#ifdef PLATFORM_WINDOWS

#include "Core\Settings.hpp"
#include "Core\Logger.hpp"

#include <Windows.h>
#include <hidsdi.h>

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

static constexpr const W16* MENU_NAME = L"Nihility Menu";
static constexpr const W16* CLASS_NAME = L"Nihility Class";

bool Platform::Initialize(const W16* applicationName)
{
	windowData.instance = GetModuleHandleW(0);
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	running = true;

	//Load cursor images
	arrow = LoadCursorW(nullptr, IDC_ARROW);
	hand = LoadCursorW(nullptr, IDC_HAND);
	sizeNS = LoadCursorW(nullptr, IDC_SIZENS);
	sizeWE = LoadCursorW(nullptr, IDC_SIZEWE);
	sizeNESW = LoadCursorW(nullptr, IDC_SIZENESW);
	sizeNWSE = LoadCursorW(nullptr, IDC_SIZENWSE);

	//Setup and register window class.
	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = WindowsMessageProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = windowData.instance;
	wc.hIcon = LoadIconW(nullptr, L""); //MAKEINTRESOURCEW(IDI_ICON)
	wc.hCursor = arrow;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = MENU_NAME;
	wc.lpszClassName = CLASS_NAME;
	wc.hIconSm = LoadIconW(nullptr, L""); //MAKEINTRESOURCEW(IDI_ICON)

	if (!RegisterClassExW(&wc))
	{
		MessageBoxW(nullptr, L"Window registration failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	if (Settings::Fullscreen) { style = WS_POPUP | WS_SYSMENU | WS_MAXIMIZE; }
	else { style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME; }

	windowData.window = CreateWindowExW(0, CLASS_NAME, applicationName, style, 0, 0, 0, 0, nullptr, nullptr, windowData.instance, nullptr);

	if (!windowData.window)
	{
		MessageBoxW(nullptr, L"Window creation failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	U32 dpi = Settings::Dpi;
	Settings::DPI = GetDpiForWindow(windowData.window);

	Settings::SCREEN_WIDTH = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::DPI);
	Settings::SCREEN_HEIGHT = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::DPI);

	Settings::WINDOW_POSITION_X_SMALL = MulDiv(Settings::WindowPositionXSmall, Settings::Dpi, dpi);
	Settings::WINDOW_POSITION_Y_SMALL = MulDiv(Settings::WindowPositionYSmall, Settings::Dpi, dpi);
	Settings::WINDOW_WIDTH_SMALL = MulDiv(Settings::WindowWidthSmall, Settings::Dpi, dpi);
	Settings::WINDOW_HEIGHT_SMALL = MulDiv(Settings::WindowHeightSmall, Settings::Dpi, dpi);

	if (Settings::Fullscreen)
	{
		Settings::WINDOW_POSITION_X = 0;
		Settings::WINDOW_POSITION_Y = 0;
		Settings::WINDOW_WIDTH = Settings::ScreenWidth;
		Settings::WINDOW_HEIGHT = Settings::ScreenHeight;
	}
	else
	{
		Settings::WINDOW_POSITION_X = Settings::WindowPositionXSmall;
		Settings::WINDOW_POSITION_Y = Settings::WindowPositionYSmall;
		Settings::WINDOW_WIDTH = Settings::WindowWidthSmall;
		Settings::WINDOW_HEIGHT = Settings::WindowHeightSmall;
	}

	AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi);
	SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX + border.left, Settings::WindowPositionY + border.top,
		Settings::WindowWidth + border.right - border.left, Settings::WindowHeight + border.bottom - border.top, SWP_NOZORDER | SWP_NOACTIVATE);

	DEVMODEA monitorInfo{};
	EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &monitorInfo);
	if (Settings::TargetFrametime == 0.0) { Settings::TARGET_FRAMETIME = 1.0 / monitorInfo.dmDisplayFrequency; }
	Settings::MONITOR_HZ = monitorInfo.dmDisplayFrequency;

	ShowWindow(windowData.window, Settings::Fullscreen ? SW_SHOWMAXIMIZED : SW_SHOW);

	return true;
}

void Platform::Shutdown()
{
	if (windowData.window)
	{
		DestroyWindow(windowData.window);
		windowData.window = nullptr;

		UnregisterClassW(CLASS_NAME, windowData.instance);
	}
}

bool Platform::Update()
{
	MSG message;

	while (PeekMessageW(&message, windowData.window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	UpdateMouse();

	return running;
}

void Platform::SetFullscreen(bool fullscreen)
{
	Settings::FULLSCREEN = fullscreen;

	if (fullscreen)
	{
		style = WS_POPUP | WS_SYSMENU | WS_MAXIMIZE;

		AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi);
		SetWindowLongPtrW(windowData.window, GWL_STYLE, style);
		SetWindowPos(windowData.window, nullptr, border.left, border.top,
			Settings::ScreenWidth + border.right - border.left, Settings::ScreenHeight + border.bottom - border.top, SWP_SHOWWINDOW);
	}
	else
	{
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
		AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi);
		SetWindowLongPtrW(windowData.window, GWL_STYLE, style);
		SetWindowPos(windowData.window, nullptr, Settings::WindowPositionXSmall + border.left, Settings::WindowPositionYSmall + border.top,
			Settings::WindowWidthSmall + border.right - border.left, Settings::WindowHeightSmall + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowSize(U32 width, U32 height)
{
	if (Settings::Fullscreen)
	{
		//TODO: Log, can't change size in fullscreen
	}
	else
	{
		SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX + border.left, Settings::WindowPositionY + border.top,
			width + border.right - border.left, height + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowPosition(I32 x, I32 y)
{
	if (Settings::Fullscreen)
	{
		//TODO: Log, can't change position in fullscreen
	}
	else
	{
		SetWindowPos(windowData.window, nullptr, x + border.left, y + border.top,
			Settings::WindowWidth + border.right - border.left, Settings::WindowHeight + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetMousePosition(I32 x, I32 y)
{
	SetCursorPos(x, y);
}

void Platform::HideCursor(bool hide)
{
	Settings::HIDE_CURSOR = hide;
}

void Platform::LockCursor(bool lock)
{
	Settings::LOCK_CURSOR = lock;
}

const WindowData& Platform::GetWindowData()
{
	return windowData;
}

void Platform::UpdateMouse()
{
	if (Settings::Focused)
	{
		if (Settings::LockCursor)
		{
			RECT clip{};
			clip.left = Settings::WindowPositionX + Settings::WindowWidth / 2;
			clip.right = clip.left;
			clip.top = Settings::WindowPositionY + Settings::WindowHeight / 2;
			clip.bottom = clip.top;

			ClipCursor(&clip);
		}
		else if (Settings::ConstrainCursor)
		{
			RECT clip{};
			if (Settings::Fullscreen)
			{
				clip.left = Settings::WindowPositionX;
				clip.top = Settings::WindowPositionY;
				clip.right = Settings::WindowPositionX + Settings::WindowWidth;
				clip.bottom = Settings::WindowPositionY + Settings::WindowHeight;
				ClipCursor(&clip);
			}
			else
			{
				GetWindowRect(windowData.window, &clip);
				ClipCursor(&clip);
			}
		}

		ShowCursor(!Settings::HideCursor); //TODO: Doesn't work after unfocusing and focusing the window
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
	case WM_CREATE: {
		POINT point{};
		GetCursorPos(&point);
		//Input::mousePos.x = point.x - windowX;
		//Input::mousePos.y = point.y - windowY;

		RAWINPUTDEVICE rid[4];

		rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
		rid[0].dwFlags = RIDEV_DEVNOTIFY;
		rid[0].hwndTarget = windowData.window;

		rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
		rid[1].dwFlags = RIDEV_DEVNOTIFY;
		rid[1].hwndTarget = windowData.window;

		rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[2].usUsage = HID_USAGE_GENERIC_MOUSE;
		rid[2].dwFlags = RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
		rid[2].hwndTarget = windowData.window;

		rid[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		rid[3].dwFlags = RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
		rid[3].hwndTarget = windowData.window;

		if (!RegisterRawInputDevices(rid, 4, sizeof(RAWINPUTDEVICE)))
		{
			DWORD d = GetLastError();
			return -1;
		}
	} return 0;
	case WM_SETFOCUS: { Settings::FOCUSED = true; } return 0;
	case WM_KILLFOCUS: { Settings::FOCUSED = false; } return 0;
	case WM_QUIT: { Settings::FOCUSED = false; running = false; } return 0;
	case WM_CLOSE: { Settings::FOCUSED = false; running = false; } return 0;
	case WM_DESTROY: { Settings::FOCUSED = false; running = false; } return 0;
	case WM_ERASEBKGND: {} return 1;
	case WM_DPICHANGED: {
		Settings::DPI = HIWORD(wParam);
		AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi);
		RECT* rect = (RECT*)lParam;

		Settings::WINDOW_POSITION_X_SMALL = rect->left - border.left;
		Settings::WINDOW_POSITION_Y_SMALL = rect->top - border.top;
		Settings::WINDOW_WIDTH_SMALL = rect->right - rect->left - border.right + border.left;
		Settings::WINDOW_HEIGHT_SMALL = rect->bottom - rect->top - border.bottom + border.top;

		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_POSITION_X = Settings::WindowPositionXSmall;
			Settings::WINDOW_POSITION_Y = Settings::WindowPositionYSmall;
			Settings::WINDOW_WIDTH = Settings::WindowWidthSmall;
			Settings::WINDOW_HEIGHT = Settings::WindowHeightSmall;

			SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX + border.left, Settings::WindowPositionY + border.top,
				Settings::WindowWidth + border.right - border.left, Settings::WindowHeight + border.bottom - border.top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	} return 0;
	case WM_SIZE: {
		switch (wParam)
		{
		case SIZE_MINIMIZED: { Settings::FOCUSED = false; Settings::MINIMISED = true; } break;
		case SIZE_RESTORED: { Settings::MINIMISED = false; } break;
		}

		RECT rect{};
		GetWindowRect(hwnd, &rect);

		Settings::WINDOW_POSITION_X = rect.left - border.left;
		Settings::WINDOW_POSITION_Y = rect.top - border.top;
		Settings::WINDOW_WIDTH = rect.right - rect.left - border.right + border.left;
		Settings::WINDOW_HEIGHT = rect.bottom - rect.top - border.bottom + border.top;

		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_POSITION_X_SMALL = Settings::WindowPositionX;
			Settings::WINDOW_POSITION_Y_SMALL = Settings::WindowPositionY;
			Settings::WINDOW_WIDTH_SMALL = Settings::WindowWidth;
			Settings::WINDOW_HEIGHT_SMALL = Settings::WindowHeight;
		}

		Settings::SCREEN_WIDTH = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::DPI);
		Settings::SCREEN_HEIGHT = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::DPI);

		//RendererFrontend::OnResize();
	} return 0;
	case WM_MOVE: {
		Settings::WINDOW_POSITION_X = LOWORD(lParam);
		Settings::WINDOW_POSITION_Y = HIWORD(lParam);

		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_POSITION_X_SMALL = Settings::WindowPositionX;
			Settings::WINDOW_POSITION_Y_SMALL = Settings::WindowPositionY;
		}

		Settings::SCREEN_WIDTH = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::DPI);
		Settings::SCREEN_HEIGHT = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::DPI);
	} return 0;


	case WM_DEVICECHANGE: {
		//TODO: 
	} break;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

#endif