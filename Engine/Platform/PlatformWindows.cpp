#include "Platform.hpp"

#include "Settings.hpp"
#include "Audio.hpp"
#include "Input.hpp"

#include "Resources\Resources.hpp"
#include "Core\Logger.hpp"
#include "Core\Events.hpp"

#ifdef NH_PLATFORM_WINDOWS

#include <Windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include <ole2.h>

#endif

bool Platform::running;
WindowData Platform::windowData;
U32 Platform::dpi;
U32 Platform::screenWidth;
U32 Platform::screenHeight;
U32 Platform::virtualScreenWidth;
U32 Platform::virtualScreenHeight;
U32 Platform::windowWidth;
U32 Platform::windowHeight;
U32 Platform::windowWidthSmall;
U32 Platform::windowHeightSmall;
U32 Platform::windowPositionX;
U32 Platform::windowPositionY;
U32 Platform::windowPositionXSmall;
U32 Platform::windowPositionYSmall;
U32 Platform::refreshRate;
I32 Platform::cursorDisplayCount;
bool Platform::resized = false;
bool Platform::focused;
bool Platform::minimised = false;
bool Platform::fullscreen;
bool Platform::cursorLocked = false;
bool Platform::cursorConstrained;
bool Platform::cursorShowing = true;

U32 style;
U32 styleEx;
RECT border;

HICON arrow;
HICON hand;
HICON sizeNS;
HICON sizeWE;
HICON sizeNESW;
HICON sizeNWSE;

static constexpr const C8* MENU_NAME = "Nihility Menu";
static constexpr const C8* CLASS_NAME = "Nihility Class";

#ifdef NH_PLATFORM_WINDOWS

bool Platform::Initialize(const StringView& applicationName)
{
	Logger::Trace("Initializing Platform...");

	//Register Events
	Events::RegisterEvent("Unfocused");
	Events::RegisterEvent("Focused");

	if (OleInitialize(nullptr) < 0) { return false; }
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	windowData.instance = GetModuleHandleA(0);
	running = true;

	//Load cursor images
	arrow = LoadCursorW(nullptr, IDC_ARROW);
	hand = LoadCursorW(nullptr, IDC_HAND);
	sizeNS = LoadCursorW(nullptr, IDC_SIZENS);
	sizeWE = LoadCursorW(nullptr, IDC_SIZEWE);
	sizeNESW = LoadCursorW(nullptr, IDC_SIZENESW);
	sizeNWSE = LoadCursorW(nullptr, IDC_SIZENWSE);

	//Setup and register window class.
	WNDCLASSEXA wc{};
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_OWNDC;
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
	styleEx = WS_EX_ACCEPTFILES;

	U32 lastDpi = 0;
	dpi = GetDpiForSystem();
	Settings::GetSetting(Dpi, lastDpi);
	Settings::SetSetting(Dpi, dpi);

	screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
	screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);
	virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, dpi);
	virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, dpi);

	if (lastDpi)
	{
		U32 value;
		Settings::GetSetting(WindowPositionXSmall, value);
		windowPositionXSmall = MulDiv(value, dpi, lastDpi);
		Settings::SetSetting(WindowPositionXSmall, windowPositionXSmall);

		Settings::GetSetting(WindowPositionYSmall, value);
		windowPositionYSmall = MulDiv(value, dpi, lastDpi);
		Settings::SetSetting(WindowPositionYSmall, windowPositionYSmall);

		Settings::GetSetting(WindowWidthSmall, value);
		windowWidthSmall = MulDiv(value, dpi, lastDpi);
		Settings::SetSetting(WindowWidthSmall, windowWidthSmall);

		Settings::GetSetting(WindowHeightSmall, value);
		windowHeightSmall = MulDiv(value, dpi, lastDpi);
		Settings::SetSetting(WindowHeightSmall, windowHeightSmall);
	}

	Settings::GetSetting(Fullscreen, fullscreen);

	if (fullscreen)
	{
		windowPositionX = 0;
		Settings::SetSetting(WindowPositionX, windowPositionX);
		windowPositionY = 0;
		Settings::SetSetting(WindowPositionY, windowPositionY);
		windowWidth = screenWidth;
		Settings::SetSetting(WindowWidth, windowWidth);
		windowHeight = screenHeight;
		Settings::SetSetting(WindowHeight, windowHeight);
	}
	else
	{
		Settings::GetSetting(WindowPositionXSmall, windowPositionX);
		Settings::SetSetting(WindowPositionX, windowPositionX);

		Settings::GetSetting(WindowPositionYSmall, windowPositionY);
		Settings::SetSetting(WindowPositionY, windowPositionY);

		Settings::GetSetting(WindowWidthSmall, windowWidth);
		Settings::SetSetting(WindowWidth, windowWidth);

		Settings::GetSetting(WindowHeightSmall, windowHeight);
		Settings::SetSetting(WindowHeight, windowHeight);
	}

	AdjustWindowRectExForDpi(&border, style, 0, styleEx, dpi);

	windowData.window = CreateWindowExA(styleEx, CLASS_NAME, applicationName.Data(), style, 
		windowPositionX + border.left, windowPositionY + border.top,
		windowWidth + border.right - border.left, windowHeight + border.bottom - border.top,
		nullptr, nullptr, windowData.instance, nullptr);

	if (!windowData.window)
	{
		MessageBoxA(nullptr, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	DEVMODEA monitorInfo{};
	EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &monitorInfo);
	F64 framerate;
	Settings::GetSetting(TargetFrametime, framerate);

	if (framerate == 0.0) { Settings::SetSetting(TargetFrametime, 1.0 / monitorInfo.dmDisplayFrequency); }
	refreshRate = monitorInfo.dmDisplayFrequency;

	RegisterClipboardFormatA("NihilityClipboard");
	DragAcceptFiles(windowData.window, TRUE);

	Settings::GetSetting(CursorConstrained, cursorConstrained);

	ShowWindow(windowData.window, fullscreen ? SW_SHOWMAXIMIZED : SW_SHOW);

	return true;
}

