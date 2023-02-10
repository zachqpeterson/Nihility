#include "Jobs.hpp"

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <process.h>

bool Jobs::Initialize()
{
	return true;
}

void Jobs::Shutdown()
{
	
}

bool Jobs::StartJob()
{
	U64 i;
	if (i = _beginthreadex(nullptr, 0, StartThread, nullptr, 0, nullptr))
	{
		CloseHandle((HANDLE)i);
	}

	return true;
}

U32 __stdcall Jobs::StartThread(void* func)
{
	_endthreadex(0);
	return 0;
}

#endif