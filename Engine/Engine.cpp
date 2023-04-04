#include "Engine.hpp"

#include "Memory\Memory.hpp"
#include "Platform\Platform.hpp"
#include "Platform\Input.hpp"
#include "Platform\Jobs.hpp"
#include "Core\Logger.hpp"
#include "Core\Time.hpp"
#include "Core\File.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"
#include "Containers\String.hpp"
#include "Containers\Vector.hpp"
#include "Containers\Queue.hpp"
#include "Rendering\Renderer.hpp"

#include <stdio.h>

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;
bool Engine::suspended;

void work(void* data)
{
	Logger::Info((C8*)data);
}

void Engine::Initialize(const C8* applicationName, InitializeFn init, UpdateFn update, ShutdownFn shutdown)
{
	GameInit = init;
	GameUpdate = update;
	GameShutdown = shutdown;

	running = true;
	suspended = false;

	ASSERT(Time::Initialize());
	ASSERT(Memory::Initialize());
	ASSERT(Logger::Initialize());

	F32 f = 123.123f;
	//TODO: Only works with decimal count of 5 or 0
	//String s512("{.3}", f);

	ASSERT(Settings::Initialize());
	ASSERT(Jobs::Initialize());
	ASSERT(Platform::Initialize(applicationName));	//Thread
	ASSERT(Input::Initialize());					//Probably run on platform thread
	ASSERT(Resources::Initialize());
	//Audio
	ASSERT(Renderer::Initialize(applicationName));
	//UI
	//Physics
	//Particle
	ASSERT(GameInit());

	Platform::HideCursor(true);

	UpdateLoop();

	Shutdown();
}

void Engine::Shutdown()
{
	GameShutdown();
	//Particle
	//Physics
	//UI
	Renderer::Shutdown();
	//Audio
	Resources::Shutdown();
	Input::Shutdown();
	Platform::Shutdown();
	Jobs::Shutdown();
	Settings::Shutdown();
	Logger::Shutdown();
	Memory::Shutdown();
	Time::Shutdown();
}

void Engine::UpdateLoop()
{
	while (running)
	{
		Time::Update();
		//Logger::Info("Framerate: {}", Time::FrameRate());

		Input::Update();
		if (!Platform::Update()) { break; } //TODO: Run on separate thread

		//Physics::Update();
		GameUpdate();
		//UI::Update();
		//Animations::Update();
		Renderer::Update(); //TODO: Checks for being minimised
		//Audio::Update();

		if (Settings::Focused())
		{
			F64 remainingFrameTime = Settings::TargetFrametime() - Time::FrameUpTime();
			U64 remainingUS = (U64)(remainingFrameTime * 1090000.0);

			if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }
		}
		else
		{
			F64 remainingFrameTime = Settings::TargetFrametimeSuspended() - Time::FrameUpTime();
			U64 remainingUS = (U64)(remainingFrameTime * 1090000.0);

			if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }
		}
	}
}

