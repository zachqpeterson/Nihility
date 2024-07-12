#pragma once

#include "Defines.hpp"

import Containers;

typedef bool(*InitializeFn)();
typedef void(*UpdateFn)();
typedef void(*ShutdownFn)();

struct NH_API GameInfo
{
	/// <summary>
	/// A function pointer of the game's initialization function
	/// </summary>
	InitializeFn GameInit = nullptr;

	/// <summary>
	/// A function pointer of the game's update function
	/// </summary>
	UpdateFn GameUpdate = nullptr;

	/// <summary>
	/// A function pointer of the game's shutdown function
	/// </summary>
	ShutdownFn GameShutdown = nullptr;

	/// <summary>
	/// The name of the game
	/// </summary>
	StringView gameName = "My Game";

	/// <summary>
	/// The version of the game, use MakeVersionNumber
	/// </summary>
	U32 gameVersion = 0;

	/// <summary>
	/// The steam application ID for the game, used when publishing on steam
	/// </summary>
	U32 steamAppId = 0;

	/// <summary>
	/// The Discord application ID for the game, used when integrating with Discord
	/// </summary>
	U64 discordAppId = 0;
};

class NH_API Engine
{
public:
	/// <summary>
	/// Initializes all parts of the engine, must be call before using ANY part of the engine
	/// </summary>
	/// <param name="gameInfo:">The information about your game</param>
	static void Initialize(const GameInfo& gameInfo);

	static bool InEditor();

private:
	static void UpdateLoop();
	static void Shutdown();

	static void Focus();
	static void Unfocus();

	static GameInfo gameInfo;

	static F64 targetFrametime;
	static bool inEditor;
	static bool running;

	STATIC_CLASS(Engine);
};