#include "Platform.hpp"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <windowsx.h>
#include <memory>
#undef ZeroMemory
#undef CopyMemory

struct PlatformState
{
    HINSTANCE hInstance;
    HWND hwnd;
    U32 clientX, clientY;
    U32 clientWidth;
    U32 clientHeight;
    U32 windowX, windowY;
    U32 windowWidth;
    U32 windowHeight;
};

static PlatformState* platformState;

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

bool Platform::Initialize(
    void* state,
    const char* application_name,
    I32 x,
    I32 y,
    I32 width,
    I32 height)
{
    platformState = (PlatformState*)state;
    platformState->hInstance = GetModuleHandleA(0);

    // Setup and register window class.
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = Win32MessageProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = platformState->hInstance;
    wc.hIcon = LoadIcon(platformState->hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "Nihility Window Class";

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(0, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Create window
    platformState->clientX = x;
    platformState->clientY = y;
    platformState->clientWidth = width;
    platformState->clientHeight = height;

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
    platformState->windowX = platformState->clientX + border_rect.left;
    platformState->windowY = platformState->clientY + border_rect.top;

    // Grow by the size of the OS border.
    platformState->windowWidth = platformState->clientWidth + border_rect.right - border_rect.left;
    platformState->windowHeight = platformState->clientHeight + border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(
        exStyle, "Nihility Window Class", application_name,
        style, platformState->windowX, platformState->windowY, platformState->windowWidth, platformState->windowHeight,
        0, 0, platformState->hInstance, 0);

    if (handle == nullptr) 
    {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        //TODO: logger
        //KFATAL("Window creation failed!");
        return false;
    }
    else 
    {
        platformState->hwnd = handle;
    }

    // Show the window
    bool should_activate = true;  //TODO: If the window should not accept input, this should be false.
    I32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    //TODO: If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    //TODO: If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE;
    ShowWindow(platformState->hwnd, show_window_command_flags);

    ClockSetup();

    return true;
}

void Platform::Shutdown()
{
    if (platformState && platformState->hwnd)
    {
        DestroyWindow(platformState->hwnd);
        platformState->hwnd = nullptr;
    }
}

bool Platform::ProcessMessages()
{
    MSG message;

    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return true;
}

const U64 Platform::GetMemoryRequirements()
{
    return sizeof(PlatformState);
}

void* Platform::Allocate(U64 size, bool aligned)
{
    return malloc(size);
}

void Platform::Free(void* block, bool aligned)
{
    free(block);
}

void* Platform::ZeroMemory(void* block, U64 size)
{
    return memset(block, 0, size);
}

void* Platform::CopyMemory(void* dest, const void* source, U64 size)
{
    return memcpy(dest, source, size);
}

void* Platform::SetMemory(void* dest, I32 value, U64 size)
{
    return memset(dest, value, size);
}

void Platform::ConsoleWrite(const char* message, U8 colour)
{
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static U8 levels[6] = { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(console_handle, levels[colour]);
    OutputDebugStringA(message);
    U64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void Platform::ConsoleWriteError(const char* message, U8 colour)
{
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static U8 levels[6] = { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(console_handle, levels[colour]);
    OutputDebugStringA(message);
    U64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
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

LRESULT CALLBACK Win32MessageProc(HWND hwnd, U32 msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
    case WM_SETFOCUS: /*TODO: Notify engine has focus*/ break;
    case WM_KILLFOCUS: /*TODO: Notify engine doesn't has focus*/ break;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: return 0;
    case WM_CLOSE: /*TODO: Notify engine to close*/ return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_SIZE: /*TODO: Notify engine to resize*/ break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        //TODO: Handle input
    } return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        //TODO: Handle input
    } break;
    case WM_MOUSEWHEEL:
    {
        //TODO: Handle input
    } break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif