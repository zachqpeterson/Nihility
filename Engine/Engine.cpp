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
#include "Rendering\UI.hpp"
#include "Math\Math.hpp"

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;
bool Engine::suspended;

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
	ASSERT(Settings::Initialize());
	ASSERT(Jobs::Initialize());
	ASSERT(Platform::Initialize(applicationName));
	ASSERT(Input::Initialize());
	ASSERT(Renderer::Initialize(applicationName, applicationVersion));
	ASSERT(Resources::Initialize());
	ASSERT(UI::Initialize());

	//Physics
	//Particle
	//Audio
	ASSERT(GameInit());

	UpdateLoop();

	Shutdown();
}

void Engine::Shutdown()
{
	GameShutdown();
	//Audio
	//Particle
	//Physics
	UI::Shutdown();
	Renderer::Shutdown();
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

		if (Input::OnButtonDown(BUTTON_CODE_F11))
		{
			Platform::SetFullscreen(!Settings::Fullscreen());
		}

		if (!Settings::Minimised()) { Renderer::BeginFrame(); }

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

