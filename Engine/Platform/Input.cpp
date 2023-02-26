#include "Input.hpp"

#include "Device.hpp"
#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"
#include "Containers\WString.hpp"

void* Input::devInfoSet;

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

	GUID guid;
	HidD_GetHidGuid(&guid);

	if ((devInfoSet = SetupDiGetClassDevsW(&guid, nullptr, wd.window, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)) == INVALID_HANDLE_VALUE)
	{
		Logger::Error("Failed to get the device info set, {}!", GetLastError());
		return false;
	}

	SP_DEVICE_INTERFACE_DATA interfaceData{};
	interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	UL32 error;
	UL32 index = 0;
	while (SetupDiEnumDeviceInterfaces(devInfoSet, nullptr, &guid, index++, &interfaceData))
	{
		UL32 size;
		if(!SetupDiGetDeviceInterfaceDetailW(devInfoSet, &interfaceData, nullptr, 0, &size, nullptr) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
		{
			Logger::Error("Failed to get device interface detail size, {}", error);
			continue;
		}

		PSP_DEVICE_INTERFACE_DETAIL_DATA_W interfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)Memory::Allocate(size);
		interfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

		SP_DEVINFO_DATA infoData{};
		infoData.cbSize = sizeof(SP_DEVINFO_DATA);

		if (!SetupDiGetDeviceInterfaceDetailW(devInfoSet, &interfaceData, interfaceDetailData, size, nullptr, &infoData))
		{ 
			Logger::Error("Error when getting device interface details, {}", GetLastError());
			continue;
		}

		Device device(interfaceDetailData->DevicePath);
		if (device.openHandle) { devices.Push(Move(device)); }
	}

	if (error = GetLastError() != ERROR_NO_MORE_ITEMS) { Logger::Error("Error when enumurating devices, {}", error); return false; }

	RAWINPUTDEVICE rid[7];

	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_POINTER;
	rid[0].dwFlags = RIDEV_REMOVE | RIDEV_DEVNOTIFY;
	rid[0].hwndTarget = nullptr;
	
	rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[1].dwFlags = RIDEV_REMOVE | RIDEV_DEVNOTIFY;
	rid[1].hwndTarget = nullptr;

	rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[2].usUsage = HID_USAGE_GENERIC_KEYPAD;
	rid[2].dwFlags = RIDEV_REMOVE | RIDEV_DEVNOTIFY;
	rid[2].hwndTarget = nullptr;

	rid[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	rid[3].dwFlags = RIDEV_REMOVE | RIDEV_DEVNOTIFY;
	rid[3].hwndTarget = nullptr;
	
	rid[4].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[4].usUsage = HID_USAGE_GENERIC_JOYSTICK;
	rid[4].dwFlags = RIDEV_REMOVE | RIDEV_DEVNOTIFY;
	rid[4].hwndTarget = nullptr;

	rid[5].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[5].usUsage = HID_USAGE_GENERIC_GAMEPAD;
	rid[5].dwFlags = RIDEV_REMOVE | RIDEV_DEVNOTIFY;
	rid[5].hwndTarget = nullptr;

	rid[6].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[6].usUsage = HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER;
	rid[6].dwFlags = RIDEV_REMOVE | RIDEV_DEVNOTIFY;
	rid[6].hwndTarget = nullptr;
	
	if (!RegisterRawInputDevices(rid, 7, sizeof(RAWINPUTDEVICE))) { Logger::Error("Failed to register devices, {}!", GetLastError()); return false; }
	
	return true;
}

void Input::Shutdown()
{
	SetupDiDestroyDeviceInfoList(devInfoSet);
}

void Input::Update()
{
	for (Device& device : devices)
	{
		device.Update();
	}
}

#endif