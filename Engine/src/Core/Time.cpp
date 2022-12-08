#include "Time.hpp"

#include "Platform/Platform.hpp"

#if defined PLATFORM_WINDOWS
#include <Windows.h>
#endif

F64 Time::programStart;
F64 Time::frameEndTime;
F64 Time::delta;
F64 Time::frameTimer;
U16 Time::frameRate;
U16 Time::frameCounter;
F64 Time::clockFrequency;

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

const F64 Time::UpTime()
{
	return AbsoluteTime() - programStart;
}

const F64 Time::AbsoluteTime()
{
#if defined PLATFORM_WINDOWS
	LARGE_INTEGER nowTime;
	QueryPerformanceCounter(&nowTime);

	return (F64)nowTime.QuadPart * clockFrequency;
#endif
}

const F64 Time::TimeSinceLastFrame()
{
	return AbsoluteTime() - frameEndTime;
}

const F64& Time::FrameEndTime()
{
	return frameEndTime;
}

const U16& Time::FrameRate()
{
	return frameRate;
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

const F64 Timer::CurrentTime() const
{
	return elapsedTime + running * (Time::AbsoluteTime() - start);
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