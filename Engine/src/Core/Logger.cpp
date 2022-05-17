#include "Logger.hpp"

#include "File.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Containers/String.hpp"

// TODO: temporary
#include <stdarg.h>

File Logger::log;

bool Logger::Initialize()
{
    if (!log.Open("console.log", FILE_MODE_WRITE, false)) 
    {
        Platform::ConsoleWrite("[ERROR]: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
        return false;
    }

    LOG_INFO("Logger initialized.");

    return true;
}

void Logger::Shutdown()
{
    log.Close();
}

void Logger::LogOutput(LogLevel level, const char* message, ...)
{
    // TODO: Threaded
    String levelStrings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: " };

    String outMessage;

    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    outMessage.FormatV(message, arg_ptr);
    va_end(arg_ptr);

    outMessage.Format("%s%s\n", (const char*)levelStrings[level], (const char*)outMessage);

    Platform::ConsoleWrite(outMessage, level);

    //TODO: Queue a copy to be written to the log file.
    log.Write(outMessage);
}

//NOTE: Defined in Defines.hpp
void ReportAssertion(const char* expression, const char* message, const char* file, I32 line)
{
    LOG_FATAL("Expression '%s' failed with message '%s' in file '%s' on line %d", (const char*)expression, (const char*)message, (const char*)file, line);
}