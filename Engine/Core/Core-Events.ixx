module;

#include "Defines.hpp"

export module Core:Events;

import :Function;
import Containers;

export class NH_API Events
{
	struct Event
	{
		Event() {}
		Event(Event&& other) noexcept : listeners(Move(other.listeners)) {}
		Event& operator=(Event&& other) noexcept { listeners = Move(other.listeners); return *this; }

		void Destroy() { listeners.Destroy(); }

		bool operator==(const Event& other) const { return this == &other; }
		bool operator!=(const Event& other) const { return this != &other; }

		Vector<Function<void()>> listeners;

		Event(const Event&) = delete;
		Event& operator=(const Event&) = delete;
	};

public:
	static void RegisterEvent(const String& name);
	static void Listen(const String& name, const Function<void()>& response);
	static void Listen(const String& name, Function<void()>&& response) noexcept;

	static void Notify(const String& name);

private:
	static bool Initialize();
	static void Shutdown();

	static Hashmap<String, Event> events;

	STATIC_CLASS(Events);
	friend class Engine;
};