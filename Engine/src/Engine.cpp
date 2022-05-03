#include "Engine.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"

bool Engine::Initialize()
{
    Memory::Initialize(Gigabytes(1));

    Platform::Initialize(Memory::Allocate(Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION), "TEST", 100, 100, 1280, 720);

    return true;
}

void Engine::Shutdown()
{
    Memory::Free(Platform::Shutdown(), Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);
    
    Memory::Shutdown();
}