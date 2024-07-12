//#pragma once
//
//import Containers;
//
//#include "Defines.hpp"
//
//struct RegistryValue
//{
//	const C8* name;
//	U8* value;
//};
//
////TODO: Store settings in the registry
//
//class NH_API Settings
//{
//public:
//	//AUDIO
//	static U8 ChannelCount();
//	static F32 MasterVolume();
//	static bool UnfocusedAudio();
//
//	//GRAPHICS
//	static U32 WindowWidth();
//	static U32 WindowHeight();
//	static U32 WindowWidthSmall();
//	static U32 WindowHeightSmall();
//	static I32 WindowPositionX();
//	static I32 WindowPositionY();
//	static I32 WindowPositionXSmall();
//	static I32 WindowPositionYSmall();
//	static F64 TargetFrametime();
//	static F64 TargetFrametimeSuspended();
//	static U8 MsaaCount();
//	static bool VSync();
//	static bool Bloom();
//
//	static void SetVSync(bool value);
//
//	//PLATFORM
//	static U32 Dpi();
//	static U32 ThreadCount();
//	static I32 ScreenWidth();
//	static I32 ScreenHeight();
//	static I32 VirtualScreenWidth();
//	static I32 VirtualScreenHeight();
//	static F64 MonitorHz();
//	static bool Fullscreen();
//	static bool CursorConstrained();
//	static bool Focused();
//	static bool Minimised();
//	static bool CursorLocked();
//	static bool CursorShowing();
//	static bool Resized();
//	static bool InEditor();
//
//	static bool GetRegistryValue(void* hKey, const String& path, const String& name, U8* value, bool fixedSize = false);
//
//private:
//	static bool Initialize();
//	static void Shutdown();
//
//	static struct Data
//	{
//		//AUDIO
//		U8 channelCount{ 2 };
//		F32 masterVolume{ 1.0f };
//		F32 musicVolume{ 1.0f };
//		F32 sfxVolume{ 1.0f };
//		bool unfocusedAudio{ false };
//
//		//GRAPHICS
//		U32 windowWidth{ 0 };
//		U32 windowHeight{ 0 };
//		U32 windowWidthSmall{ 1280 };
//		U32 windowHeightSmall{ 720 };
//		I32 windowPositionX{ 0 };
//		I32 windowPositionY{ 0 };
//		I32 windowPositionXSmall{ 320 };
//		I32 windowPositionYSmall{ 180 };
//		F64 targetFrametime{ 0.0 };
//		F64 targetFrametimeSuspended{ 0.1 };
//		U8 msaaCount{ 1 };
//		bool vSync{ false };
//		bool bloom{ true };
//
//		//PLATFORM
//		U32 dpi{ 0 };
//		U32 threadCount{ 1 };
//		bool fullscreen{ false };
//	} data;
//
//	static I32 screenWidth;
//	static I32 screenHeight;
//	static I32 virtualScreenWidth;
//	static I32 virtualScreenHeight;
//	static F64 monitorHz;
//
//	static bool focused;
//	static bool minimised;
//	static bool showCursor;
//	static bool constrainCursor;
//	static bool lockCursor;
//	static bool resized;
//
//	static bool inEditor;
//
//	STATIC_CLASS(Settings);
//	friend class Platform;
//	friend class Engine;
//	friend class Input;
//	friend class Audio;
//	friend class Jobs;
//	friend class Renderer;
//};