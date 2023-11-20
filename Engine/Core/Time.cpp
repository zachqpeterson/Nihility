#include "Time.hpp"
#include "Logger.hpp"

#if defined PLATFORM_WINDOWS
#include <Windows.h>
#endif

F64 Time::clockFrequency;
F64 Time::programStart;
F64 Time::frameEndTime;
F64 Time::delta;
F64 Time::frameTimer;
U32 Time::frameRate;
U32 Time::frameCounter;

bool Time::Initialize()
{
#if defined PLATFORM_WINDOWS
	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	clockFrequency = 1.0 / (F64)frequency.QuadPart;

	programStart = (F64)startTime.QuadPart * clockFrequency;
#endif

	frameEndTime = programStart;
	delta = 0.0;
	frameTimer = 0.0;
	frameRate = 0;
	frameCounter = 0;

	return true;
}

void Time::Shutdown()
{

}

void Time::Update()
{
	F64 now = AbsoluteTime();

	delta = now - frameEndTime;
	frameEndTime = now;
	frameTimer += delta;
	++frameCounter;

	if (frameTimer >= 1.0)
	{
		frameTimer -= 1.0;
		frameRate = frameCounter;
		frameCounter = 0;
	}
}

const F64& Time::DeltaTime()
{
	return delta;
}

const F64& Time::FrameEndTime()
{
	return frameEndTime;
}

const U32& Time::FrameRate()
{
	return frameRate;
}

F64 Time::UpTime()
{
	return AbsoluteTime() - programStart;
}

F64 Time::FrameUpTime()
{
	return AbsoluteTime() - frameEndTime;
}

F64 Time::AbsoluteTime()
{
#if defined PLATFORM_WINDOWS
	LARGE_INTEGER nowTime;
	QueryPerformanceCounter(&nowTime);

	return (F64)nowTime.QuadPart * clockFrequency;
#endif
}

I64 Time::CoreCounter()
{
#if defined PLATFORM_WINDOWS
	LARGE_INTEGER nowTime;
	QueryPerformanceCounter(&nowTime);

	return nowTime.QuadPart;
#endif
}

Timer::Timer() : start{ Time::AbsoluteTime() }, elapsedTime{ 0.0 }, running{ false } {}

void Timer::Start()
{
	start += !running * (Time::AbsoluteTime() - start);
	running = true;
}

void Timer::Stop()
{
	elapsedTime += running * (Time::AbsoluteTime() - start);
	running = false;
}

void Timer::Reset()
{
	start = 0.0;
	elapsedTime = 0.0;
	running = false;
}

void Timer::Restart()
{
	start = Time::AbsoluteTime();
	elapsedTime = 0.0;
	running = true;
}

F64 Timer::CurrentTime() const
{
	return elapsedTime + running * (Time::AbsoluteTime() - start);
}