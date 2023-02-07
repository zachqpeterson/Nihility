#include "Logger.hpp"

#include "String.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>

void* Logger::consoleHandle;
void* Logger::errorHandle;

String Logger::fatalTag("[FATAL]: ");
String Logger::errorTag("[ERROR]: ");
String Logger::warnTag("[WARN]:  ");
String Logger::debugTag("[DEBUG]: ");
String Logger::infoTag("[INFO]:  ");
String Logger::traceTag("[TRACE]: ");
String Logger::endLine("\n");

bool Logger::Initialize()
{
#if defined PLATFORM_WINDOWS
	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	errorHandle = GetStdHandle(STD_ERROR_HANDLE);

	SetConsoleTitleW(L"Nihility Console");

#endif

	//TODO: Open log file

	return true;
}

void Logger::Shutdown()
{
	//TODO: Close log file
}

void Logger::Print(const String& message, U16 color)
{
	SetConsoleTextAttribute(consoleHandle, color);
	WriteConsoleA(consoleHandle, (const char*)message, (UL32)message.Length(), nullptr, nullptr);
}

void Logger::PrintError(const String& message, U16 color)
{
	SetConsoleTextAttribute(errorHandle, color);
	WriteConsoleA(errorHandle, (const char*)message, (UL32)message.Length(), nullptr, nullptr);
}

#endif