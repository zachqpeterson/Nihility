#pragma once

#include "Defines.hpp"

class Time
{
public:
    static void Initialize();
    static void Shutdown();

    static void Update();

    static NH_API const F64& DeltaTime();
    static NH_API const F64 UpTime();
    static NH_API const F64 TimeSinceLastFrame();
    static NH_API const F64& FrameEndTime();
    static NH_API const U16& FrameRate();

private:
    Time() = delete;

    static F64 programStart;
    static F64 frameEndTime;
    static F64 delta;
    static F64 frameTimer;
    static U16 frameRate;
    static U16 frameCounter;
};

struct Timer
{
    Timer();

    void Start();
    void Stop();
    const F64 CurrentTime() const;
    void Reset();

private:
    F64 start;
    F64 elapsedTime;
    bool running;
};