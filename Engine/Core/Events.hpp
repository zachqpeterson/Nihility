#pragma once

#include "Defines.hpp"

#include "Containers/Hashmap.hpp"
#include "Containers/Vector.hpp"

#include <functional>

template<typename... Args>
struct Event
{
public:
	Event() {}
	~Event() { invocationList.Destroy(); }

	Event& operator+=(std::function<bool(Args...)>&& func)
	{
		invocationList.Push(Move(func));
		return *this;
	}

	Event& operator-=(std::function<bool(Args...)>&& func)
	{
		U32 i = 0;
		for (const std::function<bool(Args...)>& f : invocationList)
		{
			if (func == f) { invocationList.Remove(i); return *this; }
			++i;
		}
	
		return *this;
	}

	void operator()(Args... args) const
	{
		for (const std::function<bool(Args...)>& func : invocationList)
		{
			if (func(args...)) { return; }
		}
	}

	operator bool() const
	{
		return invocationList.Size();
	}

	void Destroy()
	{
		invocationList.Destroy();
	}
	
	U32 InvocationSize() const
	{
		return invocationList.Size();
	}

private:

	Vector<std::function<bool(Args...)>> invocationList;
};