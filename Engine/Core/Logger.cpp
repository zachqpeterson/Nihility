#include "Logger.hpp"

#include "File.hpp"

#include "Platform\Jobs.hpp"
#include "Platform\Platform.hpp"

File logFile = File("Log.log", FILE_OPEN_LOG);
File console = File("CONOUT$", FILE_OPEN_CONSOLE);

SafeQueue<String, 64> Logger::messageQueue;
bool Logger::writing = false;

bool Logger::Initialize()
{
	Platform::SetConsoleWindowTitle("Nihility Console");
	return logFile.Opened() && console.Opened();
}

void Logger::Shutdown()
{
	console.Destroy();
	logFile.Destroy();
}

void Logger::Write(String&& message) noexcept
{
	logFile.Write(message);
	console.Write(message);
}

void Logger::Queue(String&& message) noexcept
{
	messageQueue.Push(Move(message));

	if (!SafeCheckAndSet((U8*)&writing, 0))
	{
		Jobs::Excecute(Output, JOB_PRIORITY_LOW);
	}
}

void Logger::Output()
{
	String message;
	while (messageQueue.Pop(message))
	{
		logFile.Write(message);
		console.Write(message);
	}

	writing = false;
}
