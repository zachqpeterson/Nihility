#include "Logger.hpp"

fast_io::obuf_file Logger::log("log.txt", fast_io::open_mode::out);

bool Logger::Initialize()
{
	return true;
}

void Logger::Shutdown()
{
	log.close();
}