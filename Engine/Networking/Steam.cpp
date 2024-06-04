#include "Steam.hpp"

#include "Core\Logger.hpp"

#include "Steam\steam_api.h"

bool Steam::Initialize()
{
	Logger::Trace("Initializing Steam...");

#ifdef NH_DEBUG
	ESteamAPIInitResult result;

	SteamErrMsg msg;
	result = SteamAPI_InitEx(&msg);

	switch (result)
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

}