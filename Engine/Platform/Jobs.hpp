#pragma once

#include "Defines.hpp"

//TODO: Take in any amount of args
struct Job
{
	typedef void(*JobFunc)(void*);

	Job(JobFunc func, void* data) : func{ func }, data{ data } {}

	void Execute() { return func(data); }

	JobFunc func;
	void* data;
};

/*
* TODO: Have to call CloseHandle() after thread ends
* TODO: Limit jobs active at once, maybe add a queue system for low priority jobs
* TODO: Wait for seconds
* TODO: Wait for semaphore/fence
*/
class NH_API Jobs
{
public:
	static bool StartJob(Job job);
	static void SleepFor(U64 ns);

private:
	static bool Initialize();
	static void Shutdown();
	static void Update();

#if defined PLATFORM_WINDOWS
	static U32 __stdcall StartThread(void* func);
#endif

	STATIC_CLASS(Jobs);
	friend class Engine;
};

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <process.h>

static U32(__stdcall* NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (U32(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");
static U32(__stdcall* ZwSetTimerResolution)(ULONG RequestedResolution, BOOLEAN Set, PULONG ActualResolution) = (U32(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");

inline bool Jobs::Initialize()
{
	ULONG actualResolution;
	ZwSetTimerResolution(1, true, &actualResolution);

	return true;
}

inline void Jobs::Shutdown()
{
	//TODO: Close all threads
}

inline void Jobs::Update()
{
	//TODO: Check if job has thread has ended, call CloseHandle();
}

inline bool Jobs::StartJob(Job job)
{
	return _beginthreadex(nullptr, 0, StartThread, &job, 0, nullptr);
}

inline void Jobs::SleepFor(U64 ns)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -1 * (I32)(ns);
	NtDelayExecution(false, &interval);
}

inline U32 __stdcall Jobs::StartThread(void* func)
{
	((Job*)func)->Execute();

	//TODO: Signal job is done
	//TODO: Queue for done jobs to close
	_endthreadex(0);
	return 0;
}

#endif