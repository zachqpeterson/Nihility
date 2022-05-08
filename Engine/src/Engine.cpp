#include "Engine.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Core/Logger.hpp"
#include "Core/Input.hpp"
#include "Core/Events.hpp"
#include "Containers/Vector.hpp"
#include "Containers/String.hpp"

void Engine::Initialize()
{
    Memory::Initialize(Gigabytes(1));

    Logger::Initialize(Memory::Allocate(Logger::GetMemoryRequirements(), MEMORY_TAG_APPLICATION));

    Platform::Initialize(Memory::Allocate(Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION), "TEST", 100, 100, 1280, 720);

    Input::Initialize(Memory::Allocate(Input::GetMemoryRequirements(), MEMORY_TAG_APPLICATION));

    Events::Subscribe("CLOSE", OnClose);

    Memory::GetMemoryStats();

    MainLoop();
}

void Engine::MainLoop()
{
    running = true;

    while(running)
    {
        Platform::ProcessMessages();
        if(Input::OnButtonDown(ESCAPE))
        {
            running = false;
        }
    }

    Shutdown();
}

void Engine::Shutdown()
{
    Memory::Free(Input::Shutdown(), Input::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);

    Memory::Free(Platform::Shutdown(), Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);

    Memory::Free(Logger::Shutdown(), Logger::GetMemoryRequirements(), MEMORY_TAG_APPLICATION);

    Memory::Shutdown();
}

bool Engine::OnClose(void* data)
{
    running = false;
    return true;
}