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

	Logger::Fatal("Hello, World!");
	Logger::Error("Hello, World!");
	Logger::Warn("Hello, World!");
	Logger::Info("Hello, World!");
	Logger::Debug("Hello, World!");
	Logger::Trace("Hello, World!");

	F32 f = 123.123f;
	//TODO: Only works with decimal count of 5 or 0
	String s512("{.3}", f);

	String path("test.txt");
	File file(path, FILE_OPEN_WRITE | FILE_OPEN_APPEND | FILE_OPEN_SEQUENTIAL | FILE_OPEN_BINARY);

	String str0("Hello, World!");
	//String str1;
	file.Write(str0);
	file.Write(str0);
	file.Write(str0);
	file.Write(str0);
	file.Write(str0);
	file.Write(str0);
	file.Close();
	//I32 i1 = file.ReadCount(str1, 1);

	ASSERT(Settings::Initialize());
	ASSERT(Jobs::Initialize());
	ASSERT(Platform::Initialize(applicationName));	//Thread
	ASSERT(Input::Initialize());					//Probably run on platform thread
	ASSERT(Resources::Initialize());
	//Audio
	//Renderer
	//UI
	//Physics
	//Particle
	ASSERT(GameInit());

	UpdateLoop();

	Shutdown();
}

void Engine::Shutdown()
{
	GameShutdown();
	//Particle
	//Physics
	//UI
	//Renderer
	Resources::Shutdown();
	//Audio
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

		if (!Platform::Update()) { break; } //TODO: Run on separate thread

		//Physics::Update();
		GameUpdate();
		//UI::Update();
		//Animations::Update();
		//Renderer::Update(); //TODO: Checks for being minimised
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

