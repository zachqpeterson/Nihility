#include "Platform.hpp"

#ifdef PLATFORM_WINDOWS
#include "Core/Logger.hpp"
#include "Core/Input.hpp"
#include "Core/Events.hpp"
#include "Math/Math.hpp"
#include "Containers/String.hpp"

#include <windows.h>
#include <windowsx.h>
#include <memory>

#define WHEEL_MULTIPLIER 0.00833333333

struct PlatformState
{
    HINSTANCE hInstance;
    HWND hwnd;
    U32 clientX, clientY;
    Vector2Int clientSize;
    U32 windowX, windowY;
    U32 windowWidth;
    U32 windowHeight;
};

PlatformState Platform::platformState;

static F64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK Win32MessageProc(HWND hwnd, U32 msg, WPARAM w_param, LPARAM l_param);

void ClockSetup()
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / (F64)frequency.QuadPart;
    QueryPerformanceCounter(&startTime);
}

bool Platform::Initialize(const String& applicationName,
    I32 x, I32 y, I32 width, I32 height)
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

    // Create window
    platformState.clientX = x;
    platformState.clientY = y;
    platformState.clientSize.x = width;
    platformState.clientSize.y = height;

    //TODO: Change with config
    U32 style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    U32 exStyle = WS_EX_APPWINDOW;

    style |= WS_MAXIMIZEBOX;
    style |= WS_MINIMIZEBOX;
    style |= WS_THICKFRAME;

    // Obtain the size of the border.
    RECT border_rect = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&border_rect, style, 0, exStyle);

    // In this case, the border rectangle is negative.
    platformState.windowX = platformState.clientX + border_rect.left;
    platformState.windowY = platformState.clientY + border_rect.top;

    // Grow by the size of the OS border.
    platformState.windowWidth = platformState.clientSize.x + border_rect.right - border_rect.left;
    platformState.windowHeight = platformState.clientSize.y + border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(
        exStyle, "Nihility Window Class", applicationName,
        style, platformState.windowX, platformState.windowY, platformState.windowWidth, platformState.windowHeight,
        0, 0, platformState.hInstance, 0);

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

    // Show the window
    bool should_activate = true;  //TODO: If the window should not accept input, this should be false.
    I32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    //TODO: If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    //TODO: If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE;
    ShowWindow(platformState.hwnd, show_window_command_flags);

    ClockSetup();

    return true;
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
    MSG message;

    //TODO: See if you should pass NULL here V
    while (PeekMessageA(&message, platformState.hwnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return true;
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
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
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

void Platform::SleepFor(U64 ms)
{
    Sleep(ms);
}

void Platform::GetVulkanSurfaceInfo(void* surfaceInfo)
{
    //TODO: Find a better way for this
    ((HINSTANCE*)surfaceInfo)[0] = platformState.hInstance;
    ((HINSTANCE*)surfaceInfo)[0] = *(HINSTANCE*)&platformState.hwnd;
}

LRESULT CALLBACK Win32MessageProc(HWND hwnd, U32 msg, WPARAM wParam, LPARAM lParam)
{
    static Vector2Int size;

    switch (msg)
    {
    case WM_SETFOCUS: /*TODO: Notify engine has focus*/ return 0;
    case WM_KILLFOCUS: /*TODO: Notify engine doesn't have focus*/ return 0;
    case WM_ERASEBKGND: return 1;
    case WM_CLOSE: Events::Notify("CLOSE", nullptr); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_SIZE:
    {
        size = { LOWORD(lParam), HIWORD(lParam) };
        Events::Notify("Resize", (void*)&size);
    } return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        U8 code = (U8)wParam;

        //TODO: Handle left and right menu keys

        Input::SetButtonState(code, pressed);
    } return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        Input::SetButtonState((U8)wParam, (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN));
    } break;
    case WM_MOUSEWHEEL:
    {
        Input::SetMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) * WHEEL_MULTIPLIER);
    } break;
    case WM_MOUSEMOVE:
    {
        Input::SetMousePos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    } break;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#endif