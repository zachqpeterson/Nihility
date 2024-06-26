module;

#include "Defines.hpp"
#include "Platform\Platform.hpp"

module Core:Logger;

import :File;
import Containers;

File log{ File("Log.log", FILE_OPEN_LOG) };
File console{ File("CONOUT$", FILE_OPEN_CONSOLE) };

bool Logger::Initialize()
{
	Platform::SetConsoleWindowTitle("Nihility Console");
	return log.Opened() && console.Opened();
}

void Logger::Shutdown()
{
	console.Destroy();
	log.Destroy();
}

void Logger::Write(String&& message) noexcept
{
	log.Write(message);
	console.Write(message);
}