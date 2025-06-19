#include "Engine.hpp"

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Platform/Platform.hpp"
#include "Platform/Memory.hpp"
#include "Platform/Input.hpp"
#include "Resources/Settings.hpp"
#include "Resources/Resources.hpp"
#include "Containers/String.hpp"
#include "Core/Time.hpp"
#include "Core/File.hpp"
#include "Core/Logger.hpp"
#include "Core/Events.hpp"
#include "Math/Math.hpp"
#include "Math/Random.hpp"
#include "Math/Physics.hpp"
#include "Multithreading/Jobs.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/UI.hpp"
#include "Audio/Audio.hpp"

#include "tracy/Tracy.hpp"

#include <crtdbg.h>

GameInfo Engine::game;

bool Engine::Initialize(const GameInfo& _info)
{
#ifdef NH_DEBUG
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#endif

	game = _info;

	if (!Logger::Initialize()) { return false; }
	if (!Memory::Initialize()) { return false; }
	if (!Settings::Initialize()) { return false; }
	if (!Platform::Initialize(game.name)) { return false; }
	if (!Input::Initialize()) { return false; }
	if (!Audio::Initialize()) { return false; }
	if (!Renderer::Initialize(game.name, game.version)) { return false; }
	if (!Resources::Initialize()) { return false; }
	if (!UI::Initialize()) { return false; }
	if (!Physics::Initialize()) { return false; }
	game.componentsInit();
	if (!Scene::Initialize()) { return false; }
	if (!game.initialize()) { return false; }
	if (!Time::Initialize()) { return false; }

	Renderer::SubmitTransfer();

	MainLoop();
	Shutdown();

	return true;
}

void Engine::Shutdown()
{
	Time::Shutdown();
	game.shutdown();
	Scene::Shutdown();
	Physics::Shutdown();
	UI::Shutdown();
	Resources::Shutdown();
	Renderer::Shutdown();
	Audio::Shutdown();
	Input::Shutdown();
	Platform::Shutdown();
	Settings::Shutdown();
	Memory::Shutdown();
	Logger::Shutdown();
}

void Engine::MainLoop()
{
	F64 timeAccumulation = 0.0;
	while (Platform::running)
	{
		FrameMark;
		Time::Update();
		Input::Update();
		Platform::Update();

		if (Input::OnButtonDown(ButtonCode::Escape)) { Platform::running = false; }

		game.update();

		Physics::Update();
		Renderer::Update();

		F64 remainingFrameTime = Settings::targetFrametime - Time::FrameUpTime();
		I64 remainingUS = (I64)(remainingFrameTime * 1000000.0);

		while (remainingUS > 0)
		{
			Jobs::Yield();

			remainingFrameTime = Settings::targetFrametime - Time::FrameUpTime();
			remainingUS = (I64)(remainingFrameTime * 1000000.0);
		}
	}
}