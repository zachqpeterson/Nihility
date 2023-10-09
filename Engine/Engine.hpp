#pragma once

#include "Defines.hpp"

typedef bool(*InitializeFn)();
typedef void(*UpdateFn)();
typedef void(*ShutdownFn)();

class NH_API Engine
{
public:
	/// <summary>
	/// Initializes all parts of the engine, must be call before using ANY part of the engine
	/// </summary>
	/// <param name="applicationName:">The name of your application</param>
	/// <param name="applicationVersion:">The version of your application (Use MakeVersionNumber)</param>
	/// <param name="init:">Function pointer to your application initialization, called after engine initialization</param>
	/// <param name="update:">Function pointer to your application update, called once per frame</param>
	/// <param name="shutdown:">Function pointer to your application shutdown, called before engine shudown</param>
	static void Initialize(CSTR applicationName, U32 applicationVersion, InitializeFn init, UpdateFn update, ShutdownFn shutdown);

private:
	static void UpdateLoop();
	static void Shutdown();

	static InitializeFn GameInit;
	static UpdateFn GameUpdate;
	static ShutdownFn GameShutdown;

	static bool running;

	STATIC_CLASS(Engine);
};