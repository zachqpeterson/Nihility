#include "Settings.hpp"

#include "Core\File.hpp"
#include "Core\Logger.hpp"
#include "Containers\String.hpp"

bool Settings::Initialize()
{
	Logger::Trace("Loading Settings...");

	File config("Settings.cfg", FILE_OPEN_READ_SEQ);

	if (config.Opened())
	{
		config.Read(CHANNEL_COUNT);
		config.Read(MASTER_VOLUME);
		config.Read(MUSIC_VOLUME);
		config.Read(SFX_VOLUME);
		config.Read(WINDOW_WIDTH);
		config.Read(WINDOW_HEIGHT);
		config.Read(WINDOW_WIDTH_SMALL);
		config.Read(WINDOW_HEIGHT_SMALL);
		config.Read(WINDOW_POSITION_X);
		config.Read(WINDOW_POSITION_Y);
		config.Read(WINDOW_POSITION_X_SMALL);
		config.Read(WINDOW_POSITION_Y_SMALL);
		config.Read(TARGET_FRAMETIME);
		config.Read(TARGET_FRAMETIME_SUSPENDED);
		config.Read(MSAA_COUNT);
		config.Read(DPI);

		config.Close();
	}

	return true;
}

void Settings::Shutdown()
{
	File config("Settings.cfg", FILE_OPEN_WRITE_NEW);

	if (config.Opened())
	{
		config.Write(CHANNEL_COUNT);
		config.Write(MASTER_VOLUME);
		config.Write(MUSIC_VOLUME);
		config.Write(SFX_VOLUME);
		config.Write(WINDOW_WIDTH);
		config.Write(WINDOW_HEIGHT);
		config.Write(WINDOW_WIDTH_SMALL);
		config.Write(WINDOW_HEIGHT_SMALL);
		config.Write(WINDOW_POSITION_X);
		config.Write(WINDOW_POSITION_Y);
		config.Write(WINDOW_POSITION_X_SMALL);
		config.Write(WINDOW_POSITION_Y_SMALL);
		config.Write(TARGET_FRAMETIME);
		config.Write(TARGET_FRAMETIME_SUSPENDED);
		config.Write(MSAA_COUNT);
		config.Write(DPI);

		config.Close();
	}
	else
	{
		Logger::Error("Failed to open Settings.cnf!");
	}
}