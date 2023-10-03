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
	struct Data
	{
		//AUDIO
		U8 channelCount{ 2 };
		F32 masterVolume{ 1.0f };
		F32 musicVolume{ 1.0f };
		F32 sfxVolume{ 1.0f };
		bool unfocusedAudio{ false };

		//GRAPHICS
		U32 windowWidth{ 0 };
		U32 windowHeight{ 0 };
		U32 windowWidthSmall{ 1280 };
		U32 windowHeightSmall{ 720 };
		U32 windowPositionX{ 0 };
		U32 windowPositionY{ 0 };
		U32 windowPositionXSmall{ 320 };
		U32 windowPositionYSmall{ 180 };
		F64 targetFrametime{ 0.0 };
		F64 targetFrametimeSuspended{ 0.1 };
		U8 msaaCount{ 1 };
		bool vSync{ false };
		bool bloom{ true };

		//PLATFORM
		U32 dpi{ 0 };
		U32 threadCount{ 1 };
		U32 screenWidth{ 0 };
		U32 screenHeight{ 0 };
		F64 monitorHz{ 0.0 };
		bool fullscreen{ false };
		bool constrainCursor{ false };
	};

public:
	//AUDIO
	static const U8& ChannelCount();
	static const F32& MasterVolume();
	static const F32& MusicVolume();
	static const F32& SfxVolume();
	static const bool& UnfocusedAudio();

	//GRAPHICS
	static const U32& WindowWidth();
	static const U32& WindowHeight();
	static const U32& WindowWidthSmall();
	static const U32& WindowHeightSmall();
	static const U32& WindowPositionX();
	static const U32& WindowPositionY();
	static const U32& WindowPositionXSmall();
	static const U32& WindowPositionYSmall();
	static const F64& TargetFrametime();
	static const F64& TargetFrametimeSuspended();
	static const U8& MsaaCount();
	static const bool& VSync();
	static const bool& Bloom();

	//PLATFORM
	static const U32& Dpi();
	static const U32& ThreadCount();
	static const U32& ScreenWidth();
	static const U32& ScreenHeight();
	static const F64& MonitorHz();
	static const bool& Fullscreen();
	static const bool& ConstrainCursor();
	static const bool& Focused();
	static const bool& Minimised();
	static const bool& LockCursor();
	static const bool& HideCursor();
	static const bool& Resized();

	static bool GetRegistryValue(void* hKey, const String& path, const String& name, U8* value, bool fixedSize = false);

private:
	static bool Initialize();
	static void Shutdown();

	static Data data;

	static bool focused;
	static bool minimised;
	static bool lockCursor;
	static bool hideCursor;
	static bool resized;

	STATIC_CLASS(Settings);
	friend class Platform;
	friend class Engine;
	friend class Audio;
	friend class Jobs;
	friend class Renderer;
};