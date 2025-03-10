#include "Engine.hpp"

#include "Platform/Platform.hpp"
#include "Platform/Memory.hpp"
#include "Resources/Settings.hpp"

#include "TypeTraits.hpp"

bool Engine::Initialize()
{
	Memory::Initialize();
	Settings::Initialize();
	Platform::Initialize();

	Engine::MainLoop();

	return true;
}

void Engine::Shutdown()
{
	Platform::Shutdown();
	Settings::Shutdown();
	Memory::Shutdown();
}

void Engine::MainLoop()
{
	while (Platform::Update())
	{

	}
}