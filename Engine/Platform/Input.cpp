#include "Input.hpp"

#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"
#include "Containers\WString.hpp"
#include "Controller.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>

bool Input::Initialize()
{
	Logger::Trace("Initializing Input...");

	U32 expectedNumberOfHIDs = 0;
	if (GetRawInputDeviceList(nullptr, &expectedNumberOfHIDs, sizeof(RAWINPUTDEVICELIST)))
	{
		//TODO: Error, no device or no support for raw input
		return false;
	}

	RAWINPUTDEVICELIST arr[100];
	if (GetRawInputDeviceList(arr, &expectedNumberOfHIDs, sizeof(RAWINPUTDEVICELIST)) != expectedNumberOfHIDs)
	{
		//TODO: Error
		return false;
	}

	for (U32 i = 0; i < expectedNumberOfHIDs; ++i)
	{
		RAWINPUTDEVICELIST hidDescriptor = arr[i];
		switch (hidDescriptor.dwType)
		{
		case RIM_TYPEMOUSE: {//TODO:

			Controller controller(hidDescriptor.hDevice);

		} break;
		case RIM_TYPEKEYBOARD: { //TODO:



			Controller controller(hidDescriptor.hDevice);
		} break;
		case RIM_TYPEHID: {
			Controller controller(hidDescriptor.hDevice);

		} break;
		default: break;
		}
	}

	return true;
}

void Input::Shutdown()
{

}

void Input::Update()
{

}

#endif