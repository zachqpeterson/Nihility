#pragma once

#include "Defines.hpp"

typedef bool(*InitializeFn)();
typedef void(*UpdateFn)();
typedef void(*ShutdownFn)();

class NH_API Engine
{
public:
	static void Initialize(CSTR applicationName, U32 applicationVersion, InitializeFn init, UpdateFn update, ShutdownFn shutdown);

private:
	static void UpdateLoop();
	static void Shutdown();

	static InitializeFn GameInit;
	static UpdateFn GameUpdate;
	static ShutdownFn GameShutdown;

	static bool running;
	static bool suspended;

	STATIC_CLASS(Engine);
};