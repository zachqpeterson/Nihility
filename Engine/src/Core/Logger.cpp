#include "Logger.hpp"

#include "File.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Containers/String.hpp"

// TODO: temporary
#include <stdarg.h>

struct LoggerState {
    File log;
};

static LoggerState* loggerState;

bool Logger::Initialize(void* state)
{
    loggerState = (LoggerState*)state;

    if (!loggerState->log.Open("console.log", FILE_MODE_WRITE, false)) {
        Platform::ConsoleWrite("[ERROR]: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
        return false;
    }

    return true;
}

void* Logger::Shutdown()
{
    loggerState->log.Close();
    return loggerState;
}

void Logger::LogOutput(LogLevel level, const char* message, ...)
{
    // TODO: Threaded
    const char* levelStrings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: " };

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!

    String out_message(new char[32000]);
    Memory::ZeroMemory(out_message, out_message.Length());

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    out_message.FormatV(message, arg_ptr);
    va_end(arg_ptr);

    // Prepend log level to message.
    out_message.Format("%s%s\n", levelStrings[level], (const char*)out_message);

    // Print accordingly
    Platform::ConsoleWrite(out_message, level);

    // Queue a copy to be written to the log file.
    loggerState->log.Write(out_message);
}

const U64 Logger::GetMemoryRequirements()
{
    return sizeof(LoggerState);
}