#include "Engine.hpp"

#include "Platform/Platform.hpp"
#include "Resources/Settings.hpp"

bool Engine::Initialize()
{
	Settings::Initialize();
	Platform::Initialize();

	Engine::MainLoop();

	return true;
}

void Engine::Shutdown()
{
	Platform::Shutdown();
	Settings::Shutdown();
}

void Engine::MainLoop()
{
	while (Platform::Update())
	{
	}
}