module;

#include "Defines.hpp"
#include "Platform\Platform.hpp"

module Core:Logger;

import :File;
import Containers;
import Multithreading;

File log{ File("Log.log", FILE_OPEN_LOG) };
File console{ File("CONOUT$", FILE_OPEN_CONSOLE) };

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

void Logger::Update()
{
	while (!messageQueue.Empty())
	{
		String message;
		messageQueue.Pop(message);

		log.Write(message);
		console.Write(message);
	}

	writing = false;
}

void Logger::Write(String&& message) noexcept
{
	messageQueue.Push(Move(message));

	if (!SafeCheckAndSet((U8*)&writing, 0))
	{
		Jobs::Excecute(Update, JOB_PRIORITY_LOW);
	}
}