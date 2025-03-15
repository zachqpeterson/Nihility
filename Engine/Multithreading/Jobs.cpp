#include "Jobs.hpp"

#include "Core/Time.hpp"

#include <xthreads.h>

void Jobs::Yield()
{
	_Thrd_yield();
}