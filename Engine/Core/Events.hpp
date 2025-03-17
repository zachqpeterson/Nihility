#pragma once

#include "Defines.hpp"

#include "Containers/Hashmap.hpp"
#include "Containers/Vector.hpp"
#include "Containers/Function.hpp"

template<typename... Args>
struct NH_API Event
{
public:
	Event() {}
	~Event() { invocationList.Destroy(); }

	Event& operator+=(Function<bool(Args...)>&& func)
	{
		invocationList.Push(Move(func));
		return *this;
	}

	//Event& operator-=(Function<bool(Args...)>&& func)
	//{
	//	U32 i = 0;
	//	for (const Function<bool(Args...)>& f : invocationList)
	//	{
	//		if (func == f) { invocationList.RemoveSwap(i); return *this; }
	//		++i;
	//	}
	//
	//	return *this;
	//}

	void operator()(Args... args) const
	{
		for (const Function<bool(Args...)>& func : invocationList)
		{
			if (func(args...)) { return; }
		}
	}

private:

	Vector<Function<bool(Args...)>> invocationList;
};