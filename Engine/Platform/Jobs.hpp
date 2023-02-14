#pragma once

#include "Defines.hpp"

#include "Containers\Vector.hpp"
#include "Containers\Queue.hpp"
#include "Containers\String.hpp"

//TODO: Variadic args
using JobFunc = void(*)(void*);

struct Job
{
	JobFunc func;
	void* data;
};

struct Thread
{
	U64 handle;
	U32 id;
};

/*
* TODO: Limit jobs active at once, maybe add a queue system for low priority jobs
* TODO: Wait for semaphore/fence
*/
class NH_API Jobs
{
public:
	static bool StartJob(JobFunc job, void* data);
	static bool StartJob(Job job);
	static void SleepForSeconds(U64 s);
	static void SleepForMilli(U64 ms);
	static void SleepForMicro(U64 us);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

#if defined PLATFORM_WINDOWS
	static U32 __stdcall RunThread(void*);

	static void* workSemaphore;
	static UL32 sleepRes;
#endif

	static Vector<Thread> threads;
	static Queue<Job> jobs;

	STATIC_CLASS(Jobs);
	friend class Engine;
};