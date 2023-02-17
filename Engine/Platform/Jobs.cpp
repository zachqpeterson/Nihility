#include "Jobs.hpp"

#include "ThreadSafety.hpp"
#include "Resources\Settings.hpp"
#include "Core\Logger.hpp"

WorkQueue Jobs::jobs;
bool Jobs::running;

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
	Settings::THREAD_COUNT = sysInfo.dwNumberOfProcessors;

	jobs.semaphore = CreateSemaphoreExW(nullptr, 0, Settings::ThreadCount, nullptr, 0, SEMAPHORE_ALL_ACCESS);
	jobs.maxEntries = 256;
	running = true;

	for (U32 i = 0; i < Settings::ThreadCount - 1; ++i)
	{
		U32 id;
		CloseHandle((HANDLE)_beginthreadex(nullptr, 0, RunThread, nullptr, 0, &id));
		Logger::Trace("Starting thread {}", id);
	}

	return true;
}

void Jobs::Shutdown()
{
	running = false;
	CloseHandle(jobs.semaphore);
}

void Jobs::Update()
{

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

void Jobs::StartJob(JobFunc func, void* data)
{
	jobs.queue[SafeIncrement(&jobs.entryCount) - 1] = {func, data};
	ReleaseSemaphore(jobs.semaphore, 1, nullptr);
}

U32 __stdcall Jobs::RunThread(void*)
{
	while (running) //TODO: Run condition
	{
		//TODO: Don't use InterlockedCompareExchange

		U64 entry = jobs.nextEntry;
		if (entry < jobs.entryCount)
		{
			U64 index = InterlockedCompareExchange(&jobs.nextEntry, entry + 1, entry);

			if (index == entry)
			{
				Job job = jobs.queue[index];
				job.func(job.data);
				SafeIncrement(&jobs.entriesCompleted);
			}
		}
		else
		{
			Logger::Debug("I'm going to sleep");
			WaitForSingleObjectEx(jobs.semaphore, INFINITE, false);
		}
	}

	Logger::Debug("Exiting thread");
	_endthreadex(0);
	return 0;
}

#endif