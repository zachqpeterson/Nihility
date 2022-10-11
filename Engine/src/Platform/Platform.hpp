#pragma once

#include "Defines.hpp"

struct PlatformState;
struct Vector2Int;

class NH_API Platform
{
public:
    static bool Initialize(const struct String& applicationName);
    static void Shutdown();
    static void ClockSetup();

    static bool ProcessMessages();

    static void* Allocate(U64 size, bool aligned);
    static void  Free(void* block, bool aligned);
    static void* Zero(void* block, U64 size);
    static void* Copy(void* dest, const void* source, U64 size);
    static void* Set(void* dest, I32 value, U64 size);

    static void ConsoleWrite(const struct String& message, U8 color);
    static const F64 AbsoluteTime();

    static void GetVulkanSurfaceInfo(void* surfaceInfo);

    static void SetFullscreen(bool fullscreen);
    static void LockMouse(bool lock);

    static const Vector2Int& ScreenSize();

    static const void* Handle();

private:
#ifdef PLATFORM_WINDOWS
    static I64 __stdcall Win32MessageProc(struct HWND__* hwnd, U32 msg, U64 wParam, I64 lParam);
#endif

    Platform() = delete;

    static PlatformState platformState;
};