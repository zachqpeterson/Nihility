#pragma once

#include "Defines.hpp"
#include "Containers\String.hpp"

struct RegistryValue
{
	const C8* name;
	U8* value;
};

class NH_API Settings
{
public:
	//AUDIO
	static const U8& ChannelCount() { return data.channelCount; }
	static const F32& MasterVolume() { return data.masterVolume; }
	static const F32& MusicVolume() { return data.musicVolume; }
	static const F32& SfxVolume() { return data.sfxVolume; }

	//GRAPHICS
	static const I32& WindowWidth() { return data.windowWidth; }
	static const I32& WindowHeight() { return data.windowHeight; }
	static const I32& WindowWidthSmall() { return data.windowWidthSmall; }
	static const I32& WindowHeightSmall() { return data.windowHeightSmall; }
	static const I32& WindowPositionX() { return data.windowPositionX; }
	static const I32& WindowPositionY() { return data.windowPositionY; }
	static const I32& WindowPositionXSmall() { return data.windowPositionXSmall; }
	static const I32& WindowPositionYSmall() { return data.windowPositionYSmall; }
	static const F64& TargetFrametime() { return data.targetFrametime; }
	static const F64& TargetFrametimeSuspended() { return data.targetFrametimeSuspended; }
	static const U8& MsaaCount() { return data.msaaCount; }

	//PLATFORM
	static const U32& Dpi() { return data.dpi; }
	static const U32& ThreadCount() { return data.threadCount; }
	static const I32& ScreenWidth() { return data.screenWidth; }
	static const I32& ScreenHeight() { return data.screenHeight; }
	static const F64& MonitorHz() { return data.monitorHz; }
	static const bool& Focused() { return data.focused; }
	static const bool& Minimised() { return data.minimised; }
	static const bool& Fullscreen() { return data.fullscreen; }
	static const bool& LockCursor() { return data.lockCursor; }
	static const bool& HideCursor() { return data.hideCursor; }
	static const bool& ConstrainCursor() { return data.constrainCursor; }

	static bool GetRegistryValue(void* hKey, const String& path, const String& name, U8* value, bool fixedSize = false);

private:
	static bool Initialize();
	static void Shutdown();

	static inline struct Data
	{
		//AUDIO
		U8 channelCount{ 2 };
		F32 masterVolume{ 1.0f };
		F32 musicVolume{ 1.0f };
		F32 sfxVolume{ 1.0f };

		//GRAPHICS
		I32 windowWidth{ 0 };
		I32 windowHeight{ 0 };
		I32 windowWidthSmall{ 1280 };
		I32 windowHeightSmall{ 720 };
		I32 windowPositionX{ 0 };
		I32 windowPositionY{ 0 };
		I32 windowPositionXSmall{ 320 };
		I32 windowPositionYSmall{ 180 };
		F64 targetFrametime{ 0.0 };
		F64 targetFrametimeSuspended{ 0.1 };
		U8 msaaCount{ 1 };

		//PLATFORM
		U32 dpi{ 0 };
		U32 threadCount{ 1 };
		I32 screenWidth{ 0 };
		I32 screenHeight{ 0 };
		F64 monitorHz{ 0.0 };
		bool focused{ true };
		bool minimised{ true };
		bool fullscreen{ false };
		bool lockCursor{ false };
		bool hideCursor{ false };
		bool constrainCursor{ false };
	} data{};

	STATIC_CLASS(Settings);
	friend class Platform;
	friend class Engine;
	friend class Jobs;
};