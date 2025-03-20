#include "Engine.hpp"

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Platform/Platform.hpp"
#include "Platform/Memory.hpp"
#include "Platform/Input.hpp"
#include "Resources/Settings.hpp"
#include "Containers/String.hpp"
#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Core/Events.hpp"
#include "Math/Math.hpp"
#include "Multithreading/Jobs.hpp"

bool Engine::Initialize()
{
	Logger::Initialize();
	Memory::Initialize();
	Settings::Initialize();
	Platform::Initialize();
	Input::Initialize();
	Time::Initialize();

	MainLoop();
	Shutdown();

	return true;
}

void Engine::Shutdown()
{
	Time::Shutdown();
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
		Time::Update();
		Input::Reset();
		Platform::Update();
		Input::Update();

		//game update

		//physics update

		//renderer update

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