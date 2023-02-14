#include "Jobs.hpp"

#include "Core\Settings.hpp"

Vector<Thread> Jobs::threads;
Queue<Job> Jobs::jobs;

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <process.h>

static U32(__stdcall* NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (U32(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");
static U32(__stdcall* ZwSetTimerResolution)(ULONG RequestedResolution, BOOLEAN Set, PULONG ActualResolution) = (U32(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");

void* Jobs::workSemaphore;

inline bool Jobs::Initialize()
{
	ULONG actualResolution;
	ZwSetTimerResolution(1, true, &actualResolution);

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

inline bool Jobs::StartJob(JobFunc job, String str)
{
	Job j{ job, str };
	jobs.Push(j);

	ReleaseSemaphore(workSemaphore, 1, nullptr);

	return true;
}

inline void Jobs::SleepFor(U64 ns)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -1 * (I32)(ns);
	NtDelayExecution(false, &interval);
}

inline U32 __stdcall Jobs::RunThread(void*)
{
	while (true) //TODO: Run condition
	{
		if (jobs.Size()) //TODO: If work to do
		{
			Job job;
			jobs.Pop(job);
			job.func(job.param);
		}
		else
		{
			WaitForSingleObjectEx(workSemaphore, INFINITE, false);
		}
	}

	_endthreadex(0);
	return 0;
}

#endif