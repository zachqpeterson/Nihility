#include "Events.hpp"

Hashmap<String, Events::Event> Events::events(128);

bool Events::Initialize()
{
	return true;
}

void Events::Shutdown()
{
	events.Destroy();
}

void Events::RegisterEvent(const String& name)
{
	*events.Request(name) = {};
}

void Events::Listen(const String& name, const Function<void()>& response)
{
	Event* event = events[name];

	if (event) { event->listeners.Push(response); }
}

void Events::Listen(const String& name, Function<void()>&& response) noexcept
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