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
	template<JobFunc func, typename... Args>
	static void StartJob(const Args&... args);
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

	static void* workSemaphore;
	static UL32 sleepRes;
#endif

	static Vector<Thread> threads;
	static Queue<Job> jobs;

	STATIC_CLASS(Jobs);
	friend class Engine;
};

#include <Windows.h> //TODO: temp

template<JobFunc func, typename... Args>
inline void Jobs::StartJob(const Args&... args)
{
	jobs.Push({ func, args... });

	ReleaseSemaphore(workSemaphore, 1, nullptr);
}