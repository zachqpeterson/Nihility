module;

#include "Defines.hpp"

#include <Windows.h>

export module Platform:Settings;

import Containers;

//Registry Keys

//General

export constexpr StringView TargetFrametime = "Nihility\\TargetFrametime";
export constexpr StringView TargetFrametimeSuspended = "Nihility\\TargetFrametimeSuspended";

//Window

export constexpr StringView WindowWidth = "Nihility\\Window\\Width";
export constexpr StringView WindowHeight = "Nihility\\Window\\Height";
export constexpr StringView WindowWidthSmall = "Nihility\\Window\\WidthSmall";
export constexpr StringView WindowHeightSmall = "Nihility\\Window\\HeightSmall";
export constexpr StringView WindowPositionX = "Nihility\\Window\\PositionX";
export constexpr StringView WindowPositionY = "Nihility\\Window\\PositionY";
export constexpr StringView WindowPositionXSmall = "Nihility\\Window\\PositionXSmall";
export constexpr StringView WindowPositionYSmall = "Nihility\\Window\\PositionYSmall";
export constexpr StringView Fullscreen = "Nihility\\Window\\Fullscreen";
export constexpr StringView CursorConstrained = "Nihility\\Window\\CursorConstrained";

//Graphics

export constexpr StringView VSync = "Nihility\\Graphics\\VSync";

//Platform

export constexpr StringView Dpi = "Nihility\\Platform\\Dpi";

//Audio

export constexpr StringView ChannelCount = "Nihility\\Audio\\ChannelCount";
export constexpr StringView MasterVolume = "Nihility\\Audio\\MasterVolume";
export constexpr StringView UnfocusedAudio = "Nihility\\Audio\\UnfocusedAudio";

export class Settings
{
	enum SettingType
	{
		SETTING_TYPE_STRING = 1,
		SETTING_TYPE_BINARY = 3,
		SETTING_TYPE_32BIT = 4,
		SETTING_TYPE_64BIT = 11,
	};

public:
	template <NonPointer Type>
	static bool CreateSetting(const StringView& path, const Type& defaultValue);

	template <NonPointer Type>
	static bool GetSetting(const StringView& path, Type& value);

	template <NonPointer Type>
	static bool SetSetting(const StringView& path, const Type& value);

private:
	static bool Initialize();
	static void Shutdown();

	static bool CreateSetting(const StringView& path, const U8* defaultValue, UL32 size, I32 type);
	static bool GetSetting(const StringView& path, U8* data, UL32 size);
	static bool SetSetting(const StringView& path, const U8* data, UL32 size, I32 type);

	static U32 GetSettingSize(const StringView& path);

	static HKEY registryKey;

	STATIC_CLASS(Settings);
	friend class Engine;
};

template <NonPointer Type>
bool Settings::CreateSetting(const StringView& path, const Type& defaultValue)
{
	if constexpr (IsStringType<Type> || IsStringViewType<Type>)
	{
		return CreateSetting(path, defaultValue.Data(), (UL32)defaultValue.Size() + 1, SETTING_TYPE_STRING);
	}
	else if constexpr (sizeof(Type) <= 4)
	{
		return CreateSetting(path, (const U8*)&defaultValue, (UL32)sizeof(Type), SETTING_TYPE_32BIT);
	}
	else if constexpr (sizeof(Type) <= 8)
	{
		return CreateSetting(path, (const U8*)&defaultValue, (UL32)sizeof(Type), SETTING_TYPE_64BIT);
	}
	else
	{
		return CreateSetting(path, (const U8*)&defaultValue, (UL32)sizeof(Type), SETTING_TYPE_BINARY);
	}
}

template <NonPointer Type>
bool Settings::GetSetting(const StringView& path, Type& value)
{
	if constexpr (IsStringType<Type>)
	{
		U32 size = GetSettingSize(path);
		value.Resize(size - 1);
		return GetSetting(path, value.Data(), (UL32)size);
	}
	else if constexpr (sizeof(Type) <= 4)
	{
		return GetSetting(path, (U8*)&value, (UL32)sizeof(Type));
	}
	else if constexpr (sizeof(Type) <= 8)
	{
		return GetSetting(path, (U8*)&value, (UL32)sizeof(Type));
	}
	else
	{
		U32 size = GetSettingSize(path);
		return GetSetting(path, (U8*)&value, (UL32)sizeof(Type));
	}
}

template <NonPointer Type>
bool Settings::SetSetting(const StringView& path, const Type& value)
{
	if constexpr (IsStringType<Type> || IsStringViewType<Type>)
	{
		return SetSetting(path, value.Data(), (UL32)value.Size() + 1, SETTING_TYPE_STRING);
	}
	else if constexpr (sizeof(Type) <= 4)
	{
		return SetSetting(path, (const U8*)&value, (UL32)sizeof(Type), SETTING_TYPE_32BIT);
	}
	else if constexpr (sizeof(Type) <= 8)
	{
		return SetSetting(path, (const U8*)&value, (UL32)sizeof(Type), SETTING_TYPE_64BIT);
	}
	else
	{
		return SetSetting(path, (const U8*)&value, (UL32)sizeof(Type), SETTING_TYPE_BINARY);
	}
}