#include "Steam.hpp"

#include "Core\Logger.hpp"

#include "Steam\steam_api.h"

U32 Steam::appID;

bool Steam::Initialize(U32 appID_)
{
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

	return true;
}

void Steam::Shutdown()
{
	Logger::Trace("Shutting Down Steam Integration...");

	SteamAPI_Shutdown();
}

void Steam::Update()
{
	SteamAPI_RunCallbacks();
}