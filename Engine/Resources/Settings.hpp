#pragma once

#include "Defines.hpp"
#include "Containers\String.hpp"

struct RegistryValue
{
	const C8* name;
	U8* value;
};

//TODO: Store settings in the registry

class NH_API Settings
{
public:
	//AUDIO
	static inline const U8& ChannelCount() { return data.channelCount; }
	static inline const F32& MasterVolume() { return data.masterVolume; }
	static inline const F32& MusicVolume() { return data.musicVolume; }
	static inline const F32& SfxVolume() { return data.sfxVolume; }
	static inline const bool& UnfocusedAudio() { return data.unfocusedAudio; }

	//GRAPHICS
	static inline const I32& WindowWidth() { return data.windowWidth; }
	static inline const I32& WindowHeight() { return data.windowHeight; }
	static inline const I32& WindowWidthSmall() { return data.windowWidthSmall; }
	static inline const I32& WindowHeightSmall() { return data.windowHeightSmall; }
	static inline const I32& WindowPositionX() { return data.windowPositionX; }
	static inline const I32& WindowPositionY() { return data.windowPositionY; }
	static inline const I32& WindowPositionXSmall() { return data.windowPositionXSmall; }
	static inline const I32& WindowPositionYSmall() { return data.windowPositionYSmall; }
	static inline const F64& TargetFrametime() { return data.targetFrametime; }
	static inline const F64& TargetFrametimeSuspended() { return data.targetFrametimeSuspended; }
	static inline const U8& MsaaCount() { return data.msaaCount; }

	//PLATFORM
	static inline const U32& Dpi() { return data.dpi; }
	static inline const U32& ThreadCount() { return data.threadCount; }
	static inline const I32& ScreenWidth() { return data.screenWidth; }
	static inline const I32& ScreenHeight() { return data.screenHeight; }
	static inline const F64& MonitorHz() { return data.monitorHz; }
	static inline const bool& Fullscreen() { return data.fullscreen; }
	static inline const bool& ConstrainCursor() { return data.constrainCursor; }
	static inline const bool& Focused() { return focused; }
	static inline const bool& Minimised() { return minimised; }
	static inline const bool& LockCursor() { return lockCursor; }
	static inline const bool& HideCursor() { return hideCursor; }
	static inline const bool& Resized() { return resized; }

	static bool GetRegistryValue(void* hKey, const String& path, const String& name, U8* value, bool fixedSize = false);

private:
	static bool Initialize();
	static void Shutdown();

	NH_HEADER_STATIC struct Data
	{
		//AUDIO
		U8 channelCount{ 2 };
		F32 masterVolume{ 1.0f };
		F32 musicVolume{ 1.0f };
		F32 sfxVolume{ 1.0f };
		bool unfocusedAudio{ false };

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
		bool fullscreen{ false };
		bool constrainCursor{ false };
	} data{};

	NH_HEADER_STATIC bool focused{ true };
	NH_HEADER_STATIC bool minimised{ true };
	NH_HEADER_STATIC bool lockCursor{ false };
	NH_HEADER_STATIC bool hideCursor{ false };
	NH_HEADER_STATIC bool resized{ false };

	STATIC_CLASS(Settings);
	friend class Platform;
	friend class Engine;
	friend class Jobs;
};