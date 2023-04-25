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
	float f;
	int i;
};

void Work(Data d)
{
	Logger::Debug("Works: {}, {}", d.f, d.i);
}

void Work2(Data d)
{
	Logger::Debug("This one also works: {}, {}", d.f, d.i);
}

void Job(int i)
{
	double d = sin(i);
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

	Func work = Work;
	Func work2 = Work2;

	Data d0{ 3.14f, 27 };
	Data d1{ 1.23f, 52 };
	Function<void()> func([&, d0] { work(d0); });
	func();
	Function<void()> func2([&, d1] { work2(d1); });
	func2();

	Function<void()> testFunc0(func);
	Function<void()> testFunc1(Move(func2));

	testFunc0();
	testFunc1();

	Jobs::Execute([&] { Work(d0); });

	auto x = [](JobDispatchArgs args) { Job(args.jobIndex); };
	x({});

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

	U32 dataSize = 10000000;

	Timer t;
	t.Start();
	for (int i = 0; i < dataSize; ++i)
	{
		Job(i);
	}
	Logger::Debug("Regular: {}", t.CurrentTime());
	t.Reset();

	const U32 groupSize = 1;
	t.Start();
	Jobs::Dispatch(dataSize, groupSize, [](JobDispatchArgs args) { Job(args.jobIndex); });
	Logger::Debug("Dispatch: {}", t.CurrentTime());
	t.Reset();

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

