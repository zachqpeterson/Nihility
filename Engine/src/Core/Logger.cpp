#include "Logger.hpp"

#include "File.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Containers/String.hpp"

File Logger::log;
void* Logger::consoleHandle;
void* Logger::errorHandle;

void Logger::Shutdown()
{
	log.Close();
}

#if defined PLATFORM_WINDOWS

#include <Windows.h>

bool Logger::Initialize()
{
	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	errorHandle = GetStdHandle(STD_ERROR_HANDLE);

	String str = "Initializing Logger...";
	Print(str, LOG_LEVEL_INFO);
	SetConsoleTitleW(L"Nihility Console");

	if (!log.Open("console.log", FILE_MODE_WRITE, false))
	{
		String str = "Unable to open console.log for writing.";
		PrintError(str, LOG_LEVEL_ERROR);
		return false;
	}

	return true;
}

void Logger::Print(String& message, LogLevel level)
{
	static constexpr const char* levelStrings[4] = { "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: " };
	static constexpr U8 levels[4] = { 6, 10, 11, 8 };

	message.Surround(levelStrings[level], "\n");

	SetConsoleTextAttribute(consoleHandle, levels[level]);
	WriteConsoleA(consoleHandle, message, (DWORD)message.Length(), nullptr, nullptr);

	log.Write(message);
}

void Logger::PrintError(String& message, LogLevel level)
{
	static constexpr const char* levelStrings[2] = { "[FATAL]: ", "[ERROR]: " };
	static constexpr U8 levels[2] = { 64, 4 };

	message.Surround(levelStrings[level], "\n");

	SetConsoleTextAttribute(errorHandle, levels[level]);
	WriteConsoleA(errorHandle, message, (DWORD)message.Length(), nullptr, nullptr);

	log.Write(message);
}

#endif

//NOTE: Defined in Defines.hpp
void ReportAssertion(const char* expression, const char* message, const char* file, I32 line)
{
	Logger::Fatal("Expression '{}' failed with message '{}' in file '{}' on line {}", expression, message, file, line);
}