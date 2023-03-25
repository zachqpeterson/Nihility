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