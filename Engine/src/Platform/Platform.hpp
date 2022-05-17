#pragma once

#include "Defines.hpp"

struct PlatformState;

class Platform
{
public:
    static bool Initialize(const struct String& application_name,
        I32 x, I32 y, I32 width, I32 height);
    static void Shutdown();

    static bool ProcessMessages();

    static void* Allocate(U64 size, bool aligned);
    static void  Free(void* block, bool aligned);
    static void* Zero(void* block, U64 size);
    static void* Copy(void* dest, const void* source, U64 size);
    static void* Set(void* dest, I32 value, U64 size);

    static void ConsoleWrite(const char* message, U8 color);
    static const F64 AbsoluteTime();
    static void SleepFor(U64 ms);

    static void GetVulkanSurfaceInfo(void* surfaceInfo);

private:
    Platform() = delete;

    static PlatformState platformState;
};