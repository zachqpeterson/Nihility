#include "Logger.hpp"

#include "File.hpp"
#include "Containers\String.hpp"
#include "Platform\Platform.hpp"

const String Logger::fatalTag{ "\033[0;41m[FATAL]:\033[0m " };
const String Logger::errorTag{ "\033[0;31m[ERROR]:\033[0m " };
const String Logger::warnTag{ "\033[1;33m[WARN]:\033[0m  " };
const String Logger::infoTag{ "\033[1;32m[INFO]:\033[0m  " };
const String Logger::debugTag{ "\033[0;36m[DEBUG]:\033[0m " };
const String Logger::traceTag{ "\033[1;30m[TRACE]:\033[0m " };
const String Logger::endLine{ "\n" };

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