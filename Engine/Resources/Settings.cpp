#include "Settings.hpp"

#include "Core\File.hpp"
#include "Core\Logger.hpp"
#include "Containers\String.hpp"

//TODO: Binary writing and reading

bool Settings::Initialize()
{
	Logger::Trace("Loading Settings...");

	File config("Settings.cfg", FILE_OPEN_TEXT_READ_SEQ);

	if (config.Opened())
	{
		//TODO: Read Settings

		String str;

		if (config.ReadAllString(str))
		{
			config.ReadT(CHANNEL_COUNT);
			config.ReadT(MASTER_VOLUME);
			config.ReadT(MUSIC_VOLUME);
			config.ReadT(SFX_VOLUME);
			config.ReadT(WINDOW_WIDTH);
			config.ReadT(WINDOW_HEIGHT);
			config.ReadT(WINDOW_WIDTH_SMALL);
			config.ReadT(WINDOW_HEIGHT_SMALL);
			config.ReadT(WINDOW_POSITION_X);
			config.ReadT(WINDOW_POSITION_Y);
			config.ReadT(WINDOW_POSITION_X_SMALL);
			config.ReadT(WINDOW_POSITION_Y_SMALL);
			config.ReadT(TARGET_FRAMETIME);
			config.ReadT(TARGET_FRAMETIME_SUSPENDED);
			config.ReadT(MSAA_COUNT);
			config.ReadT(DPI);
		}

		config.Close();
	}

	return true;
}

void Settings::Shutdown()
{
	File config("Settings.cfg", FILE_OPEN_BINARY_WRITE_NEW);

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