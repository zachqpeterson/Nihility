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

	U64 size = sizeof(Memory::Region1kb);

	//TODO: Load Settings, First time running or if the config is missing, get monitor Hz and dpi scaling

	ASSERT(Jobs::Initialize());

	Jobs::StartJob(work, (char *)"String 0");
	Jobs::StartJob(work, (char *)"String 1");
	Jobs::StartJob(work, (char *)"String 2");
	Jobs::StartJob(work, (char *)"String 3");
	Jobs::StartJob(work, (char *)"String 4");
	Jobs::StartJob(work, (char *)"String 5");
	Jobs::StartJob(work, (char *)"String 6");
	Jobs::StartJob(work, (char *)"String 7");
	Jobs::StartJob(work, (char *)"String 8");
	Jobs::StartJob(work, (char *)"String 9");
	Jobs::StartJob(work, (char *)"String 10");
	Jobs::StartJob(work, (char *)"String 11");
	Jobs::StartJob(work, (char *)"String 12");
	Jobs::StartJob(work, (char *)"String 13");
	Jobs::StartJob(work, (char *)"String 14");
	Jobs::StartJob(work, (char *)"String 15");
	Jobs::StartJob(work, (char *)"String 16");
	Jobs::StartJob(work, (char *)"String 17");
	Jobs::StartJob(work, (char *)"String 18");
	Jobs::StartJob(work, (char *)"String 19");
	Jobs::StartJob(work, (char *)"String 20");
	Jobs::StartJob(work, (char *)"String 21");
	Jobs::StartJob(work, (char *)"String 22");
	Jobs::StartJob(work, (char *)"String 23");
	Jobs::StartJob(work, (char *)"String 24");
	Jobs::StartJob(work, (char *)"String 25");
	Jobs::StartJob(work, (char *)"String 26");
	Jobs::StartJob(work, (char *)"String 27");
	Jobs::StartJob(work, (char *)"String 28");
	Jobs::StartJob(work, (char *)"String 29");

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

