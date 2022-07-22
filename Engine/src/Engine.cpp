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
#include "Physics/Physics.hpp"
#include "Audio/Audio.hpp"

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

	ASSERT(Memory::Initialize(Gigabytes(1)));
	ASSERT(Logger::Initialize());

	Resources::LoadSettings();

	ASSERT_MSG(Platform::Initialize(applicationName), "Platform layer failed to initialize!");

	ASSERT_MSG(Input::Initialize(), "Input system failed to initialize!");

	ASSERT_MSG(RendererFrontend::Initialize(applicationName), "Renderer system failed to initialize!");

	ASSERT_MSG(Resources::Initialize(), "Resource system failed to initialize!");

	//TODO: Load all materials
	Resources::CreateShaders();

	ASSERT_MSG(UI::Initialize(), "UI system failed to initialize!");

	ASSERT_MSG(Physics::Initialize(), "Physics system failed to initialize!");

	ASSERT_MSG(Audio::Initialize(), "Audio system failed to initialize!");

	ASSERT_MSG(GameInit(), "Game failed to initialize!");

	ASSERT_MSG(Time::Initialize(), "Time system failed to initialize!");

	Events::Subscribe("CLOSE", OnClose);

	Memory::GetMemoryStats();

	MainLoop();
}

void Engine::MainLoop()
{
	running = true;
	suspended = false;

	F64 accumulatedTime = 0.0f;

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
			accumulatedTime += Settings::TargetFrametime;

			while (accumulatedTime >= 0)
			{
				Time::Update();

				Physics::Update();

				running = GameUpdate();

				//OTHER UPDATES
				Audio::Update();

				accumulatedTime -= Time::TimeSinceLastFrame();
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

	Audio::Shutdown();

	Physics::Shutdown();

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