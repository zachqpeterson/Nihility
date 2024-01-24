#include "Platform.hpp"

#include "Input.hpp"
#include "Audio.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"
#include "Core\Logger.hpp"

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include <ole2.h>

bool Platform::running;
WindowData Platform::windowData;

bool cursorShowing = true;
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

bool Platform::Initialize(CSTR applicationName)
{
	Logger::Trace("Initializing Platform...");

	if (OleInitialize(nullptr) < 0) { return false; }
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	windowData.instance = GetModuleHandleA(0);
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

	U32 dpi = Settings::Dpi();

	Settings::data.dpi = GetDpiForSystem();

	Settings::screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::Dpi());
	Settings::screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::Dpi());
	Settings::virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, Settings::Dpi());
	Settings::virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, Settings::Dpi());

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

	AdjustWindowRectExForDpi(&border, style, 0, styleEx, Settings::Dpi());

	windowData.window = CreateWindowExA(styleEx, CLASS_NAME, applicationName, style, Settings::WindowPositionX() + border.left, Settings::WindowPositionY() + border.top,
		Settings::WindowWidth() + border.right - border.left, Settings::WindowHeight() + border.bottom - border.top, nullptr, nullptr, windowData.instance, nullptr);

	if (!windowData.window)
	{
		MessageBoxA(nullptr, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	DEVMODEA monitorInfo{};
	EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &monitorInfo);
	if (Settings::TargetFrametime() == 0.0) { Settings::data.targetFrametime = 1.0 / monitorInfo.dmDisplayFrequency; }
	Settings::monitorHz = monitorInfo.dmDisplayFrequency;

	RegisterClipboardFormatA("NihilityClipboard");
	DragAcceptFiles(windowData.window, TRUE);

	ShowWindow(windowData.window, Settings::Fullscreen() ? SW_SHOWMAXIMIZED : SW_SHOW);

	return true;
}

void Platform::Shutdown()
{
	Logger::Trace("Cleaning Up Platform...");

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

void Platform::SetFullscreen(bool fullscreen)
{
	Settings::data.fullscreen = fullscreen;

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
	if (!Settings::Fullscreen())
	{
		SetWindowPos(windowData.window, nullptr, Settings::WindowPositionX() + border.left, Settings::WindowPositionY() + border.top,
			width + border.right - border.left, height + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetWindowPosition(I32 x, I32 y)
{
	if (!Settings::Fullscreen())
	{
		SetWindowPos(windowData.window, nullptr, x + border.left, y + border.top,
			Settings::WindowWidth() + border.right - border.left, Settings::WindowHeight() + border.bottom - border.top, SWP_SHOWWINDOW);
	}
}

void Platform::SetConsoleWindowTitle(CSTR name)
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
		if (Settings::CursorLocked())
		{
			RECT clip{};
			clip.left = Settings::WindowPositionX() + Settings::WindowWidth() / 2;
			clip.right = clip.left;
			clip.top = Settings::WindowPositionY() + Settings::WindowHeight() / 2;
			clip.bottom = clip.top;

			ClipCursor(&clip);
		}
		else if (Settings::CursorConstrained())
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
		else
		{
			ClipCursor(nullptr);
		}

		if (cursorShowing != Settings::CursorShowing())
		{
			cursorShowing = Settings::CursorShowing();
			ShowCursor(Settings::CursorShowing());
		}

		if (Input::ButtonDown(BUTTON_CODE_LEFT_MOUSE) || Input::ButtonDown(BUTTON_CODE_RIGHT_MOUSE) || Input::ButtonDown(BUTTON_CODE_MIDDLE_MOUSE)) { SetCapture(windowData.window); }
		else { ReleaseCapture(); }
	}
	else
	{
		if (!cursorShowing) { ShowCursor(true); cursorShowing = true; }

		ClipCursor(nullptr);
	}
}

I64 __stdcall Platform::WindowsMessageProc(HWND hwnd, U32 msg, U64 wParam, I64 lParam)
{
	switch (msg)
	{
	case WM_CREATE: { } return 0;
	case WM_SETFOCUS: {
		Settings::focused = true;
		Audio::Focus();
		Input::Focus();
	} return 0;
	case WM_KILLFOCUS: {
		Settings::focused = false;
		Audio::Unfocus();
	} return 0;
	case WM_QUIT: {
		Settings::focused = false;
		running = false;
	} return 0;
	case WM_CLOSE: {
		Settings::focused = false;
		running = false;
	} return 0;
	case WM_DESTROY: {
		Settings::focused = false;
		running = false;
	} return 0;
	case WM_ERASEBKGND: {} return 1;
	case WM_DPICHANGED: {
		Settings::data.dpi = HIWORD(wParam);
		AdjustWindowRectExForDpi(&border, style, 0, styleEx, Settings::Dpi());
		RECT* rect = (RECT*)lParam;

		Settings::screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::Dpi());
		Settings::screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::Dpi());
		Settings::virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, Settings::Dpi());
		Settings::virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, Settings::Dpi());

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
		case SIZE_MINIMIZED: {
			Settings::focused = false;
			Settings::minimised = true;
			Audio::Unfocus();
		} break;
		case SIZE_RESTORED: {
			Settings::focused = true;
			Settings::minimised = false;
			Audio::Focus();

			//TODO: Fix cursor teleport
		} break;
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

		Settings::screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::Dpi());
		Settings::screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::Dpi());
		Settings::virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, Settings::Dpi());
		Settings::virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, Settings::Dpi());

		Settings::resized = true;
	} return 0;
	case WM_SIZING: {
		//TODO:
	} return 1;
	case WM_MOVE: {
		Settings::data.windowPositionX = LOWORD(lParam);
		Settings::data.windowPositionY = HIWORD(lParam);

		if (!Settings::Fullscreen())
		{
			Settings::data.windowPositionXSmall = Settings::WindowPositionX();
			Settings::data.windowPositionYSmall = Settings::WindowPositionY();
		}

		Settings::screenWidth = GetSystemMetricsForDpi(SM_CXSCREEN, Settings::Dpi());
		Settings::screenHeight = GetSystemMetricsForDpi(SM_CYSCREEN, Settings::Dpi());
		Settings::virtualScreenWidth = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, Settings::Dpi());
		Settings::virtualScreenHeight = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, Settings::Dpi());
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

bool Platform::ExecuteProcess(CSTR workingDirectory, CSTR processFullpath, CSTR arguments, CSTR searchErrorString)
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
	if (CreateProcessA(processFullpath, (char*)arguments, 0, 0, inheritHandles, 0, 0, workingDirectory, &startupInfo, &processInfo))
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

	if (strlen(searchErrorString) > 0 && strstr(processOutputBuffer, searchErrorString))
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

#endif