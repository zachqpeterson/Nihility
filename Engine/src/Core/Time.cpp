#include "Time.hpp"

#include "Platform/Platform.hpp"

F64 Time::programStart;
F64 Time::frameEndTime;
F64 Time::delta;
F64 Time::frameTimer;
U16 Time::frameRate;
U16 Time::frameCounter;

void Time::Initialize()
{
    programStart = Platform::AbsoluteTime();
    frameEndTime = programStart;
    delta = 0.0;
    frameTimer = 0.0;
    frameRate = 0;
    frameCounter = 0;
}

void Time::Shutdown()
{
    programStart = 0.0;
    frameEndTime = 0.0;
    delta = 0.0;
    frameTimer = 0.0;
    frameRate = 0;
    frameCounter = 0;
}

void Time::Update()
{
    F64 now = Platform::AbsoluteTime();

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
    return Platform::AbsoluteTime() - programStart;
}

const F64 Time::TimeSinceLastFrame()
{
    return Platform::AbsoluteTime() - frameEndTime;
}

const F64& Time::FrameEndTime()
{
    return frameEndTime;
}

const U16& Time::FrameRate()
{
    return frameRate;
}

Timer::Timer() : start{ Platform::AbsoluteTime() }, elapsedTime{ 0.0 }, running{ false } {}

void Timer::Start()
{
    start += !running * (Platform::AbsoluteTime() - start);
}

void Timer::Stop()
{
    elapsedTime += running * (Platform::AbsoluteTime() - start);
    running = false;
}

const F64 Timer::CurrentTime() const
{
    return elapsedTime + running * (Platform::AbsoluteTime() - start);
}

void Timer::Reset()
{
    start = 0.0;
    elapsedTime = 0.0;
    running = false;
}
