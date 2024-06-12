#include "Engine.hpp"

import Containers;
import Math;

#include "Introspection.hpp"
#include "Memory\Memory.hpp"
#include "Platform\Platform.hpp"
#include "Platform\Audio.hpp"
#include "Platform\Input.hpp"
#include "Platform\Jobs.hpp"
#include "Core\Events.hpp"
#include "Core\Function.hpp"
#include "Core\Logger.hpp"
#include "Core\Time.hpp"
#include "Core\File.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"
#include "Containers\Queue.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"
#include "Math\Physics.hpp"
#include "Networking\Discord.hpp"
#include "Networking\Steam.hpp"

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;

void Engine::Initialize(CSTR applicationName, U32 applicationVersion, U32 steamAppId, InitializeFn init, UpdateFn update, ShutdownFn shutdown)
{
	GameInit = init;
	GameUpdate = update;
	GameShutdown = shutdown;

	running = true;

	ASSERT(Time::Initialize());
	ASSERT(Memory::Initialize());
	ASSERT(Events::Initialize());
	ASSERT(Logger::Initialize());
	ASSERT(Settings::Initialize());
	ASSERT(Jobs::Initialize());
	ASSERT(Physics::Initialize());
	ASSERT(Platform::Initialize(applicationName));
	ASSERT(Renderer::Initialize(applicationName, applicationVersion));
	ASSERT(Resources::Initialize());
	ASSERT(UI::Initialize());

	//Particles
	ASSERT(Audio::Initialize());
	ASSERT(Input::Initialize());
	ASSERT(Steam::Initialize(steamAppId));
	Discord::Initialize(applicationName);

	Logger::Trace("Initializing Game...");
	ASSERT(GameInit());

	Renderer::InitialSubmit();

	UpdateLoop();

	Shutdown();
}

void Engine::Shutdown()
{
	Logger::Trace("Shutting Down Game...");
	GameShutdown();
	Discord::Shutdown();
	Steam::Shutdown();
	Audio::Shutdown();
	//Particles
	UI::Shutdown();
	Resources::Shutdown();
	Renderer::Shutdown();
	Input::Shutdown();
	Platform::Shutdown();
	Physics::Shutdown();
	Jobs::Shutdown();
	Settings::Shutdown();
	Logger::Shutdown();
	Events::Shutdown();
	Memory::Shutdown();
	Time::Shutdown();
}

void Engine::UpdateLoop()
{
	F64 timeAccumulation = 0.0;
	while (running)
	{
		Time::Update();
		Input::Update();

		if (!Platform::Update() || Input::OnButtonDown(BUTTON_CODE_ESCAPE)) { break; } //TODO: Separate Thread

		if (Input::OnButtonDown(BUTTON_CODE_F11))
		{
			Platform::SetFullscreen(!Settings::Fullscreen());
		}

#ifdef NH_DEBUG
		if (Input::OnButtonDown(BUTTON_CODE_F5))
		{
			Settings::inEditor = !Settings::inEditor;
		}
#endif

		bool runFrame = false;
		if (!Settings::Minimised()) { runFrame = Renderer::BeginFrame(); }

		Physics::Update((F32)Time::DeltaTime()); //TODO: constant step
		
		GameUpdate();
		//Animations::Update();

		Audio::Update();

		if (runFrame)
		{
			UI::Update();
			Renderer::EndFrame();
		}

		Steam::Update();
		Discord::Update();

		F64 remainingFrameTime;

		if (Settings::Focused()) { remainingFrameTime = Settings::TargetFrametime() - Time::FrameUpTime(); }
		else { remainingFrameTime = Settings::TargetFrametimeSuspended() - Time::FrameUpTime(); }

		U64 remainingUS = (U64)(remainingFrameTime * 990000.0);

		if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }
	}
}

