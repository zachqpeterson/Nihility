#include "Logger.hpp"

#include "Platform\Platform.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <fileapi.h>

bool Logger::Initialize()
{
	Platform::SetConsoleWindowTitle("Nihility Console");

	console = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	return true;
}

void Logger::Shutdown()
{
	CloseHandle(console);
	log.Close();
}

void Logger::Write(const String& str)
{
	UL32 wrote;
	WriteFile(console, str.Data(), (UL32)str.Size(), &wrote, nullptr); //TODO: Thread-Safe
	log.Write(str);
}

#endif