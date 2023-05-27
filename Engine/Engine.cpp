#include "Engine.hpp"

#include "Memory\Memory.hpp"
#include "Platform\Platform.hpp"
#include "Platform\Input.hpp"
#include "Platform\Jobs.hpp"
#include "Platform\Function.hpp"
#include "Core\Logger.hpp"
#include "Core\Time.hpp"
#include "Core\File.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"
#include "Containers\String.hpp"
#include "Containers\Vector.hpp"
#include "Containers\Queue.hpp"
#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"

#include "External\tracy\tracy\Tracy.hpp"

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;
bool Engine::suspended;

struct Data
{
	int i;
	float f;
};

void Work(int i)
{
	Logger::Debug("I am working: {}", i);
}

void Work2(Data d)
{
	Logger::Debug("I am also working: {}, {}", d.i, d.f);
}

void Work3(Data d)
{
	Logger::Debug("I, too, am working: {}, {}", d.i, d.f);
}

void Work4()
{
	Logger::Debug("I'm work without data :(");
}

typedef void(*Func)(Data);

void Engine::Initialize(CSTR applicationName, U32 applicationVersion, InitializeFn init, UpdateFn update, ShutdownFn shutdown)
{
	GameInit = init;
	GameUpdate = update;
	GameShutdown = shutdown;

	running = true;
	suspended = false;

	ASSERT(Time::Initialize());
	ASSERT(Memory::Initialize());
	ASSERT(Logger::Initialize());

	//Data d{ 27, 3.14f };
	//
	//auto f = Work3;
	//
	//Jobs::Execute([] { Work(354); });
	//Jobs::Execute([&] { Work2(d); });
	//Jobs::Execute([&] { f(d); });
	//Jobs::Execute(Work4);

	//TODO: Only works with decimal count of 5 or 0
	//F32 f = 123.123f;
	//String s512("{.3}", f);

	//std::bit_floor(12);	//TODO:
	//std::but_ceiling(12); //TODO:

	//Logger::Info(Vector2{ 12.123f, 123.12f });

	ASSERT(Settings::Initialize());
	ASSERT(Jobs::Initialize());
	ASSERT(Platform::Initialize(applicationName));
	ASSERT(Input::Initialize());
	//Audio
	ASSERT(Renderer::Initialize(applicationName, applicationVersion));
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
	Renderer::Shutdown();
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
		//ZoneScoped;
		Time::Update();
		Input::Update();

		if (!Platform::Update() || Input::OnButtonDown(BUTTON_CODE_ESCAPE)) { break; }

		if (!Settings::Minimised()) { Renderer::BeginFrame(); }

		if (Settings::Resized())
		{
			Renderer::Resize();
		}

		//Physics::Update();
		GameUpdate();
		//Animations::Update();
		
		if (Settings::Focused() || Settings::UnfocusedAudio())
		{
			//Audio::Update();
		}
		
		if (!Settings::Minimised())
		{
			//UI::Update();
			
			Renderer::EndFrame();
			Settings::resized = false;
		}

		F64 remainingFrameTime;

		if (Settings::Focused()) { remainingFrameTime = Settings::TargetFrametime() - Time::FrameUpTime(); }
		else { remainingFrameTime = Settings::TargetFrametimeSuspended() - Time::FrameUpTime(); }

		U64 remainingUS = (U64)(remainingFrameTime * 1090000.0);

		if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }

		//FrameMark;
	}
}

