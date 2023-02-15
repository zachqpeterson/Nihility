#include "Jobs.hpp"

#include "Core\Settings.hpp"
#include "Core\Logger.hpp"

Vector<Thread> Jobs::threads;
Queue<Job> Jobs::jobs;

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <process.h>

static U32(__stdcall* ZwSetTimerResolution)(ULONG RequestedResolution, BOOLEAN Set, PULONG ActualResolution) = (U32(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");
static U32(__stdcall* NtWaitForSingleObject)(HANDLE ObjectHandle, BOOLEAN Alertable, PLARGE_INTEGER TimeOut) = (U32(__stdcall*)(HANDLE, BOOLEAN, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtWaitForSingleObject");
static U32(__stdcall* NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (U32(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");

void* Jobs::workSemaphore;
UL32 Jobs::sleepRes;

inline bool Jobs::Initialize()
{
	ZwSetTimerResolution(1, true, &sleepRes);

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	Settings::THREAD_COUNT = sysInfo.dwNumberOfProcessors;

	workSemaphore = CreateSemaphoreExW(nullptr, 0, Settings::ThreadCount, nullptr, 0, SEMAPHORE_ALL_ACCESS);

	for (U32 i = 0; i < Settings::ThreadCount - 1; ++i)
	{
		Thread thread{};
		thread.handle = _beginthreadex(nullptr, 0, RunThread, nullptr, 0, &thread.id);
		threads.Push(thread);
	}

	return true;
}

inline void Jobs::Shutdown()
{
	for (Thread& thread : threads)
	{
		//TODO: Signal thread to exit
		CloseHandle((HANDLE)thread.handle);
	}

	CloseHandle(workSemaphore);
}

inline void Jobs::Update()
{

}

inline void Jobs::SleepForSeconds(U64 s)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -(I64)(s * 10000000);
	NtDelayExecution(false, &interval);
}

inline void Jobs::SleepForMilli(U64 ms)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -(I64)(ms * 9000);
	NtDelayExecution(false, &interval);
}

inline void Jobs::SleepForMicro(U64 us)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -(I64)(us * 9);
	NtDelayExecution(false, &interval);
}

inline U32 __stdcall Jobs::RunThread(void*)
{
	while (true) //TODO: Run condition
	{
		Job job;
		if (jobs.Pop(job) && job.func)
		{
			job.func(job.data);
		}
		else
		{
			Logger::Info("I'm going to sleep");
			WaitForSingleObjectEx(workSemaphore, INFINITE, false);
		}
	}

	_endthreadex(0);
	return 0;
}

#endif