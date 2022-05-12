#include "Platform.hpp"

#ifdef PLATFORM_WINDOWS
#include "Core/Logger.hpp"
#include "Core/Input.hpp"
#include "Core/Events.hpp"

#include <windows.h>
#include <windowsx.h>
#include <memory>
#undef ZeroMemory
#undef CopyMemory

#define WHEEL_MULTIPLIER 0.00833333333

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
    LOG_INFO("Initializing Platform.");

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

        LOG_FATAL("Window creation failed!");
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

void* Platform::Shutdown()
{
    if (platformState->hwnd)
    {
        DestroyWindow(platformState->hwnd);
        platformState->hwnd = nullptr;
    }

    return platformState;
}

bool Platform::ProcessMessages()
{
    MSG message;

    //TODO: See if you should pass NULL here V
    while (PeekMessageA(&message, platformState->hwnd, 0, 0, PM_REMOVE))
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

void Platform::ConsoleWrite(const char* message, U8 color)
{
    HANDLE console_handle = GetStdHandle(color < 2 ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static U8 levels[6] = { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    U64 length = strlen(message);
    LPDWORD number_written = 0;
    
    WriteConsoleA(console_handle, message, (DWORD)length, number_written, 0);
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
    ((HINSTANCE*)surfaceInfo)[0] = platformState->hInstance;
    ((HINSTANCE*)surfaceInfo)[0] = *(HINSTANCE*)&platformState->hwnd;
}

LRESULT CALLBACK Win32MessageProc(HWND hwnd, U32 msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
    case WM_SETFOCUS: /*TODO: Notify engine has focus*/ return 0;
    case WM_KILLFOCUS: /*TODO: Notify engine doesn't have focus*/ return 0;
    case WM_ERASEBKGND: return 1;
    case WM_CLOSE: Events::Notify("CLOSE", nullptr); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_SIZE: /*TODO: Notify engine to resize*/ return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        U8 code = (U8)w_param;

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
        Input::SetButtonState((U8)w_param, (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN));
    } break;
    case WM_MOUSEWHEEL:
    {
        Input::SetMouseWheel(GET_WHEEL_DELTA_WPARAM(w_param) * WHEEL_MULTIPLIER);
    } break;
    case WM_MOUSEMOVE:
    {
        Input::SetMousePos(GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
    } break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif