#include "Settings.hpp"

#include "Core/Logger.hpp"

F64 Settings::targetFrametime = 0.0;
F64 Settings::targetSuspendedFrametime = 0.0;

U32 Settings::windowWidth = 0;
U32 Settings::windowHeight = 0;
U32 Settings::windowWidthSmall = 1920;
U32 Settings::windowHeightSmall = 1080;
U32 Settings::windowPositionX = 0;
U32 Settings::windowPositionY = 0;
U32 Settings::windowPositionXSmall = 320;
U32 Settings::windowPositionYSmall = 180;
bool Settings::fullscreen = false;
bool Settings::cursorConstrained = false;
bool Settings::cursorLocked = false;
bool Settings::cursorShowing = false;

bool Settings::vSync = false;

U32 Settings::dpi = 0;

U8 Settings::channelCount = 2;
F32 Settings::masterVolume = 1.0f;
bool Settings::unfocusedAudio = false;

#ifdef NH_PLATFORM_WINDOWS

#include "Platform/WindowsInclude.hpp"

static HKEY registryKey;

bool Settings::Initialize()
{
	Logger::Trace("Initializing Settings...");

	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &registryKey, nullptr))
	{
		return false;
	}

	//General
	CreateSetting(TargetFrametime, &targetFrametime, sizeof(F64), SettingType::NUM64);
	CreateSetting(TargetFrametimeSuspended, &targetSuspendedFrametime, sizeof(F64), SettingType::NUM64);
	GetSetting(TargetFrametime, &targetFrametime, sizeof(F64));
	GetSetting(TargetFrametimeSuspended, &targetSuspendedFrametime, sizeof(F64));

	//Window
	CreateSetting(WindowWidth, &windowWidth, sizeof(U32), SettingType::NUM32);
	CreateSetting(WindowHeight, &windowHeight, sizeof(U32), SettingType::NUM32);
	CreateSetting(WindowWidthSmall, &windowWidthSmall, sizeof(U32), SettingType::NUM32);
	CreateSetting(WindowHeightSmall, &windowHeightSmall, sizeof(U32), SettingType::NUM32);
	CreateSetting(WindowPositionX, &windowPositionX, sizeof(U32), SettingType::NUM32);
	CreateSetting(WindowPositionY, &windowPositionY, sizeof(U32), SettingType::NUM32);
	CreateSetting(WindowPositionXSmall, &windowPositionXSmall, sizeof(U32), SettingType::NUM32);
	CreateSetting(WindowPositionYSmall, &windowPositionYSmall, sizeof(U32), SettingType::NUM32);
	CreateSetting(Fullscreen, &fullscreen, sizeof(bool), SettingType::NUM32);
	CreateSetting(CursorConstrained, &cursorConstrained, sizeof(bool), SettingType::NUM32);
	CreateSetting(CursorLocked, &cursorLocked, sizeof(bool), SettingType::NUM32);
	CreateSetting(CursorShowing, &cursorShowing, sizeof(bool), SettingType::NUM32);
	GetSetting(WindowWidth, &windowWidth, sizeof(U32));
	GetSetting(WindowHeight, &windowHeight, sizeof(U32));
	GetSetting(WindowWidthSmall, &windowWidthSmall, sizeof(U32));
	GetSetting(WindowHeightSmall, &windowHeightSmall, sizeof(U32));
	GetSetting(WindowPositionX, &windowPositionX, sizeof(U32));
	GetSetting(WindowPositionY, &windowPositionY, sizeof(U32));
	GetSetting(WindowPositionXSmall, &windowPositionXSmall, sizeof(U32));
	GetSetting(WindowPositionYSmall, &windowPositionYSmall, sizeof(U32));
	GetSetting(Fullscreen, &fullscreen, sizeof(bool));
	GetSetting(CursorConstrained, &cursorConstrained, sizeof(bool));
	GetSetting(CursorLocked, &cursorLocked, sizeof(bool));
	GetSetting(CursorShowing, &cursorShowing, sizeof(bool));

	//Graphics
	CreateSetting(VSync, &vSync, sizeof(bool), SettingType::NUM32);
	GetSetting(VSync, &vSync, sizeof(bool));

	//Platform
	CreateSetting(Dpi, &dpi, sizeof(U32), SettingType::NUM32);
	GetSetting(Dpi, &dpi, sizeof(U32));

	//Audio
	CreateSetting(ChannelCount, &channelCount, sizeof(U8), SettingType::NUM32);
	CreateSetting(MasterVolume, &masterVolume, sizeof(F32), SettingType::NUM32);
	CreateSetting(UnfocusedAudio, &unfocusedAudio, sizeof(bool), SettingType::NUM32);
	GetSetting(ChannelCount, &channelCount, sizeof(U8));
	GetSetting(MasterVolume, &masterVolume, sizeof(F32));
	GetSetting(UnfocusedAudio, &unfocusedAudio, sizeof(bool));

	return true;
}

