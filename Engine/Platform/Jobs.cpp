#include "Jobs.hpp"

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <process.h>

static U32(__stdcall* NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (U32(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");
static U32(__stdcall* ZwSetTimerResolution)(ULONG RequestedResolution, BOOLEAN Set, PULONG ActualResolution) = (U32(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");

inline bool Jobs::Initialize()
{
	ULONG actualResolution;
	ZwSetTimerResolution(1, true, &actualResolution);

	//TODO: Get thread count
	for (U32 i = 0; i < 11; ++i)
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
		//TODO: Exit thread
		CloseHandle((HANDLE)thread.handle);
	}
}

inline void Jobs::Update()
{

}

inline bool Jobs::StartJob()
{

}

inline void Jobs::SleepFor(U64 ns)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -1 * (I32)(ns);
	NtDelayExecution(false, &interval);
}

inline U32 __stdcall Jobs::RunThread(void*)
{
	//TODO: Run loop





	//TODO: End thread from outside
	_endthreadex(0);
	return 0;
}

#endif