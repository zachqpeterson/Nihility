#pragma once

#include "Defines.hpp"
#undef ZeroMemory
#undef CopyMemory

class Platform
{
public:
    static bool Initialize(
        void* state,
        const char* application_name,
        I32 x,
        I32 y,
        I32 width,
        I32 height);
    static void* Shutdown();

    static bool ProcessMessages();

    static const U64 GetMemoryRequirements();
    static void* Allocate(U64 size, bool aligned);
    static void Free(void* block, bool aligned);
    static void* ZeroMemory(void* block, U64 size);
    static void* CopyMemory(void* dest, const void* source, U64 size);
    static void* SetMemory(void* dest, I32 value, U64 size);

    static void ConsoleWrite(const char* message, U8 color);
    static const F64 AbsoluteTime();
    static void SleepFor(U64 ms);

    static void GetVulkanSurfaceInfo(void* surfaceInfo);

private:
    Platform() = delete;
};