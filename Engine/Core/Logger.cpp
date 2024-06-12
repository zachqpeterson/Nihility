#include "Logger.hpp"

import Containers;

#include "File.hpp"
#include "Platform\Platform.hpp"

File Logger::log{ "Log.log", FILE_OPEN_LOG };
File Logger::console{ "CONOUT$", FILE_OPEN_CONSOLE };

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

void Logger::Write(const String& message)
{
	log.Write(message);
	console.Write(message);
}