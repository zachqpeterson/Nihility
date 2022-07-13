#include "Engine.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Core/Input.hpp"
#include "Core/Events.hpp"
#include "Core/Settings.hpp"
#include <Containers/Vector.hpp>
#include "Containers/String.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Resources/Resources.hpp"
#include "Resources/UI.hpp"

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
CleanupFn Engine::GameCleanup;

bool Engine::running;
bool Engine::suspended;

void Engine::Initialize(const char* applicationName, InitializeFn init, UpdateFn update, CleanupFn cleanup)
{
	GameInit = init;
	GameUpdate = update;
	GameCleanup = cleanup;

	Memory::Initialize(Gigabytes(1));

	Logger::Initialize();

	Resources::LoadSettings();

	Platform::Initialize("TEST");

	Input::Initialize();

	RendererFrontend::Initialize(applicationName);

	Resources::Initialize();

	//TODO: Load all materials
	Resources::CreateShaders();

	UI::Initialize();

	GameInit();

	Time::Initialize();

	Events::Subscribe("CLOSE", OnClose);

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
		running = Platform::ProcessMessages();

		if (Input::OnButtonDown(ESCAPE))
		{
			running = false;
			break;
		}

		if (!suspended && running)
		{
			Time::Update();
			upTime = Time::UpTime();
			accumulatedTime += upTime - lastUpTime;
			lastUpTime = upTime;

			while (accumulatedTime >= Settings::TargetFrametime)
			{
				//PHYSICS

				running = GameUpdate();

				//OTHER UPDATES

				accumulatedTime -= Settings::TargetFrametime;
			}

			RendererFrontend::DrawFrame();
		}
	}

	Shutdown();
}

void Engine::Shutdown()
{
	GameCleanup();

	Time::Shutdown();

	UI::Shutdown();

	Resources::Shutdown();

	RendererFrontend::Shutdown();

	Input::Shutdown();

	Platform::Shutdown();

	Logger::Shutdown();

	Resources::WriteSettings();

	Events::Shutdown();

	Memory::GetMemoryStats();

	Memory::Shutdown();
}

bool Engine::OnClose(void* data)
{
	running = false;
	return true;
}