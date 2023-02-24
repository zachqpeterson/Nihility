#include "Input.hpp"

#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"
#include "Containers\WString.hpp"
#include "Controller.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"

Vector<Mouse> Input::mice;
Vector<Keyboard> Input::keyboards;
Vector<Controller> Input::controllers;

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>

bool Input::Initialize()
{
	Logger::Trace("Initializing Input...");

	const WindowData& wd = Platform::GetWindowData();

	RAWINPUTDEVICE rid[4];

	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
	rid[0].dwFlags = RIDEV_DEVNOTIFY;
	rid[0].hwndTarget = wd.window;

	rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
	rid[1].dwFlags = RIDEV_DEVNOTIFY;
	rid[1].hwndTarget = wd.window;

	rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[2].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[2].dwFlags = RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
	rid[2].hwndTarget = wd.window;

	rid[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	rid[3].dwFlags = RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
	rid[3].hwndTarget = wd.window;

	if (!RegisterRawInputDevices(rid, 4, sizeof(RAWINPUTDEVICE))) { Logger::Error("Failed to register devices!"); return false; }

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
		case RIM_TYPEMOUSE: {
			Mouse mouse(hidDescriptor.hDevice);
			mice.Push(mouse);
		} break;
		case RIM_TYPEKEYBOARD: {
			Keyboard keyboard(hidDescriptor.hDevice);
			keyboards.Push(keyboard);
		} break;
		case RIM_TYPEHID: {
			Controller controller(hidDescriptor.hDevice);
			if (controller.openHandle)
			{
				controllers.Push(controller);
			}
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
	for (Keyboard& keyboard : keyboards)
	{
		keyboard.Update();
	}
}

#endif