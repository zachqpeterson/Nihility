#include "Engine.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"

bool Engine::Initialize()
{
    Memory::Initialize(Gigabytes(1));

    Logger::Initialize(Memory::Allocate(Logger::GetMemoryRequirements(), MEMORY_TAG_APPLICATION));

    Platform::Initialize(Memory::Allocate(Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION), "TEST", 100, 100, 1280, 720);

    FATAL("testing %d, %d, %d", 1, 2, 3);
    ERROR("testing %d, %d, %d", 1, 2, 3);
    WARN("testing %d, %d, %d", 1, 2, 3);
    INFO("testing %d, %d, %d", 1, 2, 3);
    DEBUG("testing %d, %d, %d", 1, 2, 3);
    TRACE("testing %d, %d, %d", 1, 2, 3);

    return true;
}

void Engine::Shutdown()
{
    Memory::Free(Platform::Shutdown(), Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);

    Memory::Free(Logger::Shutdown(), Logger::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);
    
    Memory::Shutdown();
}