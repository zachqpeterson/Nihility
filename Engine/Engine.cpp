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

void work(String str)
{
	Logger::Info(str);
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

	U64 size = sizeof(Memory::Region1kb);

	//TODO: Load Settings, First time running or if the config is missing, get monitor Hz and dpi scaling

	ASSERT(Jobs::Initialize());

	Jobs::StartJob(work, "String 0");
	Jobs::StartJob(work, "String 1");
	Jobs::StartJob(work, "String 2");
	Jobs::StartJob(work, "String 3");
	Jobs::StartJob(work, "String 4");
	Jobs::StartJob(work, "String 5");
	Jobs::StartJob(work, "String 6");
	Jobs::StartJob(work, "String 7");
	Jobs::StartJob(work, "String 8");
	Jobs::StartJob(work, "String 9");

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
			U64 remainingNS = (U64)(remainingFrameTime * 10000000.0);

			if (remainingNS > 0) { Jobs::SleepFor(remainingNS - 5750); }
		}
		else
		{
			F64 remainingFrameTime = Settings::TargetFrametimeSuspended - Time::FrameUpTime();
			U64 remainingNS = (U64)(remainingFrameTime * 10000000.0);

			if (remainingNS > 0) { Jobs::SleepFor(remainingNS - 5750); }
		}
	}
}

