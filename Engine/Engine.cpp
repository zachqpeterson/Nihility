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
		String str0(18446744073709551615ui64);
		String str1(-9223372036854775808i64);
		String str2(4294967295ui32);
		String str3(-2147483648i32);
		String str4(65535ui16);
		String str5((I16)-32767i16);
		String str6(255ui8);
		String str7((I8)-127i8);
		String str8(true);
		String str9(false);
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