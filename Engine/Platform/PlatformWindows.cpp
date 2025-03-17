#include "Platform.hpp"

#include "Core/Logger.hpp"
#include "Core/Events.hpp"

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

Event<bool> Platform::OnFocused;
Event<String> Platform::OnDragDrop;

static constexpr const CW* MenuName = L"Nihility Menu";
static constexpr const CW* ClassName = L"Nihility Class";

I64 __stdcall WindowsMessageProc(HWND hwnd, U32 msg, U64 wParam, I64 lParam);

bool Platform::Initialize()
{
	Logger::Trace("Initializing Platform...");

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

	running = true;

	return true;
}

void Platform::Shutdown()
{
	Logger::Trace("Cleaning Up Platform...");

	if (window)
	{
		RevokeDragDrop(window);
		DestroyWindow(window);
		window = nullptr;

		UnregisterClass(ClassName, instance);
	}
}

bool Platform::Update()
{
	MSG msg;
	
	while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
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
		OnFocused(true);
	} return 0;
	case WM_KILLFOCUS: {
		focused = false;
		OnFocused(false);
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
	case WM_DPICHANGED: {
		Settings::dpi = HIWORD(wParam);
		AdjustWindowRectExForDpi(&border, style, 0, styleEx, Settings::dpi);
		RECT* rect = (RECT*)lParam;

		screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::dpi);
		screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::dpi);
		virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, Settings::dpi);
		virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, Settings::dpi);

		Settings::windowPositionXSmall = rect->left - border.left;
		Settings::windowPositionYSmall = rect->top - border.top;
		Settings::windowWidthSmall = rect->right - rect->left - border.right + border.left;
		Settings::windowHeightSmall = rect->bottom - rect->top - border.bottom + border.top;

		if (!Settings::fullscreen)
		{
			Settings::windowPositionX = Settings::windowPositionXSmall;
			Settings::windowPositionY = Settings::windowPositionYSmall;
			Settings::windowWidth = Settings::windowWidthSmall;
			Settings::windowHeight = Settings::windowHeightSmall;

			SetWindowPos(window, nullptr, Settings::windowPositionX + border.left, Settings::windowPositionY + border.top,
				Settings::windowWidth + border.right - border.left, Settings::windowHeight + border.bottom - border.top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	} return 0;
	case WM_SIZE: {
		switch (wParam)
		{
		case SIZE_MINIMIZED: {
			focused = false;
			minimised = true;
		} break;
		case SIZE_RESTORED: {
			focused = true;
			minimised = false;
		} break;
		}

		RECT rect{};
		GetWindowRect(hwnd, &rect);

		Settings::windowPositionX = rect.left - border.left;
		Settings::windowPositionY = rect.top - border.top;
		Settings::windowWidth = rect.right - rect.left - border.right + border.left;
		Settings::windowHeight = rect.bottom - rect.top - border.bottom + border.top;

		if (!Settings::fullscreen)
		{
			Settings::windowPositionXSmall = Settings::windowPositionX;
			Settings::windowPositionYSmall = Settings::windowPositionY;
			Settings::windowWidthSmall = Settings::windowWidth;
			Settings::windowHeightSmall = Settings::windowHeight;
		}

		screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::dpi);
		screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::dpi);
		virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, Settings::dpi);
		virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, Settings::dpi);

		resized = true;
	} return 0;
	case WM_SIZING: {
		//TODO:
	} return 1;
	case WM_MOVE: {
		Settings::windowPositionX = LOWORD(lParam);
		Settings::windowPositionY = HIWORD(lParam);

		if (!Settings::fullscreen)
		{
			Settings::windowPositionXSmall = Settings::windowPositionX;
			Settings::windowPositionYSmall = Settings::windowPositionY;
		}

		screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::dpi);
		screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::dpi);
		virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, Settings::dpi);
		virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, Settings::dpi);
	} return 0;
	case WM_SETCURSOR: {
		switch (LOWORD(lParam))
		{
		case HTCLIENT: { SetCursor(arrow); } return 1; //Client Area
		case HTCAPTION: { SetCursor(arrow); } return 1; //Title Bar
		case HTSYSMENU: { SetCursor(hand); } return 1; //Window Menu
		case HTGROWBOX: { SetCursor(arrow); } return 1; //Size Box?
		case HTMENU: { SetCursor(hand); } return 1; //Menu
		case HTHSCROLL: { SetCursor(hand); } return 1; //Horizontal Scroll Bar
		case HTVSCROLL: { SetCursor(hand); } return 1; //Vertical Scroll Bar
		case HTMINBUTTON: { SetCursor(hand); } return 1; //Minimize Button
		case HTMAXBUTTON: { SetCursor(hand); } return 1; //Maximize Button
		case HTLEFT: { SetCursor(sizeWE); } return 1; //Border Left
		case HTRIGHT: { SetCursor(sizeWE); } return 1; //Border Right
		case HTTOP: { SetCursor(sizeNS); } return 1; //Border Top
		case HTTOPLEFT: { SetCursor(sizeNWSE); } return 1; //Border Top Left
		case HTTOPRIGHT: { SetCursor(sizeNWSE); } return 1; //Border Top Right
		case HTBOTTOM: { SetCursor(sizeNS); } return 1; //Border Bottom
		case HTBOTTOMLEFT: { SetCursor(sizeNWSE); } return 1; //Border Bottom Left
		case HTBOTTOMRIGHT: { SetCursor(sizeNWSE); } return 1; //Border Bottom Right
		case HTBORDER: { SetCursor(arrow); } return 1; //Any Border (Not Resizable)
			//case HTOBJECT: { SetCursor(); } return 1;
			//case HTCLOSE: { SetCursor(); } return 1;
			//case HTHELP: { SetCursor(); } return 1;
		default: break;
		}
	} break;
	case WM_DROPFILES: {
		HDROP dropInfo = (HDROP)wParam;
		U32 count = DragQueryFileA(dropInfo, 0xFFFFFFFF, nullptr, 0);

		String path;
		path.Reserve(256);
		U32 pathSize;

		for (U32 i = 0; i < count; ++i)
		{
			pathSize = DragQueryFileA(dropInfo, i, path.Data(), 256);
			path.Resize(pathSize);

			OnDragDrop(path);
		}

		DragFinish(dropInfo);
	} return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Platform::SetFullscreen(bool fullscreen)
{
	Settings::fullscreen = fullscreen;

	if (Settings::fullscreen)
	{
		SetWindowPos(window, nullptr, border.left, border.top,
			screenWidth + border.right - border.left, screenHeight + border.bottom - border.top, SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos(window, nullptr, Settings::windowPositionXSmall + border.left, Settings::windowPositionYSmall + border.top,
			Settings::windowWidthSmall + border.right - border.left, Settings::windowHeightSmall + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowSize(U32 width, U32 height)
{
	if (!Settings::fullscreen)
	{
		SetWindowPos(window, nullptr, Settings::windowPositionX + border.left, Settings::windowPositionY + border.top,
			width + border.right - border.left, height + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowPosition(I32 x, I32 y)
{
	if (!Settings::fullscreen)
	{
		SetWindowPos(window, nullptr, x + border.left, y + border.top,
			Settings::windowWidth + border.right - border.left, Settings::windowHeight + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetConsoleWindowTitle(const char* name)
{
	SetConsoleTitleA(name);
}

U32 Platform::ScreenWidth()
{
	return screenWidth;
}

U32 Platform::ScreenHeight()
{
	return screenHeight;
}

U32 Platform::VirtualScreenWidth()
{
	return virtualScreenWidth;
}

U32 Platform::VirtualScreenHeight()
{
	return virtualScreenHeight;
}

bool Platform::Focused()
{
	return focused;
}

bool Platform::Minimised()
{
	return minimised;
}

bool Platform::Resized()
{
	return resized;
}

bool Platform::MouseConstrained()
{
	return Settings::cursorConstrained;
}

void Platform::ConstrainMouse(bool b)
{
	Settings::cursorConstrained = b;
}

#endif