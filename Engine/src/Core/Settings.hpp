#pragma once

#include "Defines.hpp"

class Settings
{
private:
    //AUDIO
    inline static U8 CHANNEL_COUNT = 2;
    inline static F32 MASTER_VOLUME = 1.0f;
    inline static F32 MUSIC_VOLUME = 1.0f;
    inline static F32 SFX_VOLUME = 1.0f;

    //GRAPHICS
    inline static bool BORDERLESS = false;
    inline static U16 WINDOW_WIDTH = 1280;
    inline static U16 WINDOW_HEIGHT = 720;
    inline static U16 WINDOW_POSITION_X = 320;
    inline static U16 WINDOW_POSITION_Y = 180;
    inline static F64 TARGET_FRAMETIME = 0.00833333333;

    //TODO: multisampling, 

public:
    inline static const U8& ChannelCount = CHANNEL_COUNT;
    inline static const F32& MasterVolume = MASTER_VOLUME;
    inline static const F32& MusicVolume = MUSIC_VOLUME;
    inline static const F32& SfxVolume = SFX_VOLUME;

    inline static const bool& Borderless = BORDERLESS;
    inline static const U16& WindowWidth = WINDOW_WIDTH;
    inline static const U16& WindowHeight = WINDOW_HEIGHT;
    inline static const U16& WindowPositionX = WINDOW_POSITION_X;
    inline static const U16& WindowPositionY = WINDOW_POSITION_Y;
    inline static const F64& TargetFrametime = TARGET_FRAMETIME;

    friend class Resources;
    friend class Platform;
};