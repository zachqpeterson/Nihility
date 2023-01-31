#include "Platform.hpp"

#ifdef PLATFORM_WINDOWS

#include "Settings.hpp"

#include <Windows.h>

static U32(__stdcall* NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (U32(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");
static U32(__stdcall* ZwSetTimerResolution)(ULONG RequestedResolution, BOOLEAN Set, PULONG ActualResolution) = (U32(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");

I32 Platform::windowX;
I32 Platform::windowY;
I32 Platform::windowWidth;
I32 Platform::windowHeight;
I32 Platform::screenWidth;
I32 Platform::screenHeight;
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
	ULONG actualResolution;
	ZwSetTimerResolution(1, true, &actualResolution);
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

	if (Settings::Fullscreen)
	{
		style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZE;

		windowData.window = CreateWindowExW(0, CLASS_NAME, applicationName, style, 0, 0, 0, 0, nullptr, nullptr, windowData.instance, nullptr);

		Settings::DPI = GetDpiForWindow(windowData.window);
		Settings::SCREEN_WIDTH = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::DPI);
		Settings::SCREEN_HEIGHT = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::DPI);
		Settings::WINDOW_POSITION_X = 0;
		Settings::WINDOW_POSITION_Y = 0;
		Settings::WINDOW_WIDTH = Settings::ScreenWidth;
		Settings::WINDOW_HEIGHT = Settings::ScreenHeight;

		AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi);

		SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX, Settings::WindowPositionY,
			Settings::WindowWidth, Settings::WindowHeight, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;

		windowData.window = CreateWindowExW(0, CLASS_NAME, applicationName, style, 0, 0, 0, 0, nullptr, nullptr, windowData.instance, nullptr);

		U32 dpi = Settings::Dpi;
		Settings::DPI = GetDpiForWindow(windowData.window);

		Settings::WINDOW_POSITION_X = MulDiv(Settings::WindowPositionXSmall, Settings::Dpi, dpi);
		Settings::WINDOW_POSITION_Y = MulDiv(Settings::WindowPositionYSmall, Settings::Dpi, dpi);
		Settings::WINDOW_WIDTH = MulDiv(Settings::WindowWidthSmall, Settings::Dpi, dpi);
		Settings::WINDOW_HEIGHT = MulDiv(Settings::WindowHeightSmall, Settings::Dpi, dpi);

		Settings::WINDOW_POSITION_X_SMALL = Settings::WindowPositionX;
		Settings::WINDOW_POSITION_Y_SMALL = Settings::WindowPositionY;
		Settings::WINDOW_WIDTH_SMALL = Settings::WindowWidth;
		Settings::WINDOW_HEIGHT_SMALL = Settings::WindowHeight;

		AdjustWindowRectExForDpi(&border, style, 0, 0, Settings::Dpi);

		SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX + border.left, Settings::WindowPositionY + border.top,
			Settings::WindowWidth + border.right - border.left, Settings::WindowHeight + border.bottom - border.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if (!windowData.window)
	{
		MessageBoxW(nullptr, L"Window creation failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	DEVMODEA monitorInfo{};
	EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &monitorInfo);
	if (Settings::TargetFrametime == 0.0) { Settings::TARGET_FRAMETIME = 1.0 / monitorInfo.dmDisplayFrequency; }

	ShowWindow(windowData.window, Settings::Fullscreen ? SW_SHOWMAXIMIZED : SW_SHOW);

	return true;
}

void Platform::Shutdown()
{

}

bool Platform::Update()
{
	MSG message;

	while (PeekMessageW(&message, windowData.window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return running;
}

void Platform::SetFullscreen(bool fullscreen)
{

}

void Platform::SetWindowSize(bool fullscreen)
{

}

void Platform::SetMousePosition(I32 x, I32 y)
{

}

void Platform::ShowMouse(bool show)
{

}

void Platform::LockMouse(bool lock)
{

}

void Platform::SleepFor(U64 ns)
{

}

const WindowData& Platform::GetWindowData()
{
	return windowData;
}

void Platform::UpdateInput()
{

}

void Platform::UpdateMouse()
{

}


I64 __stdcall Platform::WindowsMessageProc(HWND hwnd, U32 msg, U64 wParam, I64 lParam) //TODO: Separate thread?
{
	switch (msg)
	{
	case WM_CREATE: {

	} return 0;
	case WM_SETFOCUS: { Settings::FOCUSED = true; } return 0;
	case WM_KILLFOCUS: { Settings::FOCUSED = false; } return 0;
	case WM_ERASEBKGND: {} return 1;
	case WM_CLOSE: { running = false; } return 0;
	case WM_DESTROY: { PostQuitMessage(0); running = false; } return 0;
	case WM_DPICHANGED: {

	} return 0;
	case WM_SIZE: {
		switch (wParam)
		{
		case SIZE_MINIMIZED: { Settings::FOCUSED = false; Settings::MINIMISED = true; } break;
		case SIZE_RESTORED: { Settings::MINIMISED = false; } break;
		}

		RECT rect{};
		GetWindowRect(hwnd, &rect);

		windowX = rect.left;
		windowY = rect.top;
		windowWidth = rect.right - rect.left;
		windowHeight = rect.bottom - rect.top;

		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_POSITION_X_SMALL = rect.left - border.left;
			Settings::WINDOW_POSITION_Y_SMALL = rect.top - border.top;
			Settings::WINDOW_WIDTH_SMALL = (rect.right - rect.left) - border.right + border.left;
			Settings::WINDOW_HEIGHT_SMALL = (rect.bottom - rect.top) - border.bottom + border.top;
		}

		Settings::WINDOW_POSITION_X = rect.left - border.left;
		Settings::WINDOW_POSITION_Y = rect.top - border.top;
		Settings::WINDOW_WIDTH = (rect.right - rect.left) - border.right + border.left;
		Settings::WINDOW_HEIGHT = (rect.bottom - rect.top) - border.bottom + border.top;

		//RendererFrontend::OnResize();
	} return 0;
	case WM_MOVE: {
		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_POSITION_X_SMALL = LOWORD(lParam);
			Settings::WINDOW_POSITION_Y_SMALL = HIWORD(lParam);
		}

		Settings::WINDOW_POSITION_X = LOWORD(lParam);
		Settings::WINDOW_POSITION_Y = HIWORD(lParam);
	} return 0;


	case WM_DEVICECHANGE: {
		//TODO: 
	} break;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

#endif