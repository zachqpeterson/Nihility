#pragma once

#include "Defines.hpp"
#include "Containers/Map.hpp"
#include "Containers/List.hpp"
#include "Containers/String.hpp"

class Events
{
    typedef bool(*EventFunc)(void*);

public:
    static NH_API void Subscribe(const String& name, EventFunc fn);
    static NH_API void Notify(const String& name, void* data);

private:
    static Map<String, List<EventFunc>> observers;

    Events() = delete;
};