#pragma once

#include "Defines.hpp"

#include "External\Discord\discord.h"

class Discord
{
public:

private:
	static bool Initialize(CSTR applicationName);
	static void Shutdown();
	static void Update();

	static void Log(discord::LogLevel level, const char* message);
	static void Callback(discord::Result result);

	static discord::User currentUser;
	static discord::Core* core;
	static discord::Activity activity;

	friend class Engine;
};