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

Device::Device(void* handle) : ntHandle{ handle }, type{ DEVICE_TYPE_COUNT }, capabilities{},
preparsedData{ nullptr }, preparsedDataSize{ 0 }, stateBuffer{ nullptr }, stateLength{ 0 }, reportBuffer{ nullptr }, openHandle{ false }
{
	if (!HidD_GetProductString(ntHandle, name.Data(), (UL32)name.Capacity())) { Logger::Trace("Failed to get name, {}", GetLastError()); }
	name.Resize();

	if (!HidD_GetPreparsedData(ntHandle, &preparsedData)) { Logger::Trace("Failed to get data, {}, skipping...", GetLastError()); Destroy(); return; }

	if (HidP_GetCaps(preparsedData, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

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

	if (capabilities.Usage == HID_USAGE_GENERIC_MOUSE || capabilities.Usage == HID_USAGE_GENERIC_POINTER)
	{
		if (!SetupMouse()) { Destroy(); return; }
	}
	else if (capabilities.Usage == HID_USAGE_GENERIC_KEYBOARD || capabilities.Usage == HID_USAGE_GENERIC_KEYPAD)
	{
		if (!SetupKeyboard()) { Destroy(); return; }
	}
	else if (capabilities.Usage == HID_USAGE_GENERIC_GAMEPAD || capabilities.Usage == HID_USAGE_GENERIC_JOYSTICK || capabilities.Usage == HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER)
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
}

Device::Device(Device&& other) noexcept : ntHandle{ other.ntHandle }, type{ other.type }, capabilities{ other.capabilities }, preparsedData{ other.preparsedData },
preparsedDataSize{ other.preparsedDataSize }, stateBuffer{ other.stateBuffer }, stateLength{ other.stateLength }, reportBuffer{ other.reportBuffer }, openHandle{ other.openHandle }
{
	other.ntHandle = nullptr;
	other.preparsedData = nullptr;
	other.stateBuffer = nullptr;
	other.reportBuffer = nullptr;
	other.openHandle = false;
}

Device& Device::operator=(Device&& other) noexcept
{
	ntHandle = other.ntHandle;
	type = other.type;
	capabilities = other.capabilities;
	preparsedData = other.preparsedData;
	preparsedDataSize = other.preparsedDataSize;
	stateBuffer = other.stateBuffer;
	stateLength = other.stateLength;
	reportBuffer = other.reportBuffer;
	openHandle = other.openHandle;

	other.ntHandle = nullptr;
	other.preparsedData = nullptr;
	other.stateBuffer = nullptr;
	other.reportBuffer = nullptr;
	other.openHandle = false;

	return *this;
}

Device::~Device()
{
	Destroy();
}

void Device::Destroy()
{
	if (preparsedData)
	{
		HidD_FreePreparsedData(preparsedData);
		preparsedData = nullptr;
	}
	if (openHandle)
	{
		CloseHandle(ntHandle);
		ntHandle = nullptr;
		openHandle = false;
	}
	if (reportBuffer)
	{
		Memory::FreeArray(&reportBuffer);
		reportBuffer = nullptr;
	}
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