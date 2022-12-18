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
#include "Physics/BoxTree.hpp"
#include "Audio/Audio.hpp"
#include "Resources/Animations.hpp"

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
	ASSERT_MSG(RendererFrontend::Initialize(applicationName), "Renderer system failed to initialize!");
	ASSERT_MSG(Resources::Initialize(), "Resource system failed to initialize!");
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

	F64 accumulatedTime = 0.0;
	F64 step = Time::UpTime();
	F64 lastStep = step;

#ifdef NH_DEBUG
	UIElementConfig config{};
	config.position = { 0.98f , 0.0f };
	config.scale = { 0.02f, 0.02f };
	config.color = { 0.0f, 1.0f, 0.0f, 1.0f };
	config.enabled = true;
	config.ignore = true;
	config.scene = (Scene*)RendererFrontend::CurrentScene();
	UIText* fpsCounter = UI::GenerateText(config, "60", 12);
#endif

	while (running)
	{
		Time::Update();
#ifdef NH_DEBUG
		UI::ChangeText(fpsCounter, Time::FrameRate());
		UI::ChangeScene(fpsCounter);
#endif
		accumulatedTime = Math::Min(Settings::TargetFrametime + accumulatedTime, 0.1);

		running = Platform::Update() && !Input::OnButtonDown(ESCAPE);

		if (Input::OnButtonDown(Y))
		{
			Memory::GetMemoryStats();
		}

		if (!suspended && running)
		{
			Audio::Update();
			UI::Update();
			Physics::Update(Math::Min((F32)Time::DeltaTime(), 0.1f));
			running = GameUpdate();
			Animations::Update();
			RendererFrontend::DrawFrame();

			F64 remaining = Settings::TargetFrametime - Time::TimeSinceLastFrame();

			Timer frameTimer;
			frameTimer.Start();
			while (remaining - frameTimer.CurrentTime() > 0.0001);
		}
	}

	Shutdown();
}

void Engine::Shutdown()
{
	GameCleanup();

	Audio::Shutdown();
	Physics::Shutdown();
	UI::Shutdown();
	Animations::Shutdown();
	Resources::Shutdown();
	RendererFrontend::Shutdown();
	Platform::Shutdown();

	Memory::GetMemoryStats();

	Logger::Shutdown();
	Resources::WriteSettings();
	Events::Shutdown();
	Memory::Shutdown();
}

bool Engine::OnClose(void* data)
{
	running = false;
	return true;
}