#include "Settings.hpp"

#include "Core\File.hpp"
#include "Core\Logger.hpp"
#include "Containers\String.hpp"

bool Settings::Initialize()
{
	Logger::Trace("Loading Settings...");

	File config("Settings.cfg", FILE_OPEN_READ_SETTINGS);

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

	File config("Settings.cfg", FILE_OPEN_WRITE_SETTINGS);

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