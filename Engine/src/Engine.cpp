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

	ASSERT_MSG(UI::Initialize(), "UI system failed to initialize!");

	ASSERT_MSG(Physics::Initialize(new BoxTree()), "Physics system failed to initialize!");

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

	Audio::ClearBuffer();
	Audio::Start();
	//Audio::PlayAudio("TPOM.wav", AUDIO_TYPE_MUSIC, 1.0f, 1.0f, true);

	while (running)
	{
		Time::Update();
		//Logger::Debug(Time::FrameRate());
		//Logger::Debug(Time::DeltaTime());
		accumulatedTime = Math::Min(Settings::TargetFrametime + accumulatedTime, 0.1);

		running = Platform::ProcessMessages() && !Input::OnButtonDown(ESCAPE);

		if (Input::OnButtonDown(G))
		{
			Audio::PlayAudioSpacial("Squeal.wav", AUDIO_TYPE_SFX, { -2.0f, 11.0f });
		}
		if (Input::OnButtonDown(H))
		{
			Audio::PlayAudioSpacial("Squeal.wav", AUDIO_TYPE_SFX, { -1.1f, 9.0f });
		}
		if (Input::OnButtonDown(J))
		{
			Audio::PlayAudioSpacial("Squeal.wav", AUDIO_TYPE_SFX, { 0.0f, 9.0f });
		}
		if (Input::OnButtonDown(K))
		{
			Audio::PlayAudioSpacial("Squeal.wav", AUDIO_TYPE_SFX, { 1.1f, 9.0f });
		}
		if (Input::OnButtonDown(L))
		{
			Audio::PlayAudioSpacial("Squeal.wav", AUDIO_TYPE_SFX, { 2.0f, 11.0f });
		}
		if (Input::OnButtonDown(N))
		{
			Audio::PlayAudioSpacial("Squeal.wav", AUDIO_TYPE_SFX, { 0.0f, 15.0f });
		}

		if (!suspended && running)
		{
			Audio::Update();
			Physics::Update((F32)Time::DeltaTime());
			running = GameUpdate();
			RendererFrontend::DrawFrame();

			F64 remaining = Settings::TargetFrametime - Time::TimeSinceLastFrame();

			Timer timer;
			timer.Start();
			while (remaining - timer.CurrentTime() > 0.0001);
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

	Memory::Shutdown();
}

bool Engine::OnClose(void* data)
{
	running = false;
	return true;
}