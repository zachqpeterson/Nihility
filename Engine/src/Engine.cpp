#include "Engine.hpp"

bool Engine::Initialize()
{
    Memory::Initialize(Gigabytes(1));

    //TODO: Initialize platform

    return true;
}

void Engine::Shutdown()
{
    //TODO: Shutdown platform

    
    Memory::Shutdown();
}