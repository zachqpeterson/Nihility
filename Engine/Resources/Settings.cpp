#include "Settings.hpp"

#include "Core\File.hpp"
#include "Core\Logger.hpp"
#include "Containers\String.hpp"

Settings::Data Settings::data{};

bool Settings::focused{ true };
bool Settings::minimised{ true };
bool Settings::lockCursor{ false };
bool Settings::hideCursor{ false };
bool Settings::resized{ false };

bool Settings::Initialize()
{
	Logger::Trace("Loading Settings...");

	File config("Settings.cfg", FILE_OPEN_RESOURCE_READ);

	if (config.Opened())
	{
		config.Read(data);
		config.Close();
	}

	return true;
}

void Settings::Shutdown()
{
	Logger::Trace("Saving Settings...");

	File config("Settings.cfg", FILE_OPEN_RESOURCE_WRITE);

	if (config.Opened())
	{
		config.Write(data);
		config.Close();
	}
	else
	{
		Logger::Error("Failed to open Settings.cnf!");
	}
}

//AUDIO
const U8& Settings::ChannelCount() { return data.channelCount; }
const F32& Settings::MasterVolume() { return data.masterVolume; }
const F32& Settings::MusicVolume() { return data.musicVolume; }
const F32& Settings::SfxVolume() { return data.sfxVolume; }
const bool& Settings::UnfocusedAudio() { return data.unfocusedAudio; }

//GRAPHICS
const I32& Settings::WindowWidth() { return data.windowWidth; }
const I32& Settings::WindowHeight() { return data.windowHeight; }
const I32& Settings::WindowWidthSmall() { return data.windowWidthSmall; }
const I32& Settings::WindowHeightSmall() { return data.windowHeightSmall; }
const I32& Settings::WindowPositionX() { return data.windowPositionX; }
const I32& Settings::WindowPositionY() { return data.windowPositionY; }
const I32& Settings::WindowPositionXSmall() { return data.windowPositionXSmall; }
const I32& Settings::WindowPositionYSmall() { return data.windowPositionYSmall; }
const F64& Settings::TargetFrametime() { return data.targetFrametime; }
const F64& Settings::TargetFrametimeSuspended() { return data.targetFrametimeSuspended; }
const U8& Settings::MsaaCount() { return data.msaaCount; }
const bool& Settings::VSync() { return data.vSync; }
const bool& Settings::Bloom() { return data.bloom; }

//PLATFORM
const U32& Settings::Dpi() { return data.dpi; }
const U32& Settings::ThreadCount() { return data.threadCount; }
const I32& Settings::ScreenWidth() { return data.screenWidth; }
const I32& Settings::ScreenHeight() { return data.screenHeight; }
const F64& Settings::MonitorHz() { return data.monitorHz; }
const bool& Settings::Fullscreen() { return data.fullscreen; }
const bool& Settings::ConstrainCursor() { return data.constrainCursor; }
const bool& Settings::Focused() { return focused; }
const bool& Settings::Minimised() { return minimised; }
const bool& Settings::LockCursor() { return lockCursor; }
const bool& Settings::HideCursor() { return hideCursor; }
const bool& Settings::Resized() { return resized; }

#if defined PLATFORM_WINDOWS

#include <Windows.h>

bool Settings::GetRegistryValue(void* hKey, const String& path, const String& name, U8* value, bool fixedSize)
{
	HKEY key = nullptr;
	if (RegOpenKeyExA((HKEY)hKey, path.Data(), 0, KEY_READ, &key)) { return false; }

	UL32 valueType = 0;
	UL32 valueSize = 0;
	RegQueryValueExA(key, name.Data(), nullptr, &valueType, nullptr, &valueSize);

	if (valueType && valueSize)
	{
		if (!fixedSize) { Memory::AllocateSize(&value, valueSize); }
		RegQueryValueExA(key, name.Data(), nullptr, &valueType, value, &valueSize);

		RegCloseKey(key);
		return true;
	}

	RegCloseKey(key);
	return false;
}

#endif