#include "Input.hpp"

#include "Device.hpp"
#include "Platform.hpp"
#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"

Vector<Device> Input::devices;

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#pragma comment(lib ,"setupapi.lib")

bool Input::Initialize()
{
	Logger::Trace("Initializing Input...");

	const WindowData& wd = Platform::GetWindowData();

	RAWINPUTDEVICE rid[7];

	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_POINTER;
	rid[0].dwFlags = RIDEV_DEVNOTIFY;
	rid[0].hwndTarget = nullptr;

	rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[1].dwFlags = RIDEV_DEVNOTIFY | RIDEV_NOLEGACY;
	rid[1].hwndTarget = nullptr;

	rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[2].usUsage = HID_USAGE_GENERIC_KEYPAD;
	rid[2].dwFlags = RIDEV_DEVNOTIFY;
	rid[2].hwndTarget = nullptr;

	rid[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	rid[3].dwFlags = RIDEV_DEVNOTIFY | RIDEV_NOLEGACY;
	rid[3].hwndTarget = nullptr;

	rid[4].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[4].usUsage = HID_USAGE_GENERIC_JOYSTICK;
	rid[4].dwFlags = RIDEV_DEVNOTIFY | RIDEV_REMOVE;
	rid[4].hwndTarget = nullptr;

	rid[5].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[5].usUsage = HID_USAGE_GENERIC_GAMEPAD;
	rid[5].dwFlags = RIDEV_DEVNOTIFY | RIDEV_REMOVE;
	rid[5].hwndTarget = nullptr;

	rid[6].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[6].usUsage = HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER;
	rid[6].dwFlags = RIDEV_DEVNOTIFY | RIDEV_REMOVE;
	rid[6].hwndTarget = nullptr;

	if (!RegisterRawInputDevices(rid, 7, sizeof(RAWINPUTDEVICE))) { Logger::Error("Failed to register devices, {}!", GetLastError()); return false; }

	U32 deviceCount = 0;
	RAWINPUTDEVICELIST* deviceList = nullptr;

	if (GetRawInputDeviceList(nullptr, &deviceCount, sizeof(RAWINPUTDEVICELIST))) { return false; }
	if (deviceCount == 0) { return true; }

	Memory::AllocateArray(&deviceList, deviceCount);

	deviceCount = GetRawInputDeviceList(deviceList, &deviceCount, sizeof(RAWINPUTDEVICELIST));

	for (U32 i = 0; i < deviceCount; ++i) { AddDevice(deviceList[i].hDevice); }

	Memory::FreeArray(&deviceList);
	
	return true;
}

void Input::Shutdown()
{
	devices.Destroy();
}

void Input::Update()
{
	for (Device& device : devices)
	{
		U32 size;
		U8* buffer = device.ReadInput(size);

		if (buffer)
		{
			BreakPoint;
		}
	}
}

void Input::AddDevice(void* handle)
{
	Device device(handle);
	if (device.valid) { devices.Push(Move(device)); } //TODO: better data structure, probably HashMap
}

void Input::RemoveDevice(void* handle)
{

}

#endif