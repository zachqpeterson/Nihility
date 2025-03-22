#include "Resources.hpp"

#include "Core/Logger.hpp"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Cleaning Up Resources...");
}