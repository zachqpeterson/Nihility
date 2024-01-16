#include "Settings.hpp"

#include "Core\File.hpp"
#include "Core\Logger.hpp"
#include "Core\Events.hpp"
#include "Containers\String.hpp"

Settings::Data Settings::data{};

I32 Settings::screenWidth{ 0 };
I32 Settings::screenHeight{ 0 };
I32 Settings::virtualScreenWidth{ 0 };
I32 Settings::virtualScreenHeight{ 0 };
F64 Settings::monitorHz{ 0.0 };

bool Settings::focused{ true };
bool Settings::minimised{ true };
bool Settings::lockCursor{ false };
bool Settings::showCursor{ true };
bool Settings::resized{ false };

#ifdef NH_DEBUG
bool Settings::inEditor{ true };
#else
bool Settings::inEditor{ false };
#endif

bool Settings::Initialize()
{
	Logger::Trace("Loading Settings...");

	File config("Settings.cfg", FILE_OPEN_RESOURCE_READ);

	if (config.Opened())
	{
		U32 read = config.Read(data);

		if (read != sizeof(Settings::Data) || config.Size() != sizeof(Settings::Data))
		{
			Logger::Error("Corrupted Settings.cnf, Restoring To Default Settings!");
		}

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
U8 Settings::ChannelCount() { return data.channelCount; }
F32 Settings::MasterVolume() { return data.masterVolume; }
F32 Settings::MusicVolume() { return data.musicVolume; }
F32 Settings::SfxVolume() { return data.sfxVolume; }
bool Settings::UnfocusedAudio() { return data.unfocusedAudio; }

//GRAPHICS
U32 Settings::WindowWidth() { return data.windowWidth; }
U32 Settings::WindowHeight() { return data.windowHeight; }
U32 Settings::WindowWidthSmall() { return data.windowWidthSmall; }
U32 Settings::WindowHeightSmall() { return data.windowHeightSmall; }
I32 Settings::WindowPositionX() { return data.windowPositionX; }
I32 Settings::WindowPositionY() { return data.windowPositionY; }
I32 Settings::WindowPositionXSmall() { return data.windowPositionXSmall; }
I32 Settings::WindowPositionYSmall() { return data.windowPositionYSmall; }
F64 Settings::TargetFrametime() { return data.targetFrametime; }
F64 Settings::TargetFrametimeSuspended() { return data.targetFrametimeSuspended; }
U8 Settings::MsaaCount() { return data.msaaCount; }
bool Settings::VSync() { return data.vSync; }
bool Settings::Bloom() { return data.bloom; }

void Settings::SetVSync(bool value) { data.vSync = value; }

//PLATFORM
U32 Settings::Dpi() { return data.dpi; }
U32 Settings::ThreadCount() { return data.threadCount; }
I32 Settings::ScreenWidth() { return screenWidth; }
I32 Settings::ScreenHeight() { return screenHeight; }
I32 Settings::VirtualScreenWidth() { return virtualScreenWidth; }
I32 Settings::VirtualScreenHeight() { return virtualScreenHeight; }
F64 Settings::MonitorHz() { return monitorHz; }
bool Settings::Fullscreen() { return data.fullscreen; }
bool Settings::CursorConstrained() { return data.constrainCursor; }
bool Settings::Focused() { return focused; }
bool Settings::Minimised() { return minimised; }
bool Settings::CursorLocked() { return lockCursor; }
bool Settings::CursorShowing() { return showCursor; }
bool Settings::Resized() { return resized; }

bool Settings::InEditor() { return inEditor; }

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