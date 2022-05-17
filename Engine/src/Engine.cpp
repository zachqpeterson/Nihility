#include "Engine.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Core/Input.hpp"
#include "Core/Events.hpp"
#include "Containers/Vector.hpp"
#include "Containers/String.hpp"
#include "Renderer/ShaderSystem.hpp"
#include "Renderer/RendererFrontend.hpp"

bool Engine::running;
bool Engine::suspended;

void Engine::Initialize()
{
    Memory::Initialize(Gigabytes(1));

    Logger::Initialize(Memory::Allocate(Logger::GetMemoryRequirements(), MEMORY_TAG_APPLICATION));

    Platform::Initialize(Memory::Allocate(Platform::GetMemoryRequirements(), MEMORY_TAG_APPLICATION), "TEST", 100, 100, 1280, 720);

    Input::Initialize(Memory::Allocate(Input::GetMemoryRequirements(), MEMORY_TAG_APPLICATION));

    Time::Initialize();

    ShaderSystem::Initialize();

    RendererFrontend::Initialize();

    Events::Subscribe("CLOSE", OnClose);

    //TODO: Remove later
    Memory::GetMemoryStats();

    MainLoop();
}

void Engine::MainLoop()
{
    running = true;
    suspended = false;

    F64 accumulatedTime = 0.0f;
    F64 lastUpTime = Time::UpTime();
    F64 upTime = lastUpTime;

    while (running)
    {
        upTime = Time::UpTime();
        accumulatedTime += upTime - lastUpTime;
        lastUpTime = upTime;

        //0.00694444444	| 144
        //0.00833333333	| 120
        //0.01666666667	| 60
        //0.03333333333	| 30
        while (accumulatedTime >= 0.00833333333)
        {
            Platform::ProcessMessages();

            if (Input::OnButtonDown(ESCAPE))
            {
                running = false;
            }

            if (!suspended)
            {
                Time::Update();

                //UPDATES
            }

            accumulatedTime -= 0.00833333333;
        }

        RendererFrontend::DrawFrame();

        //LOG_DEBUG("Frame Rate: %d", Time::FrameRate());
    }

    Shutdown();
}

void Engine::Shutdown()
{
    RendererFrontend::Shutdown();

    ShaderSystem::Shutdown();

    Time::Shutdown();

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