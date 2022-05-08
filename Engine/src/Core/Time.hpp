#pragma once

#include "Defines.hpp"

class Time
{
public:
    static bool Initialize(void* state);
    static void* Shutdown();

    static void Update();

    static NH_API const F64& DeltaTime();
    static NH_API const F64 UpTime();
    static NH_API const F64 TimeSinceLastFrame();
    static NH_API const F64& FrameEndTime();
    static NH_API const U16& FrameRate();

    static const U64 GetMemoryRequirements();
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