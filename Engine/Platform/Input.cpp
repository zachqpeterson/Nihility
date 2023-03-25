#include "Input.hpp"

#include "Device.hpp"
#include "Platform.hpp"
#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"

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

	for (U32 i = 0; i < deviceCount; ++i)
	{
		RAWINPUTDEVICELIST device = deviceList[i];

		String path;
		U32 len = (U32)path.Capacity();
		len = GetRawInputDeviceInfoA(device.hDevice, RIDI_DEVICENAME, path.Data(), &len);
		path.Resize(len);

		HANDLE ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

		Logger::Debug(path);

		if (ntHandle != INVALID_HANDLE_VALUE)
		{
			Device device(ntHandle);

			if (device.openHandle) { devices.Push(Move(device)); }
		}
	}

	GUID guid;
	HidD_GetHidGuid(&guid);

	if ((devInfoSet = SetupDiGetClassDevsA(&guid, nullptr, wd.window, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)) == INVALID_HANDLE_VALUE)
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
		if(!SetupDiGetDeviceInterfaceDetailA(devInfoSet, &interfaceData, nullptr, 0, &size, nullptr) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
		{
			Logger::Error("Failed to get device interface detail size, {}", error);
			continue;
		}

		PSP_DEVICE_INTERFACE_DETAIL_DATA_A interfaceDetailData;
		Memory::AllocateSize(&interfaceDetailData, size);
		interfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

		SP_DEVINFO_DATA infoData{};
		infoData.cbSize = sizeof(SP_DEVINFO_DATA);

		if (!SetupDiGetDeviceInterfaceDetailA(devInfoSet, &interfaceData, interfaceDetailData, size, nullptr, &infoData))
		{ 
			Logger::Error("Error when getting device interface details, {}", GetLastError());
			continue;
		}

		UL32 dataType;

		if (!SetupDiGetDeviceRegistryPropertyA(devInfoSet, &infoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, &dataType, nullptr, 0, &size) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
		{
			Logger::Error("Error when getting device registry property, {}", error);
			continue;
		}

		String path("\\\\?\\GLOBALROOT");

		if (!SetupDiGetDeviceRegistryPropertyA(devInfoSet, &infoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, &dataType, (U8*)path.Data() + path.Size(), size, nullptr))
		{
			Logger::Error("Error when getting device registry property, {}", GetLastError());
			continue;
		}
		
		path.Resize();

		//Device device(path);
		//if (device.openHandle) { devices.Push(Move(device)); }
	}

	if (error = GetLastError() != ERROR_NO_MORE_ITEMS) { Logger::Error("Error when enumurating devices, {}", error); return false; }
	
	return true;
}

void Input::Shutdown()
{
	SetupDiDestroyDeviceInfoList(devInfoSet);
}

void Input::Update(HRAWINPUT handle)
{
	
}

void Input::AddDevice(void* handle)
{

}

void Input::RemoveDevice(void* handle)
{

}

#endif