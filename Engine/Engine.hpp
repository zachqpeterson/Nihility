#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"

using InitializeFn = bool(*)();
using ShutdownFn = void(*)();
using UpdateFn = void(*)();

struct NH_API GameInfo
{
	StringView name;
	U32 version;

	InitializeFn initialize;
	ShutdownFn shutdown;
	UpdateFn update;
};

class NH_API Engine
{
public:
	static bool Initialize(const GameInfo& game);

private:
	static void Shutdown();
	static void MainLoop();

	static GameInfo game;

	STATIC_CLASS(Engine);
};