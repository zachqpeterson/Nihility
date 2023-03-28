#include "Device.hpp"

#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

struct HIDCalibration
{
	L32 lMin;
	L32 lCenter;
	L32 lMax;
};

struct HIDAttributes {
	UL32 dwFlags;
	U16 wUsagePage;
	U16 wUsage;
};

struct HIDAxisMapping
{
	U16 usagePage;
	U16 usage;
	bool isCalibrated;
	HIDCalibration calibration;
	String name;
};

struct HIDButtonMapping
{
	U16 usagePage;
	U16 usage;
	String name;
};

Device::Device(void* handle) : riHandle{ handle }
{
	String path;
	U32 len = (U32)path.Capacity();
	GetRawInputDeviceInfoA(riHandle, RIDI_DEVICENAME, path.Data(), &len);

	RID_DEVICE_INFO info;
	info.cbSize = sizeof(RID_DEVICE_INFO);
	U32 size = sizeof(RID_DEVICE_INFO);

	GetRawInputDeviceInfoA(riHandle, RIDI_DEVICEINFO, &info, &size);

	if ((ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)) == INVALID_HANDLE_VALUE) { Logger::Trace("Failed to open device, {}", GetLastError()); return; }

	if (!HidD_GetProductString(ntHandle, name.Data(), (UL32)name.Capacity())) { Logger::Trace("Failed to get name, {}", GetLastError()); }
	name.Resize();

	if (!HidD_GetPreparsedData(ntHandle, &preparsedData)) { Logger::Trace("Failed to get data, {}, skipping...", GetLastError()); Destroy(); return; }

	if (HidP_GetCaps(preparsedData, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

	if (info.dwType == RIM_TYPEMOUSE && (capabilities.Usage == HID_USAGE_GENERIC_MOUSE || capabilities.Usage == HID_USAGE_GENERIC_POINTER))
	{
		if (!SetupMouse()) { Destroy(); return; }
	}
	else if (info.dwType == RIM_TYPEKEYBOARD && (capabilities.Usage == HID_USAGE_GENERIC_KEYBOARD || capabilities.Usage == HID_USAGE_GENERIC_KEYPAD))
	{
		if (!SetupKeyboard()) { Destroy(); return; }
	}
	else if (info.dwType == RIM_TYPEHID && (capabilities.Usage == HID_USAGE_GENERIC_GAMEPAD || capabilities.Usage == HID_USAGE_GENERIC_JOYSTICK || capabilities.Usage == HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER))
	{
		if (!SetupController()) { Destroy(); return; }
	}
	else
	{
		//TODO: May not want to discard
		Logger::Trace("Unkown device type, skipping...");
		Destroy();
		return;
	}

	valid = true;

	Memory::AllocateArray(&reportBuffer, (const U16)capabilities.InputReportByteLength);

	//TODO: we may want HIDs that don't have input
	if (!(stateLength = HidP_MaxDataListLength(HidP_Input, preparsedData))) { Logger::Trace("Device has no capabilities, skipping..."); Destroy(); return; }

	Memory::AllocateArray(&stateBuffer, (const U16)stateLength);

	Vector<HIDP_BUTTON_CAPS> buttonClasses(capabilities.NumberInputButtonCaps, {});
	if (HidP_GetButtonCaps(HidP_Input, buttonClasses.Data(), &capabilities.NumberInputButtonCaps, preparsedData) != HIDP_STATUS_SUCCESS) { Logger::Trace("No Buttons"); }

	Vector<HIDP_VALUE_CAPS> axisClasses(capabilities.NumberInputValueCaps, {});
	if (HidP_GetValueCaps(HidP_Input, axisClasses.Data(), &capabilities.NumberInputValueCaps, preparsedData) != HIDP_STATUS_SUCCESS) { Logger::Trace("No Axes"); }

	U64 numberOfButtons = 0;
	for (HIDP_BUTTON_CAPS& currentClass : buttonClasses)
	{
		if (currentClass.IsRange) { numberOfButtons += currentClass.Range.UsageMax - currentClass.Range.UsageMin + 1; }
		else
		{
			currentClass.Range.UsageMin = currentClass.Range.UsageMax = currentClass.NotRange.Usage;
			currentClass.Range.DataIndexMin = currentClass.Range.DataIndexMax = currentClass.NotRange.DataIndex;
			currentClass.IsRange = 1;
			++numberOfButtons;
		}
	}

	U64 numberOfAxes = 0;
	for (HIDP_VALUE_CAPS& currentClass : axisClasses)
	{
		if (currentClass.IsRange) { numberOfAxes += currentClass.Range.UsageMax - currentClass.Range.UsageMin + 1; }
		else
		{
			currentClass.Range.UsageMin = currentClass.Range.UsageMax = currentClass.NotRange.Usage;
			currentClass.Range.DataIndexMin = currentClass.Range.DataIndexMax = currentClass.NotRange.DataIndex;
			currentClass.IsRange = 1;
			++numberOfAxes;
		}
	}

	Vector<HIDButtonMapping> buttonMappings(numberOfButtons, {});
	Vector<HIDAxisMapping> axisMappings(numberOfAxes, {});
}

Device::Device(Device&& other) noexcept : riHandle{ other.riHandle }, ntHandle{ other.ntHandle }, type{ other.type }, capabilities{ other.capabilities }, preparsedData{ other.preparsedData },
preparsedDataSize{ other.preparsedDataSize }, stateBuffer{ other.stateBuffer }, stateLength{ other.stateLength }, reportBuffer{ other.reportBuffer }, valid{ other.valid }
{
	other.ntHandle = nullptr;
	other.preparsedData = nullptr;
	other.stateBuffer = nullptr;
	other.reportBuffer = nullptr;
	other.valid = false;
}

Device& Device::operator=(Device&& other) noexcept
{
	riHandle = other.riHandle;
	ntHandle = other.ntHandle;
	type = other.type;
	capabilities = other.capabilities;
	preparsedData = other.preparsedData;
	preparsedDataSize = other.preparsedDataSize;
	stateBuffer = other.stateBuffer;
	stateLength = other.stateLength;
	reportBuffer = other.reportBuffer;
	valid = other.valid;

	other.ntHandle = nullptr;
	other.preparsedData = nullptr;
	other.stateBuffer = nullptr;
	other.reportBuffer = nullptr;
	other.valid = false;

	return *this;
}

Device::~Device()
{
	Destroy();
}

void Device::Destroy()
{
	CloseHandle(ntHandle);
	ntHandle = nullptr;

	valid = false;

	if (preparsedData)
	{
		HidD_FreePreparsedData(preparsedData);
		preparsedData = nullptr;
	}

	if (stateBuffer) { Memory::FreeArray(&stateBuffer); }

	if (reportBuffer) { Memory::FreeArray(&reportBuffer); }
}

void Device::Update()
{

}

bool Device::SetupMouse()
{
	type = DEVICE_TYPE_MOUSE;

	Logger::Debug("Mouse detected");

	return true;
}

bool Device::SetupKeyboard()
{
	type = DEVICE_TYPE_KEYBOARD;

	Logger::Debug("Keyboard detected");

	return true;
}

bool Device::SetupController()
{
	type = DEVICE_TYPE_CONTROLLER;

	Logger::Debug("Controller detected");

	//HKEY_CURRENT_USER: "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_{h}&PID_{h}\\Axes\\"
	//HKEY_CURRENT_USER: "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_{h}&PID_{h}\\Buttons\\"
	//HKEY_CURRENT_USER: "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\DirectInput\\VID_{h}&PID_{h}\\Calibration\\0\\Type\\Axes\\"

	return true;
}

#endif