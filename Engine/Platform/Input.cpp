#include "Input.hpp"

#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <hidsdi.h>
#include <SetupAPI.h>
#pragma comment(lib ,"hid.lib")
#pragma comment(lib ,"setupapi.lib")

bool Input::Initialize()
{
	Logger::Trace("Initializing Input...");



	return true;
}

void Input::Shutdown()
{

}

void Input::Update()
{

}

#endif