#include "Engine.hpp"

#include "Platform\Platform.hpp"

InitializeFn Engine::GameInit;
UpdateFn Engine::GameUpdate;
ShutdownFn Engine::GameShutdown;

bool Engine::running;
bool Engine::suspended;

void Engine::Initialize(const W16* applicationName, InitializeFn init, UpdateFn update, ShutdownFn shutdown)
{
	GameInit = init;
	GameUpdate = update;
	GameShutdown = shutdown;

	running = true;
	suspended = false;

	//TODO: Initialize Memory
	//TODO: Initialize Logger

	//TODO: Load Settings, First time running or if the config is missing, get monitor Hz and dpi scaling

	Platform::Initialize(applicationName);

	UpdateLoop();

	Shutdown();
}

void Engine::Shutdown()
{
	GameShutdown();

	Platform::Shutdown();
}

void Engine::UpdateLoop()
{
	while (Platform::Update())
	{
		static bool f = true;
		if (f)
		{
			f = false;
			Platform::SetFullscreen(true);
		}
	}
}