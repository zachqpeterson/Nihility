#include "Engine.hpp"

#include "Defines.hpp"
#include "Introspection.hpp"
#include "Platform\Input.hpp"
#include "Platform\Audio.hpp"
#include "Resources\Resources.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"
#include "Math\Math.hpp"
#include "Math\Physics.hpp"

import Core;
import Memory;
import Multithreading;
import ThreadSafety;
import Networking;
import Platform;

GameInfo Engine::gameInfo;

F64 Engine::targetFrametime;
bool Engine::inEditor;
bool Engine::running;

void Engine::Initialize(const GameInfo& gameInfo_)
{
	gameInfo = gameInfo_;

	//TODO: Validate gameInfo

	running = true;

	if (!Memory::Initialize()) { Logger::Fatal("Failed To Initialize Memory!"); return; }
	if (!Jobs::Initialize()) { Logger::Fatal("Failed To Initialize Jobs!"); return; }
	if (!Logger::Initialize()) { Logger::Fatal("Failed To Initialize Logger!"); return; }
	if (!Time::Initialize()) { Logger::Fatal("Failed To Initialize Time!"); return; }
	if (!Events::Initialize()) { Logger::Fatal("Failed To Initialize Events!"); return; }
	if (!Settings::Initialize()) { Logger::Fatal("Failed To Initialize Settings!"); return; }
	if (!Physics::Initialize()) { Logger::Fatal("Failed To Initialize Physics!"); return; }
	if (!Platform::Initialize(gameInfo.gameName)) { Logger::Fatal("Failed To Initialize Platform!"); return; }
	if (!Renderer::Initialize(gameInfo.gameName, gameInfo.gameVersion)) { Logger::Fatal("Failed To Initialize Renderer!"); return; }
	if (!Resources::Initialize()) { Logger::Fatal("Failed To Initialize Resources!"); return; }
	if (!UI::Initialize()) { Logger::Fatal("Failed To Initialize UI!"); return; }
	//Particles
	if (!Audio::Initialize()) { Logger::Fatal("Failed To Initialize Audio!"); return; }
	if (!Input::Initialize()) { Logger::Fatal("Failed To Initialize Input!"); return; }

	if (gameInfo.steamAppId != 0)
	{
		if(!Steam::Initialize(gameInfo.steamAppId)) { Logger::Fatal("Failed To Initialize Steam!"); return; }
	}

	Discord::Initialize(gameInfo.discordAppId);

#ifdef NH_DEBUG
	Activity activity{};
	activity.name = "Nihility";
	activity.details = "In Editor";
	activity.state = gameInfo.gameName;
	activity.largeImage = "nihility_logo_large";

	Discord::SetActivity(activity);
#endif

	Logger::Trace("Initializing Game...");
	ASSERT(gameInfo.GameInit());

	Renderer::InitialSubmit();

	Events::Listen("Focused", Focus);
	Events::Listen("Unfocused", Unfocus);

	UpdateLoop();

	Shutdown();
}

void Engine::Shutdown()
{
	Logger::Trace("Shutting Down Game...");
	gameInfo.GameShutdown();
	Discord::Shutdown();
	Steam::Shutdown();
	Audio::Shutdown();
	//Particles
	UI::Shutdown();
	Resources::Shutdown();
	Renderer::Shutdown();
	Input::Shutdown();
	Platform::Shutdown();
	Physics::Shutdown();
	Jobs::Shutdown();
	Settings::Shutdown();
	Logger::Shutdown();
	Events::Shutdown();
	Memory::Shutdown();
	Time::Shutdown();
}

void Engine::UpdateLoop()
{
	F64 timeAccumulation = 0.0;
	while (running)
	{
		Time::Update();
		Input::Update();

		if (!Platform::Update() || Input::OnButtonDown(BUTTON_CODE_ESCAPE)) { break; } //TODO: Separate Thread

		if (Input::OnButtonDown(BUTTON_CODE_F11))
		{
			Platform::SetFullscreen(!Platform::fullscreen);
		}

#ifdef NH_DEBUG
		if (Input::OnButtonDown(BUTTON_CODE_F5))
		{
			inEditor = !inEditor;
		}
#endif

		bool runFrame = false;
		if (!Platform::minimised) { runFrame = Renderer::BeginFrame(); }

		Physics::Update((F32)Time::DeltaTime()); //TODO: constant step

		gameInfo.GameUpdate();
		//Animations::Update();

		Audio::Update();

		if (runFrame)
		{
			UI::Update();
			Renderer::EndFrame();
		}

		Steam::Update();
		Discord::Update();

		F64 remainingFrameTime = targetFrametime - Time::FrameUpTime();
		U64 remainingUS = (U64)(remainingFrameTime * 990000.0);

		if (remainingUS > 0) { Jobs::SleepForMicro(remainingUS); }
	}
}

bool Engine::InEditor()
{
	return inEditor;
}

void Engine::Focus()
{
	Settings::GetSetting(TargetFrametime, targetFrametime);
}

void Engine::Unfocus()
{
	Settings::GetSetting(TargetFrametimeSuspended, targetFrametime);
}