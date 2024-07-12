module;

#include "Defines.hpp"

#include <Windows.h>

module Platform:Settings;

import Core;
import Containers;

HKEY Settings::registryKey;

bool Settings::Initialize()
{
	Logger::Trace("Initializing Settings...");

	if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &registryKey, nullptr))
	{
		return false;
	}

	//General
	CreateSetting(TargetFrametime, 0.0);
	CreateSetting(TargetFrametimeSuspended, 0.1);

	//Window
	CreateSetting(WindowWidth, 0);
	CreateSetting(WindowHeight, 0);
	CreateSetting(WindowWidthSmall, 1280);
	CreateSetting(WindowHeightSmall, 720);
	CreateSetting(WindowPositionX, 0);
	CreateSetting(WindowPositionY, 0);
	CreateSetting(WindowPositionXSmall, 320);
	CreateSetting(WindowPositionYSmall, 180);
	CreateSetting(Fullscreen, false);
	CreateSetting(CursorConstrained, false);

	//Graphics
	CreateSetting(VSync, false);

	//Platform
	CreateSetting(Dpi, 0);

	//Audio
	CreateSetting(ChannelCount, 2);
	CreateSetting(MasterVolume, 1.0f);
	CreateSetting(UnfocusedAudio, false);

	return true;
}

void Settings::Shutdown()
{
	RegCloseKey(registryKey);
}

bool Settings::CreateSetting(const StringView& path, const U8* defaultValue, UL32 size, I32 type)
{
	HKEY key{};
	UL32 created = 0;

	if (RegCreateKeyExA(registryKey, path.Data(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &key, &created))
	{
		Logger::Error("Failed To Creating Setting: {}", path);
		return false;
	}

	RegCloseKey(key);

	if (created == REG_CREATED_NEW_KEY)
	{
		if (RegSetValueExA(registryKey, path.Data(), 0, type, defaultValue, size))
		{
			Logger::Error("Failed To Set Setting: {}", path);
			
			return false;
		}
	}

	return true;
}

bool Settings::GetSetting(const StringView& path, U8* data, UL32 size)
{
	return RegQueryValueExA(registryKey, path.Data(), 0, nullptr, data, &size) == 0;
}

bool Settings::SetSetting(const StringView& path, const U8* data, UL32 size, I32 type)
{
	if (RegSetValueExA(registryKey, path.Data(), 0, type, data, size))
	{
		Logger::Error("Failed To Set Setting: {}", path);
		return false;
	}

	return true;
}

U32 Settings::GetSettingSize(const StringView& path)
{
	UL32 size;
	RegQueryValueExA(registryKey, path.Data(), 0, nullptr, nullptr, &size);
	return size;
}