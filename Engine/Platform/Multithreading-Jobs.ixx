module;

#include "Defines.hpp"
#include "Core\Logger.hpp"
#include "Core\Function.hpp"
#include "Containers\SafeQueue.hpp"

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#include <process.h>
#endif

export module Multithreading:Jobs;

import :Semaphore;
import ThreadSafety;

#ifdef PLATFORM_WINDOWS
static U32(__stdcall* ZwSetTimerResolution)(ULONG RequestedResolution, BOOLEAN Set, PULONG ActualResolution) = (U32(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");
static U32(__stdcall* NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (U32(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");
#endif

export enum NH_API JobPriority
{
	JOB_PRIORITY_LOW,
	JOB_PRIORITY_MEDIUM,
	JOB_PRIORITY_HIGH,

	JOB_PRIORITY_COUNT
};

struct NH_API JobQueue
{
	SafeQueue<Function<void()>> jobs{ 256 };
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
	static void Request(const Function<void()>& job, JobPriority priority = JOB_PRIORITY_MEDIUM);
	static bool Dispatch(U32 jobCount, U32 groupSize, const Function<void(DispatchArgs)>& job, JobPriority priority = JOB_PRIORITY_MEDIUM);

	static void Wait(JobPriority minPriority);

	static void SleepForSeconds(U64 s);
	static void SleepForMilli(U64 ms);
	static void SleepForMicro(U64 us);

private:
	static bool Initialize();
	static void Shutdown();

	static void Poll();

	static inline bool running{ false };
	static inline U64 threadCount{ 1 };
	static inline U64 jobCount{ 0 };
	//static inline Semaphore semaphore{};
	static inline JobQueue jobQueues[JOB_PRIORITY_COUNT]{};

#if defined PLATFORM_WINDOWS
	static U32 __stdcall RunThread(void*);
	static inline UL32 sleepRes;
#endif

	STATIC_CLASS(Jobs);
	friend class Engine;
};

bool Jobs::Initialize()
{
	UL32 processorCount = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
	U32 hardwareConcurrency = _Thrd_hardware_concurrency();

	Logger::Trace("Initializing Jobs...");

	ZwSetTimerResolution(1, true, &sleepRes);

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	threadCount = sysInfo.dwNumberOfProcessors;

	running = true;

	if (processorCount > 64 && hardwareConcurrency < processorCount && hardwareConcurrency < threadCount)
	{
		U16 groupCount = GetActiveProcessorGroupCount();

		GROUP_AFFINITY mainThreadAffinity;
		if (!GetThreadGroupAffinity(GetCurrentThread(), &mainThreadAffinity)) { return false; }

		U16 mainGroup = mainThreadAffinity.Group;
		UL32 activeProcessorCount = GetActiveProcessorCount(mainGroup);

		U32 group = 0;
		while (activeProcessorCount < threadCount)
		{
			++group;
			U32 currGroup = (group + mainGroup) % groupCount;
			U32 groupNumLogicalProcessors = GetActiveProcessorCount((WORD)currGroup);
			U64 GROUPMASK = U64_MAX >> (64 - groupNumLogicalProcessors);
			for (U32 groupLogicalProcess = 0; (groupLogicalProcess < groupNumLogicalProcessors) && (activeProcessorCount < threadCount); ++groupLogicalProcess, ++activeProcessorCount)
			{
				if (activeProcessorCount > 1)
				{
					U32 address;
					HANDLE handle = (HANDLE)_beginthreadex(nullptr, 0, RunThread, nullptr, 0, &address);
					//UL32 affinityMask = 1ull << address;
					//U64 affinityResult = SetThreadAffinityMask(handle, affinityMask);
					//BOOL priorityResult = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);

					GROUP_AFFINITY threadAffinity;
					GetThreadGroupAffinity(handle, &threadAffinity);

					if (threadAffinity.Group != currGroup)
					{
						threadAffinity.Group = (WORD)currGroup;
						threadAffinity.Mask = GROUPMASK;
						SetThreadGroupAffinity(handle, &threadAffinity, nullptr);
					}

					CloseHandle(handle);
				}
			}
		}
	}
	else
	{
		for (U32 i = 1; i < threadCount; ++i)
		{
			U32 address;
			HANDLE handle = (HANDLE)_beginthreadex(nullptr, 0, RunThread, nullptr, 0, &address);
			//UL32 affinityMask = 1ull << address;
			//U64 affinityResult = SetThreadAffinityMask(handle, affinityMask);
			//BOOL priorityResult = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
			CloseHandle(handle);
		}
	}

	return true;
}

void Jobs::Shutdown()
{
	Logger::Trace("Shutting Down Jobs...");

	running = false;
}

void Jobs::Request(const Function<void()>& job, JobPriority priority)
{
	SafeIncrement(&jobCount);

	while (!jobQueues[priority].jobs.Push(job)) { Poll(); }

	//semaphore.Signal();
}

bool Jobs::Dispatch(U32 jobCount, U32 groupSize, const Function<void(DispatchArgs)>& job, JobPriority priority)
{
	if (jobCount == 0 || groupSize == 0) { return false; }

	const U32 groupCount = (jobCount + groupSize - 1) / groupSize;

	SafeAdd(&jobCount, groupCount);

	for (U32 groupIndex = 0; groupIndex < groupCount; ++groupIndex)
	{
		const auto& jobGroup = [jobCount, groupSize, job, groupIndex]() {

			const U32 groupJobOffset = groupIndex * groupSize;
			U32 end = groupJobOffset + groupSize;
			const U32 groupJobEnd = end < jobCount ? end : jobCount;

			DispatchArgs args;
			args.groupIndex = groupIndex;

			for (U32 i = groupJobOffset; i < groupJobEnd; ++i)
			{
				args.jobIndex = i;
				job(args);
			}
		};

		while (!jobQueues[priority].jobs.Push(jobGroup)) { Poll(); }

		//semaphore.Signal();
	}

	return true;
}

void Jobs::Wait(JobPriority minPriority)
{
	for (U32 i = minPriority; i < JOB_PRIORITY_COUNT; ++i)
	{
		if (!jobQueues[i].jobs.Empty()) { Poll(); }
	}
}

void Jobs::Poll()
{
	//semaphore.Signal();
	SwitchToThread();
}

void Jobs::SleepForSeconds(U64 s)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -(I64)(s * 10000000);
	NtDelayExecution(false, &interval);
}

void Jobs::SleepForMilli(U64 ms)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -(I64)(ms * 10000);
	NtDelayExecution(false, &interval);
}

void Jobs::SleepForMicro(U64 us)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -(I64)(us * 10);
	NtDelayExecution(false, &interval);
}

U32 __stdcall Jobs::RunThread(void*)
{
	Function<void()> job;

	while (running)
	{
		for (U32 i = JOB_PRIORITY_COUNT - 1; i >= 0; --i)
		{
			if (jobQueues[i].jobs.Pop(job)) { job(); SafeDecrement(&jobCount); }
			//else { semaphore.Wait(); }
		}
	}

	_endthreadex(0);
	return 0;
}