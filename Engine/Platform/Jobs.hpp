#pragma once

#include "Defines.hpp"

#include "Containers\Vector.hpp"
#include "Containers\Queue.hpp"
#include "Containers\String.hpp"

//TODO: Variadic args
using JobFunc = void(*)(void*);

//TODO: This syntax would be ideal: StartJob<func>(params, params, ...);

struct Job
{
	JobFunc func;
	void* data;
};

struct WorkQueue
{
	//TODO: circular queue
	Job queue[256];
	volatile U64 entriesCompleted;
	volatile U64 nextEntry;
	volatile U64 entryCount;
	U64 maxEntries;
	void* semaphore;
};

/*
* TODO: Limit jobs active at once, maybe add a queue system for low priority jobs
* TODO: Wait for semaphore/fence
*/
class NH_API Jobs
{
public:
	static void StartJob(JobFunc func, void* data);
	static void SleepForSeconds(U64 s);
	static void SleepForMilli(U64 ms);
	static void SleepForMicro(U64 us);
	static void WaitFor(); //TODO: Custom semaphore type

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

#if defined PLATFORM_WINDOWS
	static U32 __stdcall RunThread(void*);

	static UL32 sleepRes;
#endif

	static WorkQueue jobs;
	static bool running;

	STATIC_CLASS(Jobs);
	friend class Engine;
};