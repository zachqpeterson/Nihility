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

		//Mouse: 378DE44C-56EF-11D1-BC8C-00A0C91405DD
		//Keyboard: 884b96c3-56ef-11d1-bc8c-00a0c91405dd

		//4D36E96F-E325-11CE-BFC1-08002BE10318 mouse
		//745A17A0-74D3-11D0-B6FE-00A0C90F57DA controller
		//745A17A0-74D3-11D0-B6FE-00A0C90F57DA INTERACTIVE_CONTROL
		//745A17A0-74D3-11D0-B6FE-00A0C90F57DA mouse (no buttons)

		Device device(interfaceDetailData->DevicePath);
		if (device.openHandle) { devices.Push(Move(device)); }
	}

	if (error = GetLastError() != ERROR_NO_MORE_ITEMS) { Logger::Error("Error when enumurating devices, {}", error); return false; }

	BreakPoint;

	RAWINPUTDEVICE rid[7];

	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_JOYSTICK;
	rid[0].dwFlags = RIDEV_DEVNOTIFY;
	rid[0].hwndTarget = wd.window;

	rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[1].usUsage = HID_USAGE_GENERIC_GAMEPAD;
	rid[1].dwFlags = RIDEV_DEVNOTIFY;
	rid[1].hwndTarget = wd.window;

	rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[2].usUsage = HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER;
	rid[2].dwFlags = RIDEV_DEVNOTIFY;
	rid[2].hwndTarget = wd.window;

	rid[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[3].usUsage = HID_USAGE_GENERIC_POINTER;
	rid[3].dwFlags = RIDEV_DEVNOTIFY;
	rid[3].hwndTarget = wd.window;

	rid[4].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[4].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[4].dwFlags = RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
	rid[4].hwndTarget = wd.window;

	rid[5].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[5].usUsage = HID_USAGE_GENERIC_KEYPAD;
	rid[5].dwFlags = RIDEV_DEVNOTIFY;
	rid[5].hwndTarget = wd.window;

	rid[6].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[6].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	rid[6].dwFlags = RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
	rid[6].hwndTarget = wd.window;

	if (!RegisterRawInputDevices(rid, 7, sizeof(RAWINPUTDEVICE))) { Logger::Error("Failed to register devices, {}!", GetLastError()); return false; }

	//U32 expectedNumberOfHIDs = 0;
	//if (GetRawInputDeviceList(nullptr, &expectedNumberOfHIDs, sizeof(RAWINPUTDEVICELIST)))
	//{
	//	//TODO: Error, no device or no support for raw input
	//	return false;
	//}
	//
	//RAWINPUTDEVICELIST arr[100];
	//if (GetRawInputDeviceList(arr, &expectedNumberOfHIDs, sizeof(RAWINPUTDEVICELIST)) != expectedNumberOfHIDs)
	//{
	//	//TODO: Error
	//	return false;
	//}
	//
	//for (U32 i = 0; i < expectedNumberOfHIDs; ++i)
	//{
	//	RAWINPUTDEVICELIST hidDescriptor = arr[i];
	//	switch (hidDescriptor.dwType)
	//	{
	//	case RIM_TYPEMOUSE: {
	//		Mouse mouse(hidDescriptor.hDevice);
	//		if (mouse.Valid()) { mice.Push(mouse); }
	//	} break;
	//	case RIM_TYPEKEYBOARD: {
	//		Keyboard keyboard(hidDescriptor.hDevice);
	//		if (keyboard.Valid()) { keyboards.Push(keyboard); }
	//	} break;
	//	case RIM_TYPEHID: {
	//		Controller controller(hidDescriptor.hDevice);
	//		if (controller.Valid()) { controllers.Push(controller); }
	//	} break;
	//	default: break;
	//	}
	//}

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