#include "Logger.hpp"

File Logger::logFile("Log.txt", FILE_OPEN_LOG);
File Logger::console("CONOUT$", FILE_OPEN_CONSOLE);

bool Logger::Initialize()
{
	return true;
}

void Logger::Shutdown()
{
	logFile.Destroy();
	console.Destroy();
}