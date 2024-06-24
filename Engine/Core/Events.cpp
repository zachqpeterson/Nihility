module;

#include "Defines.hpp"

#include "Function.hpp"
#include "Containers\Hashmap.hpp"

module Core:Events;

Hashmap<String, Events::Event> Events::events{ 128 };

bool Events::Initialize()
{
	return true;
}

void Events::Shutdown()
{
	events.Destroy();
}

void Events::CreateEvent(const String& name)
{
	events.Insert(name, {});
}

void Events::ListenForEvent(const String& name, const Function<void()>& response)
{
	Event* event = events[name];

	if (event) { event->listeners.Push(response); }
}

void Events::ListenForEvent(const String& name, Function<void()>&& response) noexcept
{
	Event* event = events[name];

	if (event) { event->listeners.Push(Move(response)); }
}

void Events::Notify(const String& name)
{
	Event* event = events[name];

	if (event)
	{
		for (Function<void()>& response : event->listeners) { response(); }
	}
}