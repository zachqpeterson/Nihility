#pragma once

#include "Defines.hpp"

class RendererFrontend
{
public:
    static bool Initialize(void* state);
    static void* Shutdown();

    static bool DrawFrame();

    static const U64 GetMemoryRequirements();

private:

    RendererFrontend() = delete;
};