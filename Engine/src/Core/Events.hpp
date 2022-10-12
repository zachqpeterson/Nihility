#pragma once

#include "Defines.hpp"
#include <Containers/Map.hpp>
#include <Containers/List.hpp>
#include "Containers/String.hpp"

class NH_API Events
{
    typedef bool(*EventFunc)(void*);

public:
    static void Subscribe(const String& name, EventFunc fn);
    static void Notify(const String& name, void* data);

private:
    static void Shutdown();

    static Map<String, List<EventFunc>> observers;

    Events() = delete;

    friend class Engine;
};