#include "Engine.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"
#include "Core/Input.hpp"
#include "Containers/Vector.hpp"

bool Engine::Initialize()
{
    Memory::Initialize(Gigabytes(1));

    Logger::Initialize(Memory::Allocate(Logger::GetMemoryRequirements(), MEMORY_TAG_APPLICATION));

    Platform::Initialize(Memory::Allocate(Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION), "TEST", 100, 100, 1280, 720);

    Input::Initialize(Memory::Allocate(Input::GetMemoryRequirements(), MEMORY_TAG_APPLICATION));

    //TODO: Fix this
    Memory::GetMemoryStats();

    return true;
}

void Engine::Shutdown()
{
    Memory::Free(Input::Shutdown(), Input::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);

    Memory::Free(Platform::Shutdown(), Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);

    Memory::Free(Logger::Shutdown(), Logger::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);

    Memory::Shutdown();
}