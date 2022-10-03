/** @file Engine.hpp */
#pragma once

#include "Defines.hpp"

using InitializeFn = bool (*)();
using UpdateFn = bool (*)();
using CleanupFn = void (*)();

class NH_API Engine
{
public:
    /**
     * @brief Initializes the engine, MUST be called before using ANY part of the engine
     */
    static void Initialize(const char* applicationName, InitializeFn init, UpdateFn update, CleanupFn cleanup);

private:
    static void Shutdown();

    static void MainLoop();
    static bool OnClose(void* data);

    static InitializeFn GameInit;
    static UpdateFn GameUpdate;
    static CleanupFn GameCleanup;

    static bool running;
    static bool suspended;

    Engine() = delete;
    friend class Platform;
};