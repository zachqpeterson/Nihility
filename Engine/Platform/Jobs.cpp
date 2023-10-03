#include "Jobs.hpp"

#include "ThreadSafety.hpp"
#include "Resources\Settings.hpp"
#include "Core\Logger.hpp"

bool Jobs::running;

SafeQueue<Function<void()>> Jobs::jobPool;
U64 Jobs::currentLabel = 0;
std::atomic<U64> Jobs::finishedLabel;
void* Jobs::semaphore;

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <process.h>

static U32(__stdcall* ZwSetTimerResolution)(ULONG RequestedResolution, BOOLEAN Set, PULONG ActualResolution) = (U32(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");
static U32(__stdcall* NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (U32(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");

UL32 Jobs::sleepRes;

bool Jobs::Initialize()
{
	Logger::Trace("Initializing Jobs...");

	ZwSetTimerResolution(1, true, &sleepRes);

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	Settings::data.threadCount = sysInfo.dwNumberOfProcessors;

	semaphore = CreateSemaphoreExW(nullptr, 0, Settings::ThreadCount(), nullptr, 0, SEMAPHORE_ALL_ACCESS);
	running = true;

	finishedLabel.store(0);

	for (U32 i = 0; i < Settings::ThreadCount() - 1; ++i)
	{
		U32 id;
		HANDLE handle = (HANDLE)_beginthreadex(nullptr, 0, RunThread, nullptr, 0, &id);
		if (handle)
		{
			UL32 affinityMask = 1ull << id;
			U64 affinityResult = SetThreadAffinityMask(handle, affinityMask);
			//BOOL priorityResult = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);

			CloseHandle(handle);
		}
	}

	return true;
}

void Jobs::Shutdown()
{
	Logger::Trace("Cleaning Up Jobs...");

	running = false;
	ReleaseSemaphore(semaphore, Settings::ThreadCount(), nullptr);
	CloseHandle(semaphore);
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
	interval.QuadPart = -(I64)(ms * 9000);
	NtDelayExecution(false, &interval);
}

void Jobs::SleepForMicro(U64 us)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -(I64)(us * 9);
	NtDelayExecution(false, &interval);
}

bool Jobs::Busy()
{
	return finishedLabel.load() < currentLabel;
}

void Jobs::Wait()
{
	while (Busy()) { Poll(); }
}

void Jobs::Poll()
{
	ReleaseSemaphore(semaphore, 1, nullptr);
	SwitchToThread(); //std::this_thread::yield(), _Thrd_yield()
}

void Jobs::Execute(const Function<void()>& job)
{
	currentLabel += 1;

	while (!jobPool.Push(job)) { Poll(); }

	ReleaseSemaphore(semaphore, 1, nullptr);
}

void Jobs::Dispatch(U32 jobCount, U32 groupSize, const Function<void(JobDispatchArgs)>& job)
{
	if (jobCount == 0 || groupSize == 0) { return; }

	const U32 groupCount = (jobCount + groupSize - 1) / groupSize;

	currentLabel += groupCount;

	for (U32 groupIndex = 0; groupIndex < groupCount; ++groupIndex)
	{
		const auto& jobGroup = [jobCount, groupSize, job, groupIndex]() {

			const U32 groupJobOffset = groupIndex * groupSize;
			U32 end = groupJobOffset + groupSize;
			const U32 groupJobEnd = end < jobCount ? end : jobCount;

			JobDispatchArgs args;
			args.groupIndex = groupIndex;

			for (U32 i = groupJobOffset; i < groupJobEnd; ++i)
			{
				args.jobIndex = i;
				job(args);
			}
		};

		while (!jobPool.Push(jobGroup)) { Poll(); }

		ReleaseSemaphore(semaphore, 1, nullptr);
	}
}

U32 __stdcall Jobs::RunThread(void*)
{
	Function<void()> job;

	while (running)
	{
		if (jobPool.Pop(job))
		{
			job();
			finishedLabel.fetch_add(1);
		}
		else
		{
			WaitForSingleObjectEx(semaphore, INFINITE, false);
		}
	}

	_endthreadex(0);
	return 0;
}

#endif