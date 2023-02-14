#pragma once

#include "Defines.hpp"

class NH_API Settings
{
private:
	//AUDIO
	static inline U8 CHANNEL_COUNT = 2;
	static inline F32 MASTER_VOLUME = 1.0f;
	static inline F32 MUSIC_VOLUME = 1.0f;
	static inline F32 SFX_VOLUME = 1.0f;

	//GRAPHICS
	static inline U32 WINDOW_WIDTH = 0;
	static inline U32 WINDOW_HEIGHT = 0;
	static inline U32 WINDOW_WIDTH_SMALL = 1280;
	static inline U32 WINDOW_HEIGHT_SMALL = 720;
	static inline I32 WINDOW_POSITION_X = 0;
	static inline I32 WINDOW_POSITION_Y = 0;
	static inline I32 WINDOW_POSITION_X_SMALL = 320;
	static inline I32 WINDOW_POSITION_Y_SMALL = 180;
	static inline U32 SCREEN_WIDTH = 0;
	static inline U32 SCREEN_HEIGHT = 0;
	static inline F64 TARGET_FRAMETIME = 0.0;
	static inline F64 TARGET_FRAMETIME_SUSPENDED = 0.1;
	static inline U8 MSAA_COUNT = 1;

	//PLATFORM
	static inline U32 DPI = 192; //TODO: temp, don't leave it at 200%
	static inline F64 MONITOR_HZ = 0.0;
	static inline U32 THREAD_COUNT = 1;
	static inline bool FOCUSED = true;
	static inline bool MINIMISED = true;
	static inline bool FULLSCREEN = false;
	static inline bool LOCK_CURSOR = false;
	static inline bool HIDE_CURSOR = false;
	static inline bool CONSTRAIN_CURSOR = false;

public:
	static inline const U8& ChannelCount = CHANNEL_COUNT;
	static inline const F32& MasterVolume = MASTER_VOLUME;
	static inline const F32& MusicVolume = MUSIC_VOLUME;
	static inline const F32& SfxVolume = SFX_VOLUME;

	static inline const U32& WindowWidth = WINDOW_WIDTH;
	static inline const U32& WindowHeight = WINDOW_HEIGHT;
	static inline const U32& WindowWidthSmall = WINDOW_WIDTH_SMALL;
	static inline const U32& WindowHeightSmall = WINDOW_HEIGHT_SMALL;
	static inline const I32& WindowPositionX = WINDOW_POSITION_X;
	static inline const I32& WindowPositionY = WINDOW_POSITION_Y;
	static inline const I32& WindowPositionXSmall = WINDOW_POSITION_X_SMALL;
	static inline const I32& WindowPositionYSmall = WINDOW_POSITION_Y_SMALL;
	static inline const U32& ScreenWidth = SCREEN_WIDTH;
	static inline const U32& ScreenHeight = SCREEN_HEIGHT;
	static inline const F64& TargetFrametime = TARGET_FRAMETIME;
	static inline const F64& TargetFrametimeSuspended = TARGET_FRAMETIME_SUSPENDED;
	static inline const U8& MSAACount = MSAA_COUNT;

	static inline const U32& Dpi = DPI;
	static inline const F64& MonitorHz = MONITOR_HZ;
	static inline const U32& ThreadCount = THREAD_COUNT;
	static inline const bool& Focused = FOCUSED;
	static inline const bool& Minimised = MINIMISED;
	static inline const bool& Fullscreen = FULLSCREEN;
	static inline const bool& LockCursor = LOCK_CURSOR;
	static inline const bool& HideCursor = HIDE_CURSOR;
	static inline const bool& ConstrainCursor = CONSTRAIN_CURSOR;

	friend class Platform;
	friend class Jobs;
};