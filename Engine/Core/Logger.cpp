module;

#include "Defines.hpp"

module Core:Logger;

import :File;
import Containers;
import Multithreading;
import Platform;

File log = File("Log.log", FILE_OPEN_LOG);
File console = File("CONOUT$", FILE_OPEN_CONSOLE);

SafeQueue<String, 64> Logger::messageQueue;
bool Logger::writing = false;

bool Logger::Initialize()
{
	Platform::SetConsoleWindowTitle("Nihility Console");
	return log.Opened() && console.Opened();
}

void Logger::Shutdown()
{
	console.Destroy();
	log.Destroy();
}

void Logger::Write(String&& message) noexcept
{
	log.Write(message);
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
		log.Write(message);
		console.Write(message);
	}

	writing = false;
}
