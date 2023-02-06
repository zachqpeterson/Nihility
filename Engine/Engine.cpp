#include "Engine.hpp"

#include "Memory\Memory.hpp"
#include "Platform\Platform.hpp"
#include "Platform\Input.hpp"
#include "Core\Logger.hpp"
#include "Core\Time.hpp"
#include "Core\String.hpp"
#include "Settings.hpp"

#include <string>

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;
bool Engine::suspended;

void Engine::Initialize(const W16* applicationName, InitializeFn init, UpdateFn update, ShutdownFn shutdown)
{
	GameInit = init;
	GameUpdate = update;
	GameShutdown = shutdown;

	running = true;
	suspended = false;

	ASSERT(Time::Initialize());
	Memory::Initialize(1024 * 1024 * 1024, 1024 * 1024 * 1024);
	Logger::Initialize();

	Timer t0;
	t0.Start();
	for (U32 i = 0; i < 1000000; ++i)
	{
		String str0((I8)-100);
		String str1((U8)255);
		String str2((I16)-10000);
		String str3((U16)65535);
		String str4((I32)-12312512);
		String str5((U32)12312512);
		String str6((I64)-232412516242);
		String str7((U64)232412516242);

		I8 b0 = str0.ToI8();
		U8 b1 = str1.ToU8();
		I16 b2 = str2.ToI16();
		U16 b3 = str3.ToU16();
		I32 b4 = str4.ToI32();
		U32 b5 = str5.ToU32();
		I64 b6 = str6.ToI64();
		U64 b7 = str7.ToU64();
		BreakPoint;
	}
	t0.Stop();

	//TODO: Load Settings, First time running or if the config is missing, get monitor Hz and dpi scaling

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

		if (!Platform::Update()) { running = false; break; }

		Input::Update();
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

			if (remainingNS > 0) { Platform::SleepFor(remainingNS - 5750); }
		}
		else
		{
			F64 remainingFrameTime = Settings::TargetFrametimeSuspended - Time::FrameUpTime();
			U64 remainingNS = (U64)(remainingFrameTime * 10000000.0);

			if (remainingNS > 0) { Platform::SleepFor(remainingNS - 5750); }
		}
	}
}