#include "Time.hpp"

#include "Platform/Platform.hpp"

struct TimeState
{
    F64 programStart;
    F64 frameEndTime;
    F64 delta;
    F64 frameTimer;
    U16 frameRate;
    U16 frameCounter;
};

static TimeState* timeState;

bool Time::Initialize(void* state)
{
    timeState = (TimeState*)state;

    timeState->programStart = Platform::AbsoluteTime();
    timeState->frameEndTime = timeState->programStart;
    timeState->delta = 0.0;
    timeState->frameTimer = 0.0;
    timeState->frameRate = 0;
    timeState->frameCounter = 0;

    return true;
}

void* Time::Shutdown()
{
    return timeState;
}

void Time::Update()
{
    F64 now = Platform::AbsoluteTime();

    timeState->delta = now - timeState->frameEndTime;
    timeState->frameEndTime = now;
    timeState->frameTimer += timeState->delta;
    ++timeState->frameCounter;

    if (timeState->frameTimer >= 1.0)
    {
        timeState->frameTimer -= 1.0;
        timeState->frameRate = timeState->frameCounter;
        timeState->frameCounter = 0;
    }
}

const F64& Time::DeltaTime()
{
    return timeState->delta;
}

const F64 Time::UpTime()
{
    return Platform::AbsoluteTime() - timeState->programStart;
}

const F64 Time::TimeSinceLastFrame()
{
    return Platform::AbsoluteTime() - timeState->frameEndTime;
}

const F64& Time::FrameEndTime()
{
    return timeState->frameEndTime;
}

const U16& Time::FrameRate()
{
    return timeState->frameRate;
}

const U64 Time::GetMemoryRequirements()
{
    return sizeof(TimeState);
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
