#include "Logger.hpp"

#include "File.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Containers/String.hpp"

File Logger::log;

bool Logger::Initialize()
{
    if (!log.Open("console.log", FILE_MODE_WRITE, false))
    {
        Platform::ConsoleWrite("[ERROR]: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
        return false;
    }

    Logger::Info("Logger initialized.");

    return true;
}

void Logger::Shutdown()
{
    log.Close();
}

void Logger::LogOutput(LogLevel level, String& message)
{
    static const char* levelStrings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: " };

    message.Surround(levelStrings[level], "\n");

    Platform::ConsoleWrite(message, level);

    log.Write(message);
}

//NOTE: Defined in Defines.hpp
void ReportAssertion(const char* expression, const char* message, const char* file, I32 line)
{
    Logger::Fatal("Expression '{}' failed with message '{}' in file '{}' on line {}", expression, message, file, line);
}