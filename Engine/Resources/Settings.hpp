#pragma once

#include "Defines.hpp"

/// <summary>
/// Data types that CreateSetting supports, for more info on each one visit <see href="https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types">Microsoft</see>
/// </summary>
enum class SettingType
{
	STRING = 1ul,		// Unicode null terminated string
	STRING_ENV = 2ul,	// Unicode null terminated string (with environment variable references)
	BINARY = 3ul,		// Free form binary
	NUM32 = 4ul,		// 32-bit number
	NUM64 = 11ul,		// 64-bit number
};

/* TODO:
* Setting Caching w/ Hashmap
*/
class NH_API Settings
{
public:
	static bool CreateSetting(const char* path, const void* defaultValue, U32 size, SettingType type);
	static bool GetSetting(const char* path, void* data, U32 size);
	static bool SetSetting(const char* path, const void* data, U32 size, SettingType type);

private:
	static bool Initialize();
	static void Shutdown();

	//Engine

	static F64 targetFrametime;
	static F64 targetSuspendedFrametime;

	//Window

	static U32 windowWidth;
	static U32 windowHeight;
	static U32 windowWidthSmall;
	static U32 windowHeightSmall;
	static U32 windowPositionX;
	static U32 windowPositionY;
	static U32 windowPositionXSmall;
	static U32 windowPositionYSmall;
	static bool fullscreen;
	static bool cursorConstrained;
	static bool cursorLocked;
	static bool cursorShowing;
	static U32 dpi;

	//Graphics

	static bool vSync;

	//Audio

	static U8 channelCount;
	static F32 masterVolume;
	static bool unfocusedAudio;

	static constexpr inline const char* TargetFrametime = "Nihility\\TargetFrametime";
	static constexpr inline const char* TargetFrametimeSuspended = "Nihility\\TargetFrametimeSuspended";
	static constexpr inline const char* WindowWidth = "Nihility\\Window\\Width";
	static constexpr inline const char* WindowHeight = "Nihility\\Window\\Height";
	static constexpr inline const char* WindowWidthSmall = "Nihility\\Window\\WidthSmall";
	static constexpr inline const char* WindowHeightSmall = "Nihility\\Window\\HeightSmall";
	static constexpr inline const char* WindowPositionX = "Nihility\\Window\\PositionX";
	static constexpr inline const char* WindowPositionY = "Nihility\\Window\\PositionY";
	static constexpr inline const char* WindowPositionXSmall = "Nihility\\Window\\PositionXSmall";
	static constexpr inline const char* WindowPositionYSmall = "Nihility\\Window\\PositionYSmall";
	static constexpr inline const char* Fullscreen = "Nihility\\Window\\Fullscreen";
	static constexpr inline const char* CursorConstrained = "Nihility\\Window\\CursorConstrained";
	static constexpr inline const char* CursorLocked = "Nihility\\Window\\CursorLocked";
	static constexpr inline const char* CursorShowing = "Nihility\\Window\\CursorShowing";
	static constexpr inline const char* Dpi = "Nihility\\Platform\\Dpi";
	static constexpr inline const char* VSync = "Nihility\\Graphics\\VSync";
	static constexpr inline const char* ChannelCount = "Nihility\\Audio\\ChannelCount";
	static constexpr inline const char* MasterVolume = "Nihility\\Audio\\MasterVolume";
	static constexpr inline const char* UnfocusedAudio = "Nihility\\Audio\\UnfocusedAudio";

	friend class Engine;
	friend class Platform;
	friend class Input;

	STATIC_CLASS(Settings);
};