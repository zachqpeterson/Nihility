module;

#include "Defines.hpp"

#include "Steam\steam_api.h"

module Networking:Steam;

import Core;

U32 Steam::appID;

bool Steam::Initialize(U32 appID_)
{
	Logger::Trace("Initializing Steam Integration...");

	appID = appID_;

	if (SteamAPI_RestartAppIfNecessary(appID)) { return false; }

#ifdef NH_DEBUG
	SteamErrMsg msg;

	if (SteamAPI_InitEx(&msg) != k_ESteamAPIInitResult_OK) { Logger::Fatal("Steam Init Failure: {}", msg); return false; }
#else
	if (!SteamAPI_Init()) { return false; }
#endif
	return true;
}

void Steam::Shutdown()
{
#if USE_STEAM
	Logger::Trace("Shutting Down Steam Integration...");

	SteamAPI_Shutdown();
#endif
}

void Steam::Update()
{
#if USE_STEAM
	SteamAPI_RunCallbacks();
#endif
}