void Platform::Shutdown()
{
	Logger::Trace("Shutting Down Platform...");

	if (windowData.window)
	{
		RevokeDragDrop(windowData.window);
		DestroyWindow(windowData.window);
		windowData.window = nullptr;

		UnregisterClassA(CLASS_NAME, windowData.instance);
	}
}

bool Platform::Update()
{
	UpdateMouse();

	MSG message;

	while (PeekMessageA(&message, windowData.window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	return running;
}

void Platform::SetFullscreen(bool fullscreen_)
{
	fullscreen = fullscreen_;
	Settings::SetSetting(Fullscreen, fullscreen);

	if (fullscreen)
	{
		SetWindowPos(windowData.window, nullptr, border.left, border.top,
			screenWidth + border.right - border.left, screenHeight + border.bottom - border.top, SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos(windowData.window, nullptr, windowPositionXSmall + border.left, windowPositionYSmall + border.top,
			windowWidthSmall + border.right - border.left, windowHeightSmall + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowSize(U32 width, U32 height)
{
	if (!fullscreen)
	{
		SetWindowPos(windowData.window, nullptr, windowPositionX + border.left, windowPositionY + border.top,
			width + border.right - border.left, height + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowPosition(I32 x, I32 y)
{
	if (!fullscreen)
	{
		SetWindowPos(windowData.window, nullptr, x + border.left, y + border.top,
			windowWidth + border.right - border.left, windowHeight + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetConsoleWindowTitle(const StringView& name)
{
	SetConsoleTitleA(name.Data());
}

const WindowData& Platform::GetWindowData()
{
	return windowData;
}

void Platform::UpdateMouse()
{
	if (focused)
	{
		if (cursorLocked)
		{
			RECT clip{};
			clip.left = windowPositionX + windowWidth / 2;
			clip.right = clip.left;
			clip.top = windowPositionY + windowHeight / 2;
			clip.bottom = clip.top;

			ClipCursor(&clip);
		}
		else if (cursorConstrained)
		{
			RECT clip{};
			if (fullscreen)
			{
				clip.left = windowPositionX;
				clip.top = windowPositionY;
				clip.right = windowPositionX + windowWidth;
				clip.bottom = windowPositionY + windowHeight;
				ClipCursor(&clip);
			}
			else
			{
				GetWindowRect(windowData.window, &clip);
				ClipCursor(&clip);
			}
		}
		else
		{
			ClipCursor(nullptr);
		}

		if (cursorShowing && cursorDisplayCount == -1)
		{
			cursorDisplayCount = ShowCursor(true);
		}
		else if (!cursorShowing && cursorDisplayCount == 0)
		{
			cursorDisplayCount = ShowCursor(false);
		}

		if (Input::ButtonDown(BUTTON_CODE_LEFT_MOUSE) || Input::ButtonDown(BUTTON_CODE_RIGHT_MOUSE) || Input::ButtonDown(BUTTON_CODE_MIDDLE_MOUSE)) { SetCapture(windowData.window); }
		else { ReleaseCapture(); }
	}
	else
	{
		if (cursorDisplayCount == -1) { cursorDisplayCount = ShowCursor(true); cursorShowing = true; }

		ClipCursor(nullptr);
	}
}

I64 __stdcall Platform::WindowsMessageProc(HWND hwnd, U32 msg, U64 wParam, I64 lParam)
{
	switch (msg)
	{
	case WM_CREATE: { } return 0;
	case WM_SETFOCUS: {
		focused = true;
		Events::Notify("Focused");
	} return 0;
	case WM_KILLFOCUS: {
		focused = false;
		Events::Notify("Unfocused");
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
		dpi = HIWORD(wParam);
		Settings::SetSetting(Dpi, dpi);
		AdjustWindowRectExForDpi(&border, style, 0, styleEx, dpi);
		RECT* rect = (RECT*)lParam;

		screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
		screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);
		virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, dpi);
		virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, dpi);

		windowPositionXSmall = rect->left - border.left;
		windowPositionYSmall = rect->top - border.top;
		windowWidthSmall = rect->right - rect->left - border.right + border.left;
		windowHeightSmall = rect->bottom - rect->top - border.bottom + border.top;

		Settings::SetSetting(WindowPositionXSmall, windowPositionXSmall);
		Settings::SetSetting(WindowPositionYSmall, windowPositionYSmall);
		Settings::SetSetting(WindowWidthSmall, windowWidthSmall);
		Settings::SetSetting(WindowHeightSmall, windowHeightSmall);

		if (!fullscreen)
		{
			windowPositionX = windowPositionXSmall;
			windowPositionY = windowPositionYSmall;
			windowWidth = windowWidthSmall;
			windowHeight = windowHeightSmall;

			Settings::SetSetting(WindowPositionX, windowPositionX);
			Settings::SetSetting(WindowPositionY, windowPositionY);
			Settings::SetSetting(WindowWidth, windowWidth);
			Settings::SetSetting(WindowHeight, windowHeight);

			SetWindowPos(windowData.window, nullptr, windowPositionX + border.left, windowPositionY + border.top,
				windowWidth + border.right - border.left, windowHeight + border.bottom - border.top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	} return 0;
	case WM_SIZE: {
		switch (wParam)
		{
		case SIZE_MINIMIZED: {
			focused = false;
			minimised = true;
			Audio::Unfocus();
		} break;
		case SIZE_RESTORED: {
			focused = true;
			minimised = false;
			Audio::Focus();

			//TODO: Fix cursor teleport
		} break;
		}

		RECT rect{};
		GetWindowRect(hwnd, &rect);

		windowPositionX = rect.left - border.left;
		windowPositionY = rect.top - border.top;
		windowWidth = rect.right - rect.left - border.right + border.left;
		windowHeight = rect.bottom - rect.top - border.bottom + border.top;

		Settings::SetSetting(WindowPositionX, windowPositionX);
		Settings::SetSetting(WindowPositionY, windowPositionY);
		Settings::SetSetting(WindowWidth, windowWidth);
		Settings::SetSetting(WindowHeight, windowHeight);

		if (!fullscreen)
		{
			windowPositionXSmall = windowPositionX;
			windowPositionYSmall = windowPositionY;
			windowWidthSmall = windowWidth;
			windowHeightSmall = windowHeight;

			Settings::SetSetting(WindowPositionXSmall, windowPositionXSmall);
			Settings::SetSetting(WindowPositionYSmall, windowPositionYSmall);
			Settings::SetSetting(WindowWidthSmall, windowWidthSmall);
			Settings::SetSetting(WindowHeightSmall, windowHeightSmall);
		}

		screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
		screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);
		virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, dpi);
		virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, dpi);

		resized = true;
	} return 0;
	case WM_SIZING: {
		//TODO:
	} return 1;
	case WM_MOVE: {
		windowPositionX = LOWORD(lParam);
		windowPositionY = HIWORD(lParam);

		Settings::SetSetting(WindowPositionX, windowPositionX);
		Settings::SetSetting(WindowPositionY, windowPositionY);

		if (!fullscreen)
		{
			windowPositionXSmall = windowPositionX;
			windowPositionYSmall = windowPositionY;

			Settings::SetSetting(WindowPositionXSmall, windowPositionXSmall);
			Settings::SetSetting(WindowPositionYSmall, windowPositionYSmall);
		}

		screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
		screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);
		virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, dpi);
		virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, dpi);
	} return 0;

	case WM_INPUT_DEVICE_CHANGE: {
		if (wParam == GIDC_ARRIVAL) { Input::AddDevice((void*)lParam); }
		else { Input::RemoveDevice((void*)lParam); }
	} return 0;
	case WM_INPUT: {
		if (GET_RAWINPUT_CODE_WPARAM(wParam) == 0) { Input::ReceiveInput((HRAWINPUT)lParam); }
		else { Input::InputSink((HRAWINPUT)lParam); }
	} return 0;
	case WM_MOUSEMOVE: {
		U16 x = LOWORD(lParam);
		U16 y = HIWORD(lParam);

		Input::deltaMousePosX = x - Input::mousePosX;
		Input::deltaMousePosY = y - Input::mousePosY;

		Input::mousePosX = x;
		Input::mousePosY = y;
	} return 0;
	case WM_SETCURSOR: {
		switch (LOWORD(lParam))
		{
		case HTCLIENT: { SetCursor(arrow); } return 1; //Client Area
		case HTCAPTION: {SetCursor(arrow); } return 1; //Title Bar
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

		U32 nameSize = 0;
		C8 name[256];

		for (U32 i = 0; i < count; ++i)
		{
			nameSize = DragQueryFileA(dropInfo, i, name, 256);

			Resources::UploadFile(StringView{ name, nameSize });
		}

		DragFinish(dropInfo);
	} return 0;
	}

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

bool Platform::ExecuteProcess(const StringView& workingDirectory, const StringView& processFullpath, const StringView& arguments, const StringView& searchErrorString)
{
	static C8 processOutputBuffer[1025];

	// Create pipes for redirecting output
	HANDLE inPipeRead = NULL;
	HANDLE inPipeWrite = NULL;
	HANDLE outPipeRead = NULL;
	HANDLE outPipeWrite = NULL;

	SECURITY_ATTRIBUTES securityAttributes = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	BOOL ok = CreatePipe(&inPipeRead, &inPipeWrite, &securityAttributes, 0);
	if (ok == FALSE) { return false; }
	ok = CreatePipe(&outPipeRead, &outPipeWrite, &securityAttributes, 0);
	if (ok == FALSE) { return false; }

	// Create startup informations with std redirection
	STARTUPINFOA startupInfo = {};
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupInfo.hStdInput = inPipeRead;
	startupInfo.hStdError = outPipeWrite;
	startupInfo.hStdOutput = outPipeWrite;
	startupInfo.wShowWindow = SW_SHOW;

	bool executionSuccess = false;
	// Execute the process
	PROCESS_INFORMATION processInfo{};
	BOOL inheritHandles = TRUE;
	if (CreateProcessA(processFullpath.Data(), (C8*)arguments.Data(), 0, 0, inheritHandles, 0, 0, workingDirectory.Data(), &startupInfo, &processInfo))
	{
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);

		executionSuccess = true;
	}
	else
	{
		Logger::Error("Execute process error.\n Exe: \"{}\" - Args: \"{}\" - Work_dir: \"{}\"\n", processFullpath, arguments, workingDirectory);
	}
	CloseHandle(inPipeRead);
	CloseHandle(outPipeWrite);

	// Output
	DWORD bytesRead;
	ok = ReadFile(outPipeRead, processOutputBuffer, 1024, &bytesRead, nullptr);

	// Consume all outputs.
	// Terminate current read and initialize the next.
	while (ok == TRUE)
	{
		processOutputBuffer[bytesRead] = 0;

		ok = ReadFile(outPipeRead, processOutputBuffer, 1024, &bytesRead, nullptr);
	}

	if (searchErrorString.Size() > 0 && strstr(processOutputBuffer, searchErrorString.Data()))
	{
		executionSuccess = false;
	}

	// Close handles.
	CloseHandle(outPipeRead);
	CloseHandle(inPipeWrite);

	DWORD processExitCode = 0;
	GetExitCodeProcess(processInfo.hProcess, &processExitCode);

	return executionSuccess;
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
	return cursorConstrained;
}

void Platform::ConstrainMouse(bool b)
{
	cursorConstrained = b;
	Settings::SetSetting(CursorConstrained, b);
}

#endif