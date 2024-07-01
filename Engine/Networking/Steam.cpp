#include "Steam.hpp"

#include "Steam\steam_api.h"

import Core;

#define USE_STEAM 0

U32 Steam::appID;

bool Steam::Initialize(U32 appID_)
{
#if USE_STEAM
	Logger::Trace("Initializing Steam Integration...");

	appID = appID_;

	if (SteamAPI_RestartAppIfNecessary(appID)) { return false; }

#ifdef NH_DEBUG
	SteamErrMsg msg;

	switch (SteamAPI_InitEx(&msg))
	{
	case k_ESteamAPIInitResult_OK: break;
	case k_ESteamAPIInitResult_FailedGeneric:
	case k_ESteamAPIInitResult_NoSteamClient:
	case k_ESteamAPIInitResult_VersionMismatch: { Logger::Fatal("Steam Init Failure: {}", msg); } return false;
	}
#else
	if (!SteamAPI_Init()) { return false; }
#endif
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