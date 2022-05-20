#include "Events.hpp"

Map<String, List<Events::EventFunc>> Events::observers;

void Events::Subscribe(const String& name, EventFunc fn)
{
    observers.InsertGet(name).PushBack(fn);
}

void Events::Notify(const String& name, void* data)
{
    if (!observers.Empty())
    {
        for (const EventFunc& fn : observers[name])
        {
            if (fn(data)) { return; }
        }
    }
}