#include "Logger.hpp"

#include "File.hpp"
#include "Containers\String.hpp"
#include "Platform\Platform.hpp"

String Logger::fatalTag{ "\033[0;41m[FATAL]:\033[0m " };
String Logger::errorTag{ "\033[0;31m[ERROR]:\033[0m " };
String Logger::warnTag{ "\033[1;33m[WARN]:\033[0m  " };
String Logger::infoTag{ "\033[1;32m[INFO]:\033[0m  " };
String Logger::debugTag{ "\033[0;36m[DEBUG]:\033[0m " };
String Logger::traceTag{ "\033[1;30m[TRACE]:\033[0m " };
String Logger::endLine{ "\n" };

File Logger::log{ "Log.log", FILE_OPEN_LOG };
File Logger::console{ "CONOUT$", FILE_OPEN_CONSOLE };

bool Logger::Initialize()
{
	Platform::SetConsoleWindowTitle("Nihility Console");
	return log.Opened() && console.Opened();
}

void Logger::Shutdown()
{
	fatalTag.Destroy();
	errorTag.Destroy();
	warnTag.Destroy();
	infoTag.Destroy();
	debugTag.Destroy();
	traceTag.Destroy();
	endLine.Destroy();

	console.Destroy();
	log.Destroy();
}

void Logger::Write(const String& message)
{
	log.Write(message);
	console.Write(message);
}