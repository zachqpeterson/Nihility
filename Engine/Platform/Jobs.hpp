#pragma once

#include "Defines.hpp"

#include "Containers\Vector.hpp"
#include "Containers\Queue.hpp"
#include "Containers\String.hpp"
#include "Platform\Function.hpp"
#include "SafeQueue.hpp"

enum JobPriority
{
	JOB_PRIORITY_LOW,
	JOB_PRIORITY_MEDIUM,
	JOB_PRIORITY_HIGH,
};

struct JobDispatchArgs
{
	U32 jobIndex;
	U32 groupIndex;
};

#include <atomic>    // to use std::atomic<uint64_t>

/*
* TODO: Limit jobs active at once, maybe add a queue system for low priority jobs
* TODO: Wait for semaphore/fence
* TODO: This syntax would be ideal: StartJob<func>(params, params, ...);
*/
class NH_API Jobs
{
public:
	static void Execute(const Function<void()>& job);
	static void Dispatch(U32 jobCount, U32 groupSize, const Function<void(JobDispatchArgs)>& job);

	static bool Busy();
	static void Wait();

	static void SleepForSeconds(U64 s);
	static void SleepForMilli(U64 ms);
	static void SleepForMicro(U64 us);

private:
	static bool Initialize();
	static void Shutdown();

	static void Poll();

	static SafeQueue<Function<void()>> jobPool;
	static U64 currentLabel;
	static std::atomic<U64> finishedLabel;
	static void* semaphore;

#if defined PLATFORM_WINDOWS
	static U32 __stdcall RunThread(void*);

	static UL32 sleepRes;
#endif

	static bool running;

	STATIC_CLASS(Jobs);
	friend class Engine;
};