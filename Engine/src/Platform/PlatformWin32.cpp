#include "Platform.hpp"

#ifdef PLATFORM_WINDOWS
#include "Core/Logger.hpp"
#include "Core/Input.hpp"
#include "Core/Events.hpp"
#include "Core/Settings.hpp"
#include "Math/Math.hpp"
#include "Containers/String.hpp"

#include <windows.h>
#include <windowsx.h>
#include <memory>

#define WHEEL_MULTIPLIER 0.00833333333f
#define WIDTH_BUFFER 16
#define HEIGHT_BUFFER 39

struct PlatformState
{
	HINSTANCE hInstance{ nullptr };
	HWND hwnd{ nullptr };
	DEVMODEA monitorInfo{};
	U32 clientX{ 0 };
	U32 clientY{ 0 };
	Vector2Int clientSize{ Vector2Int::ZERO };
	Vector2Int screenSize{ Vector2Int::ZERO };
	U32 windowX{ 0 };
	U32 windowY{ 0 };
	U32 windowWidth{ 0 };
	U32 windowHeight{ 0 };
	U32 style{ 0 };
	bool running{ true };
};

PlatformState Platform::platformState;

static F64 clockFrequency;
static LARGE_INTEGER startTime;

void Platform::ClockSetup()
{
	QueryPerformanceCounter(&startTime);
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	clockFrequency = 1.0 / (F64)frequency.QuadPart;
}

bool Platform::Initialize(const String& applicationName)
{
	Logger::Info("Initializing platform...");

	platformState.hInstance = GetModuleHandleA(0);

	// Setup and register window class.
	WNDCLASSA wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = Win32MessageProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = platformState.hInstance;
	wc.hIcon = LoadIcon(platformState.hInstance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszClassName = "Nihility Window Class";

	if (!RegisterClassA(&wc))
	{
		MessageBoxA(0, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	platformState.screenSize = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };

	if (Settings::Fullscreen)
	{
		platformState.clientX = 0;
		platformState.clientY = 0;
		platformState.clientSize = platformState.screenSize;

		platformState.style = WS_VISIBLE | WS_POPUP;
	}
	else
	{
		platformState.clientX = Settings::WindowPositionX;
		platformState.clientY = Settings::WindowPositionX;
		platformState.clientSize.x = Settings::WindowWidthSmall;
		platformState.clientSize.y = Settings::WindowHeightSmall;

		platformState.style = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	}

	RECT borderRect = { };
	AdjustWindowRectEx(&borderRect, platformState.style, 0, 0);

	platformState.windowX = platformState.clientX + borderRect.left;
	platformState.windowY = platformState.clientY + borderRect.top;
	platformState.windowWidth = platformState.clientSize.x + borderRect.right - borderRect.left;
	platformState.windowHeight = platformState.clientSize.y + borderRect.bottom - borderRect.top;

	HWND handle = CreateWindowExA(0, "Nihility Window Class", applicationName, platformState.style, 
		platformState.windowX, platformState.windowY, platformState.windowWidth, platformState.windowHeight, 0, 0, platformState.hInstance, 0);

	if (handle == nullptr)
	{
		MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

		Logger::Fatal("Window creation failed!");
		return false;
	}
	else
	{
		platformState.hwnd = handle;
	}

	LockMouse(Settings::LockCursor);

	if (!EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &platformState.monitorInfo)) { Logger::Error("Couldn't get monitor info!"); }
	if (Settings::TargetFrametime == 0.0) { Settings::TARGET_FRAMETIME = 1.0 / platformState.monitorInfo.dmDisplayFrequency; }

	// Show the window
	bool shouldActivate = true;  //TODO: If the window should not accept input, this should be false.
	I32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
	//TODO: If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
	//TODO: If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE;
	ShowWindow(platformState.hwnd, SW_SHOW);

	ClockSetup();

	return true;
}

void Platform::SetFullscreen(bool fullscreen)
{
	if (fullscreen)
	{
		platformState.clientX = 0;
		platformState.clientY = 0;
		platformState.clientSize.x = GetSystemMetrics(SM_CXSCREEN);
		platformState.clientSize.y = GetSystemMetrics(SM_CYSCREEN);

		platformState.style = WS_VISIBLE | WS_POPUP;
	}
	else
	{
		platformState.clientX = Settings::WindowPositionX;
		platformState.clientY = Settings::WindowPositionX;
		platformState.clientSize.x = Settings::WindowWidthSmall;
		platformState.clientSize.y = Settings::WindowHeightSmall;

		platformState.style = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	}

	RECT borderRect = { };
	AdjustWindowRectEx(&borderRect, platformState.style, 0, 0);

	platformState.windowX = platformState.clientX + borderRect.left;
	platformState.windowY = platformState.clientY + borderRect.top;
	platformState.windowWidth = platformState.clientSize.x + borderRect.right - borderRect.left;
	platformState.windowHeight = platformState.clientSize.y + borderRect.bottom - borderRect.top;

	DEVMODE dmSettings{};
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmSettings);

	dmSettings.dmPelsWidth = platformState.windowWidth;
	dmSettings.dmPelsHeight = platformState.windowHeight;
	dmSettings.dmPosition = { (I32)platformState.windowX, (I32)platformState.windowY };
	dmSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_POSITION;

	I32 result;
	if (result = ChangeDisplaySettings(&dmSettings, 0)) { Logger::Fatal("Failed to set window size/style with code: {}", result); }
	SetWindowLongPtr(platformState.hwnd, GWL_STYLE, platformState.style);
	//SetWindowPos(platformState.hwnd, HWND_TOPMOST, platformState.windowX, platformState.windowY, platformState.windowWidth, platformState.windowHeight, SWP_SHOWWINDOW);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmSettings);
}

