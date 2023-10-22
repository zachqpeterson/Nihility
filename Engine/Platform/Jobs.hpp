#pragma once

#include "Defines.hpp"

#include "Containers\Freelist.hpp"
#include "Containers\SafeQueue.hpp"
#include "Platform\Function.hpp"

enum JobPriority
{
	JOB_PRIORITY_LOW,
	JOB_PRIORITY_MEDIUM,
	JOB_PRIORITY_HIGH,

	JOB_PRIORITY_COUNT
};

struct JobDispatchArgs
{
	U32 jobIndex;
	U32 groupIndex;
};

/*
* TODO: Limit jobs active at once, maybe add a queue system for low priority jobs
* TODO: Wait for semaphore/fence
* TODO: This syntax would be ideal: StartJob<func>(param, param, ...);
*/
class Jobs
{
public:
	static bool Execute(const Function<void()>& job);
	static bool Dispatch(U32 jobCount, U32 groupSize, const Function<void(JobDispatchArgs)>& job);

	static bool Busy();
	static void Wait();

	static void SleepForSeconds(U64 s);
	static void SleepForMilli(U64 ms);
	static void SleepForMicro(U64 us);

private:
	static bool Initialize();
	static void Shutdown();

	static void Poll();

	static SafeQueue<Function<void()>> jobs;
	static U32 runningJobs;
	static void* semaphore;

#if defined PLATFORM_WINDOWS
	static U32 __stdcall RunThread(void*);

	static UL32 sleepRes;
#endif

	static bool running;

	STATIC_CLASS(Jobs);
	friend class Engine;
};