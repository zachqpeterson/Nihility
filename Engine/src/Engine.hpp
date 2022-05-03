/** @file Engine.hpp */
#pragma once

#include "Defines.hpp"

class Engine
{
public:
    /**
     * @brief Initializes the engine, MUST be called before using ANY part of the engine
     * 
     * @return true if the initialization was successful, false otherwise
     */
    NH_API static bool Initialize();
    NH_API static void Shutdown();

private:
    Engine() = delete;
};