void Settings::Shutdown()
{
	Logger::Trace("Cleaning Up Settings...");

	//General
	SetSetting(TargetFrametime, &targetFrametime, sizeof(F64), SettingType::NUM64);
	SetSetting(TargetFrametimeSuspended, &targetSuspendedFrametime, sizeof(F64), SettingType::NUM64);

	//Window
	SetSetting(WindowWidth, &windowWidth, sizeof(U32), SettingType::NUM32);
	SetSetting(WindowHeight, &windowHeight, sizeof(U32), SettingType::NUM32);
	SetSetting(WindowWidthSmall, &windowWidthSmall, sizeof(U32), SettingType::NUM32);
	SetSetting(WindowHeightSmall, &windowHeightSmall, sizeof(U32), SettingType::NUM32);
	SetSetting(WindowPositionX, &windowPositionX, sizeof(U32), SettingType::NUM32);
	SetSetting(WindowPositionY, &windowPositionY, sizeof(U32), SettingType::NUM32);
	SetSetting(WindowPositionXSmall, &windowPositionXSmall, sizeof(U32), SettingType::NUM32);
	SetSetting(WindowPositionYSmall, &windowPositionYSmall, sizeof(U32), SettingType::NUM32);
	SetSetting(Fullscreen, &fullscreen, sizeof(bool), SettingType::NUM32);
	SetSetting(CursorConstrained, &cursorConstrained, sizeof(bool), SettingType::NUM32);
	SetSetting(CursorLocked, &cursorLocked, sizeof(bool), SettingType::NUM32);
	SetSetting(CursorShowing, &cursorShowing, sizeof(bool), SettingType::NUM32);

	//Graphics
	SetSetting(VSync, &vSync, sizeof(bool), SettingType::NUM32);

	//Platform
	SetSetting(Dpi, &dpi, sizeof(U32), SettingType::NUM32);

	//Audio
	SetSetting(ChannelCount, &channelCount, sizeof(U8), SettingType::NUM32);
	SetSetting(MasterVolume, &masterVolume, sizeof(F32), SettingType::NUM32);
	SetSetting(UnfocusedAudio, &unfocusedAudio, sizeof(bool), SettingType::NUM32);

	RegCloseKey(registryKey);
}

bool Settings::CreateSetting(const char* path, const void* defaultValue, U32 size, SettingType type)
{
	HKEY key = {};
	UL32 created = 0;

	if (RegCreateKeyExA(registryKey, path, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, &created))
	{
		return false;
	}

	RegCloseKey(key);

	if (created == REG_CREATED_NEW_KEY)
	{
		return RegSetValueExA(registryKey, path, 0, (UL32)type, (U8*)defaultValue, size) == 0;
	}

	return true;
}

bool Settings::GetSetting(const char* path, void* data, U32 size)
{
	return RegQueryValueExA(registryKey, path, nullptr, nullptr, (U8*)data, (UL32*)&size) == 0;
}

bool Settings::SetSetting(const char* path, const void* data, U32 size, SettingType type)
{
	return RegSetValueExA(registryKey, path, 0, (UL32)type, (U8*)data, size) == 0;
}

#endif