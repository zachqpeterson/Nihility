#include "Logger.hpp"

#include "String.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>

void* Logger::consoleHandle;
void* Logger::errorHandle;

String Logger::fatalTag;
String Logger::errorTag;
String Logger::warnTag;
String Logger::debugTag;
String Logger::infoTag;
String Logger::traceTag;
String Logger::endLine;

bool Logger::Initialize()
{
#if defined PLATFORM_WINDOWS
	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	errorHandle = GetStdHandle(STD_ERROR_HANDLE);

	SetConsoleTitleW(L"Nihility Console");

#endif
	fatalTag = "[FATAL]: ";
	errorTag = "[ERROR]: ";
	warnTag = "[WARN]:  ";
	debugTag = "[DEBUG]: ";
	infoTag = "[INFO]:  ";
	traceTag = "[TRACE]: ";
	endLine = "\n";

	//TODO: Open log file

	return true;
}

void Logger::Shutdown()
{

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