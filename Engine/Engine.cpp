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
#include "Platform\Function.hpp"

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;
bool Engine::suspended;

struct Data
{
	void Compute(int i) const { float f = sinf((float)i); }
};

void Work(int i)
{
	Logger::Debug(i);
}

typedef void(*Func)(Data);

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

	//Timer t;
	//t.Start();
	//for (int i = 0; i < 1000000; ++i)
	//{
	//	Function<void(Data)> func(Work);
	//	Function<void(Data)> newFunc = func;
	//	func.~Function();
	//	newFunc.~Function();
	//}
	//
	//Logger::Debug(t.CurrentTime());
	//
	//t.Reset();
	//t.Start();
	//for (int i = 0; i < 1000000; ++i)
	//{
	//	Function<void(Data)> func(Work);
	//	Function<void(Data)> newFunc = Move(func);
	//	newFunc.~Function();
	//}
	//
	//Logger::Debug(t.CurrentTime());

	//TODO: Only works with decimal count of 5 or 0
	//F32 f = 123.123f;
	//String s512("{.3}", f);

	ASSERT(Settings::Initialize());
	ASSERT(Jobs::Initialize());
	ASSERT(Platform::Initialize(applicationName));
	ASSERT(Input::Initialize());
	ASSERT(Resources::Initialize());
	//Audio
	ASSERT(Renderer::Initialize(applicationName));
	//UI
	//Physics
	//Particle
	ASSERT(GameInit());

	const U32 dataSize = 200;
	Data* data = new Data[dataSize]{};

	Timer t;
	t.Start();
	for (int i = 0; i < dataSize; ++i)
	{
		data[i].Compute(i);
	}
	Logger::Debug("Regular: {}", t.CurrentTime());
	t.Reset();

	Jobs::Execute([] { Work(10); });

	//const U32 groupSize = 1;
	//t.Start();
	//Jobs::Dispatch(dataSize, groupSize, [data](JobDispatchArgs args) { data[args.jobIndex].Compute(args.jobIndex); });
	//
	//Jobs::Wait();
	//Logger::Debug("Dispatch: {}", t.CurrentTime());
	//t.Reset();

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
		if (!Platform::Update() || Input::OnButtonDown(ESCAPE)) { break; }

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