void Platform::LockMouse(bool lock)
{
	if (lock)
	{
		Settings::LOCK_CURSOR = true;
		RECT clipRect = { };
		GetWindowRect(platformState.hwnd, &clipRect);

		F32 aspectHeight = clipRect.bottom * 1.77777777778f;
		F32 aspectWidth = clipRect.right * 0.5625f;
		
		if (clipRect.right > aspectHeight)
		{
			I32 offset = (I32)((clipRect.right - aspectHeight) * 0.5f);
			clipRect.left = offset;
			clipRect.right = (I32)(clipRect.right - offset);
		}
		else
		{
			I32 offset = (I32)((clipRect.bottom - aspectWidth) * 0.5f);
			clipRect.top = offset;
			clipRect.bottom = (I32)(clipRect.bottom - offset);
		}

		if (!ClipCursor(&clipRect)) { Logger::Error("ClipCursor failed with message: {}", (U32)GetLastError()); }
	}
	else
	{
		Settings::LOCK_CURSOR = false;
		ClipCursor(nullptr);
	}
}

const void* Platform::Handle()
{
	return platformState.hwnd;
}

void Platform::Shutdown()
{
	if (platformState.hwnd)
	{
		DestroyWindow(platformState.hwnd);
		platformState.hwnd = nullptr;
	}
}

bool Platform::ProcessMessages()
{
	Input::ResetInput();

	MSG message;

	while (PeekMessageA(&message, platformState.hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	return platformState.running;
}

void* Platform::Allocate(U64 size, bool aligned)
{
	return malloc(size);
}

void Platform::Free(void* block, bool aligned)
{
	free(block);
}

void* Platform::Zero(void* block, U64 size)
{
	return memset(block, 0, size);
}

void* Platform::Copy(void* dest, const void* source, U64 size)
{
	return memcpy(dest, source, size);
}

void* Platform::Set(void* dest, I32 value, U64 size)
{
	return memset(dest, value, size);
}

void Platform::ConsoleWrite(const String& message, U8 color)
{
	HANDLE consoleHandle = GetStdHandle(color < 2 ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	static const U8 levels[6] = { 64, 4, 6, 2, 1, 8 };
	SetConsoleTextAttribute(consoleHandle, levels[color]);
	OutputDebugStringA((const char*)message);
	LPDWORD numberWritten = 0;

	WriteConsoleA(consoleHandle, message, (DWORD)message.Length(), numberWritten, 0);
}

const F64 Platform::AbsoluteTime()
{
	LARGE_INTEGER nowTime;
	QueryPerformanceCounter(&nowTime);
	return (F64)nowTime.QuadPart * clockFrequency;
}

void Platform::GetVulkanSurfaceInfo(void* surfaceInfo)
{
	HINSTANCE* arr = (HINSTANCE*)surfaceInfo;
	arr[0] = platformState.hInstance;
	arr[1] = *(HINSTANCE*)&platformState.hwnd;
}

const Vector2Int& Platform::ScreenSize()
{
	return platformState.screenSize;
}

I64 __stdcall Platform::Win32MessageProc(HWND__* hwnd, U32 msg, U64 wParam, I64 lParam)
{
	switch (msg)
	{
	case WM_SETFOCUS: /*TODO: Notify engine has focus*/ return 0;
	case WM_KILLFOCUS: /*TODO: Notify engine doesn't have focus*/ return 0;
	case WM_ERASEBKGND: return 1;
	case WM_CLOSE: Events::Notify("CLOSE", nullptr); platformState.running = false; return 0;
	case WM_DESTROY: PostQuitMessage(0); return 0;
	case WM_SIZE: {
		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_WIDTH_SMALL = LOWORD(lParam);
			Settings::WINDOW_HEIGHT_SMALL = HIWORD(lParam);
		}
		Settings::WINDOW_WIDTH = LOWORD(lParam);
		Settings::WINDOW_HEIGHT = HIWORD(lParam);
		Events::Notify("Resize", NULL);
	} return 0;
	case WM_MOVE: {
		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_POSITION_X = LOWORD(lParam);
			Settings::WINDOW_POSITION_Y = HIWORD(lParam);
		}
	} return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP: {
		bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
		U8 code = (U8)wParam;

		//TODO: Handle left and right menu keys

		Input::SetButtonState(code, pressed, false);
	} return 0;
	case WM_LBUTTONDOWN: {
		Input::SetButtonState(1, true, false);
	} return 0;
	case WM_MBUTTONDOWN: {
		Input::SetButtonState(4, true, false);
	} return 0;
	case WM_RBUTTONDOWN: {
		Input::SetButtonState(2, true, false);
	} return 0;
	case WM_LBUTTONUP: {
		Input::SetButtonState(1, false, false);
	} return 0;
	case WM_MBUTTONUP: {
		Input::SetButtonState(4, false, false);
	} return 0;
	case WM_RBUTTONUP: {
		Input::SetButtonState(2, false, false);
	} return 0;
	case WM_LBUTTONDBLCLK: {
		Input::SetButtonState(1, false, true);
	} return 0;
	case WM_MBUTTONDBLCLK: {
		Input::SetButtonState(4, false, true);
	} return 0;
	case WM_RBUTTONDBLCLK: {
		Input::SetButtonState(2, false, true);
	} return 0;
	//TODO: Hanlde "X" buttons and "Cancel"
	case WM_MOUSEWHEEL: {
		Input::SetMouseWheel((I16)((F32)GET_WHEEL_DELTA_WPARAM(wParam) * WHEEL_MULTIPLIER));
	} return 0;
	case WM_MOUSEMOVE: {
		Input::SetMousePos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	} return 0;
	}

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#endif