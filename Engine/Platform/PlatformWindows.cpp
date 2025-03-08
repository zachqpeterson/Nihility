#include "Platform.hpp"


#ifdef NH_PLATFORM_WINDOWS

#include "Resources/Settings.hpp"

#include "WindowsInclude.hpp"
#include <ole2.h>
#include <shellapi.h>

static HMODULE instance;
static HWND window;

static U32 style;
static U32 styleEx;
static RECT border;

static HICON arrow;
static HICON hand;
static HICON sizeNS;
static HICON sizeWE;
static HICON sizeNESW;
static HICON sizeNWSE;

static I32 cursorDisplayCount;

U32 Platform::screenWidth;
U32 Platform::screenHeight;
U32 Platform::virtualScreenWidth;
U32 Platform::virtualScreenHeight;
U32 Platform::refreshRate;
bool Platform::minimised = false;
bool Platform::resized = false;
bool Platform::focused = false;

bool Platform::running = false;

static constexpr const CW* MenuName = L"Nihility Menu";
static constexpr const CW* ClassName = L"Nihility Class";

I64 __stdcall WindowsMessageProc(HWND hwnd, U32 msg, U64 wParam, I64 lParam);

bool Platform::Initialize()
{
	if (OleInitialize(nullptr) < 0) { return false; }
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	instance = GetModuleHandle(nullptr);

	arrow = LoadCursor(nullptr, IDC_ARROW);
	hand = LoadCursor(nullptr, IDC_HAND);
	sizeNS = LoadCursor(nullptr, IDC_SIZENS);
	sizeWE = LoadCursor(nullptr, IDC_SIZEWE);
	sizeNESW = LoadCursor(nullptr, IDC_SIZENESW);
	sizeNWSE = LoadCursor(nullptr, IDC_SIZENWSE);

	WNDCLASSEX wc{};
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WindowsMessageProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(nullptr, L""); //MAKEINTRESOURCEW(IDI_ICON)
	wc.hCursor = arrow;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = MenuName;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(nullptr, L""); //MAKEINTRESOURCEW(IDI_ICON)

	if (!RegisterClassEx(&wc))
	{
		MessageBox(nullptr, L"Window registration failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_VISIBLE;
	styleEx = WS_EX_ACCEPTFILES;

	U32 dpi = GetDpiForSystem();
	U32 lastDpi = Settings::dpi;
	Settings::dpi = dpi;

	screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::dpi);
	screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::dpi);
	virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, Settings::dpi);
	virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, Settings::dpi);

	if (lastDpi)
	{
		//TODO: replace with Math
		Settings::windowPositionXSmall = MulDiv(Settings::windowPositionXSmall, dpi, lastDpi);
		Settings::windowPositionYSmall = MulDiv(Settings::windowPositionYSmall, dpi, lastDpi);
		Settings::windowWidthSmall = MulDiv(Settings::windowWidthSmall, dpi, lastDpi);
		Settings::windowHeightSmall = MulDiv(Settings::windowHeightSmall, dpi, lastDpi);
	}

	if (Settings::fullscreen)
	{
		Settings::windowPositionX = 0;
		Settings::windowPositionY = 0;
		Settings::windowWidth = 0;
		Settings::windowHeight = 0;
	}
	else
	{
		Settings::windowPositionX = Settings::windowPositionXSmall;
		Settings::windowPositionY = Settings::windowPositionYSmall;
		Settings::windowWidth = Settings::windowWidthSmall;
		Settings::windowHeight = Settings::windowHeightSmall;
	}

	AdjustWindowRectExForDpi(&border, style, 0, styleEx, dpi);

	window = CreateWindowEx(styleEx, ClassName, L"Test", style,
		Settings::windowPositionX + border.left, Settings::windowPositionY + border.top,
		Settings::windowWidth + border.right - border.left, Settings::windowHeight + border.bottom - border.top,
		nullptr, nullptr, instance, nullptr);

	if (!window)
	{
		MessageBox(nullptr, L"Window creation failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	DEVMODE monitorInfo{};
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &monitorInfo);
	F64 framerate = Settings::targetFrametime;

	if (framerate == 0.0) { Settings::targetFrametime = 1.0 / monitorInfo.dmDisplayFrequency; }
	refreshRate = monitorInfo.dmDisplayFrequency;

	RegisterClipboardFormatA("NihilityClipboard");
	DragAcceptFiles(window, TRUE);

	ShowWindow(window, Settings::fullscreen ? SW_SHOWMAXIMIZED : SW_SHOW);

	return true;
}

void Platform::Shutdown()
{

}

bool Platform::Update()
{
	MSG msg;
	
	while (PeekMessage(&msg, nullptr, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return running;
}

I64 __stdcall Platform::WindowsMessageProc(HWND hwnd, U32 msg, U64 wParam, I64 lParam)
{
	switch (msg)
	{
	case WM_CREATE: {} return 0;
	case WM_SETFOCUS: {
		focused = true;
	} return 0;
	case WM_KILLFOCUS: {
		focused = false;
	} return 0;
	case WM_QUIT: {
		focused = false;
		running = false;
	} return 0;
	case WM_CLOSE: {
		focused = false;
		running = false;
	} return 0;
	case WM_DESTROY: {
		focused = false;
		running = false;
	} return 0;
	case WM_ERASEBKGND: {} return 1;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

#endif