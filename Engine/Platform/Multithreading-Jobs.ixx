module;

#include "Defines.hpp"

export module Multithreading:Jobs;

import :Semaphore;
import Containers;
import Core;

export enum NH_API JobPriority
{
	JOB_PRIORITY_LOW,
	JOB_PRIORITY_MEDIUM,
	JOB_PRIORITY_HIGH,

	JOB_PRIORITY_COUNT
};

struct NH_API JobQueue
{
	SafeQueue<Function<void()>, 256> jobs;
};

export struct NH_API DispatchArgs
{
	U32 jobIndex;
	U32 groupIndex;
};

/*
* TODO: This syntax would be ideal: StartJob<func>(param, param, ...);
*/
export class NH_API Jobs
{
public:
	static void Excecute(const Function<void()>& job, JobPriority priority = JOB_PRIORITY_MEDIUM);
	static bool Dispatch(U32 jobCount, U32 groupSize, const Function<void(DispatchArgs)>& job, JobPriority priority = JOB_PRIORITY_MEDIUM);

	static void Wait(JobPriority minPriority);

	static void SleepForSeconds(U64 s);
	static void SleepForMilli(U64 ms);
	static void SleepForMicro(U64 us);

private:
	static bool Initialize();
	static void Shutdown();

	static void Poll();

	static bool running;
	static U64 threadCount;
	static U64 activeJobCount;
	static Semaphore semaphore;
	static JobQueue jobQueues[JOB_PRIORITY_COUNT];

#if defined NH_PLATFORM_WINDOWS
	static U32 __stdcall RunThread(void*);
	static UL32 sleepRes;
#endif

	STATIC_CLASS(Jobs);
	friend class Engine;
};