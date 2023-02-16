#include "Engine.hpp"

#include "Memory\Memory.hpp"
#include "Platform\Platform.hpp"
#include "Platform\Input.hpp"
#include "Platform\Jobs.hpp"
#include "Core\Logger.hpp"
#include "Core\Time.hpp"
#include "Core\Settings.hpp"
#include "Containers\String.hpp"
#include "Containers\Vector.hpp"
#include "Containers\Queue.hpp"

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;
bool Engine::suspended;

void work(void* data)
{
	Logger::Info((char*)data);
}

void Engine::Initialize(const W16* applicationName, InitializeFn init, UpdateFn update, ShutdownFn shutdown)
{
	GameInit = init;
	GameUpdate = update;
	GameShutdown = shutdown;

	running = true;
	suspended = false;

	ASSERT(Time::Initialize());
	ASSERT(Memory::Initialize());
	ASSERT(Logger::Initialize());

	Logger::Fatal("Test");
	Logger::Error("Test");
	Logger::Warn("Test");
	Logger::Info("Test");
	Logger::Debug("Test");
	Logger::Trace("Test");

	//TODO: Load Settings, First time running or if the config is missing, get monitor Hz and dpi scaling

	ASSERT(Jobs::Initialize());
	ASSERT(Platform::Initialize(applicationName));
	ASSERT(Input::Initialize());

	ASSERT(GameInit());

	UpdateLoop();

	Shutdown();
}

void Engine::Shutdown()
{
	GameShutdown();

	Platform::Shutdown();
}

void Engine::UpdateLoop()
{
	while (running)
	{
		Time::Update();
		Logger::Info("Framerate: {}", Time::FrameRate());

		if (!Platform::Update()) { break; } //TODO: Run on separate thread

		Input::Update(); //TODO: Run on separate thread
		//Physics::Update();
		GameUpdate();
		//UI::Update();
		//Animations::Update();
		//Renderer::Update(); //TODO: Checks for being minimised
		//Audio::Update();

		if (Settings::Focused)
		{
			F64 remainingFrameTime = Settings::TargetFrametime - Time::FrameUpTime();
			U64 remainingUS = (U64)(remainingFrameTime * 1090000.0);

			if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }
		}
		else
		{
			F64 remainingFrameTime = Settings::TargetFrametimeSuspended - Time::FrameUpTime();
			U64 remainingUS = (U64)(remainingFrameTime * 1090000.0);

			if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }
		}
	}
}

