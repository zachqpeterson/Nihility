#include "Mouse.hpp"
#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

Mouse::Mouse(void* handle) : device(handle, DEVICE_TYPE_MOUSE)
{

}

Mouse::~Mouse()
{
	Destroy();
}

void Mouse::Destroy()
{
	device.UnloadDevice();
	device.Destroy();
}

#endif