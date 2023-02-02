#pragma once

#include "Defines.hpp"

class Logger
{
public:


private:
	static bool Initialize();
	static void Shutdown();

	STATIC_CLASS(Logger);
	friend class Engine;
};

inline bool Logger::Initialize()
{
	return true;
}

inline void Logger::Shutdown()
{

}