#pragma once

#include "Defines.hpp"

struct HINSTANCE__;
struct HWND__;
struct HICON__;
struct String;

class NH_API Platform
{
public:
    static bool Initialize(const String& applicationName);
    static void Shutdown();
    static bool Update();

    static void* Allocate(U64 size, bool aligned);
    static void  Free(void* block, bool aligned);
    static void* Zero(void* block, U64 size);
    static void* Copy(void* dest, const void* source, U64 size);
    static void* Set(void* dest, I32 value, U64 size);

    static void SetFullscreen(bool fullscreen);
    static void SleepFor(U64 ns);

    static void GetVulkanSurfaceInfo(void* surfaceInfo);

private:
    static void UpdateMouse();

    static I32 windowX;
    static I32 windowY;
    static I32 windowWidth;
    static I32 windowHeight;
    static I32 screenWidth;
    static I32 screenHeight;
    static bool running;

#if defined PLATFORM_WINDOWS
    static I64 __stdcall Win32MessageProc(HWND__* hwnd, U32 msg, U64 wParam, I64 lParam);

    static HINSTANCE__* instance;
    static HWND__* window;
    static U32 style;

    static HICON__* arrow;
    static HICON__* hand;
    static HICON__* sizeNS;
    static HICON__* sizeWE;
    static HICON__* sizeNESW;
    static HICON__* sizeNWSE;
#elif defined PLATFORM_LINUX
#elif defined PLATFORM_APPLE
#endif

    Platform() = delete;
};