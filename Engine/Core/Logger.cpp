#include "Logger.hpp"

#include "File.hpp"
#include "Containers\String.hpp"
#include "Platform\Platform.hpp"

static File log{ "Log.log", FILE_OPEN_LOG };
static File console{ "CONOUT$", FILE_OPEN_CONSOLE };

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