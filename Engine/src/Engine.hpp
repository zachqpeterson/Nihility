/** @file Engine.hpp */
#pragma once

#include "Defines.hpp"

typedef bool(*InitializeFn)();
typedef bool(*UpdateFn)();
typedef void(*CleanupFn)();

class Engine
{
public:
    /**
     * @brief Initializes the engine, MUST be called before using ANY part of the engine
     * 
     * @return true if the initialization was successful, false otherwise
     */
    static NH_API void Initialize(const struct String& applicationName, InitializeFn init, UpdateFn update, CleanupFn cleanup);
    static void Shutdown();

    static void MainLoop();

    static bool OnClose(void* data);

private:
    Engine() = delete;

    static InitializeFn GameInit;
    static UpdateFn GameUpdate;
    static CleanupFn GameCleanup;

    static bool running;
    static bool suspended;
};