#include "Keyboard.hpp"
#include "Core\Logger.hpp"

Keyboard::Keyboard(void* handle) : device(handle, DEVICE_TYPE_KEYBOARD)
{

}

Keyboard::~Keyboard()
{
	Destroy();
}

void Keyboard::Destroy()
{
	device.UnloadDevice();
	device.Destroy();
}

void Keyboard::Update()
{
	//if (HidP_GetData(HidP_Input, stateBuffer, &stateLength, preparsedData, reportBuffer, capabilities.InputReportByteLength) != HIDP_STATUS_SUCCESS)
	//{
	//	Logger::Error("Failed to get keyboard data!");
	//	return;
	//}

	//BreakPoint;
}