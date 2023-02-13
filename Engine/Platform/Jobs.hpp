#pragma once

#include "Defines.hpp"

#include "Containers\Vector.hpp";

template<typename... Args>
using Job = void(*)(Args...);

struct Thread
{
	U64 handle;
	U32 id;
};

/*
* TODO: Limit jobs active at once, maybe add a queue system for low priority jobs
* TODO: Wait for seconds
* TODO: Wait for semaphore/fence
*/
class NH_API Jobs
{
public:
	static bool StartJob();
	static void SleepFor(U64 ns);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

#if defined PLATFORM_WINDOWS
	static U32 __stdcall RunThread(void*);
#endif

	static Vector<Thread> threads;

	STATIC_CLASS(Jobs);
	friend class Engine;
};