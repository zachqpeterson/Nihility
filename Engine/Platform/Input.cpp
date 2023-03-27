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
	rid[4].dwFlags = RIDEV_DEVNOTIFY;
	rid[4].hwndTarget = nullptr;

	rid[5].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[5].usUsage = HID_USAGE_GENERIC_GAMEPAD;
	rid[5].dwFlags = RIDEV_DEVNOTIFY;
	rid[5].hwndTarget = nullptr;

	rid[6].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[6].usUsage = HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER;
	rid[6].dwFlags = RIDEV_DEVNOTIFY;
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

}

void Input::Update(HRAWINPUT handle)
{
	U32 size = 0;
	RAWINPUTHEADER header;

	if (GetRawInputData(handle, RID_HEADER, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1) { return; }
	if (GetRawInputData(handle, RID_HEADER, &header, &size, sizeof(RAWINPUTHEADER)) != size) { return; }

	switch (header.dwType)
	{
	case RIM_TYPEMOUSE: {
		RAWMOUSE mouse;

		if (GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1) { return; }
		if (GetRawInputData(handle, RID_INPUT, &mouse, &size, sizeof(RAWINPUTHEADER)) != size) { return; }
	} break;

	case RIM_TYPEKEYBOARD: {
		RAWKEYBOARD keyboard;

		if (GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1) { return; }
		if (GetRawInputData(handle, RID_INPUT, &keyboard, &size, sizeof(RAWINPUTHEADER)) != size) { return; }
	} break;

	case RIM_TYPEHID: {
		RAWHID hid;

		//Use calibration

		if (GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1) { return; }
		if (GetRawInputData(handle, RID_INPUT, &hid, &size, sizeof(RAWINPUTHEADER)) != size) { return; }
	} break;
	}

	//TODO: get device from handle

	//TODO: call device.Update
}

void Input::AddDevice(void* handle)
{
	U32 size = 0;
	RAWINPUTHEADER header;
	PHIDP_PREPARSED_DATA preparsedData;
	HIDP_CAPS capabilities;

	if (GetRawInputData((HRAWINPUT)handle, RID_HEADER, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1) { return; }
	if (GetRawInputData((HRAWINPUT)handle, RID_HEADER, &header, &size, sizeof(RAWINPUTHEADER)) != size) { return; }

	String path;
	U32 len = (U32)path.Capacity();
	GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, path.Data(), &len);

	HANDLE ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr); //TODO: close

	if (!HidD_GetPreparsedData(ntHandle, &preparsedData)) { Logger::Trace("Failed to get data, {}, skipping...", GetLastError()); }
	if (HidP_GetCaps(preparsedData, &capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); }

	switch (header.dwType)
	{
	case RIM_TYPEMOUSE: {
		if (capabilities.Usage == HID_USAGE_GENERIC_MOUSE || HID_USAGE_GENERIC_POINTER)
		{
			//good
		}
	} break;
	case RIM_TYPEKEYBOARD: {
		if (capabilities.Usage == HID_USAGE_GENERIC_KEYBOARD || capabilities.Usage == HID_USAGE_GENERIC_KEYPAD)
		{
			//good
		}
	} break;
	case RIM_TYPEHID: {
		if (capabilities.Usage == HID_USAGE_GENERIC_GAMEPAD || capabilities.Usage == HID_USAGE_GENERIC_JOYSTICK || capabilities.Usage == HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER)
		{
			//good
		}
	} break;
	}

	CloseHandle(ntHandle);
}

void Input::RemoveDevice(void* handle)
{

}

#endif