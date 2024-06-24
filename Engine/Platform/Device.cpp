#include "Device.hpp"

import Core;
import Memory;

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

	if ((ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr)) == INVALID_HANDLE_VALUE) { Logger::Trace("Failed to open device, {}", GetLastError()); return; }

	if (!HidD_GetProductString(ntHandle, name.Data(), (UL32)name.Capacity())) { Logger::Trace("Failed to get name, {}", GetLastError()); }
	name.Resize();

	if (!HidD_GetPreparsedData(ntHandle, &preparsedData)) { Logger::Trace("Failed to get data, {}, skipping...", GetLastError()); Destroy(); return; }

	if (HidP_GetCaps(preparsedData, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

	if (info.dwType == RIM_TYPEHID && (capabilities.Usage == HID_USAGE_GENERIC_GAMEPAD || capabilities.Usage == HID_USAGE_GENERIC_JOYSTICK || capabilities.Usage == HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER))
	{
		if (!SetupController()) { Destroy(); return; }
	}
	else
	{
		//TODO: May not want to discard
		Destroy();
		return;
	}

	valid = true;

	Memory::AllocateArray(&reportBuffer, (const U16)capabilities.InputReportByteLength);

	//TODO: we may want HIDs that don't have input
	if (!(stateLength = HidP_MaxDataListLength(HidP_Input, preparsedData))) { Logger::Trace("Device has no capabilities, skipping..."); Destroy(); return; }

	Memory::AllocateArray(&stateBuffer, (const U16)stateLength);

	async.event = CreateEventA(nullptr, false, false, nullptr);

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

Device::Device(Device&& other) noexcept : valid{ other.valid }, riHandle{ other.riHandle }, ntHandle{ other.ntHandle }, name{ Move(other.name) },
type{ other.type }, capabilities{ other.capabilities }, preparsedData{ other.preparsedData }, preparsedDataSize{ other.preparsedDataSize },
stateBuffer{ other.stateBuffer }, stateLength{ other.stateLength }, reportBuffer{ other.reportBuffer }, axes{ Move(other.axes) }, buttons{ Move(other.buttons) },
async{ other.async }, reading{ other.reading }
{
	other.ntHandle = nullptr;
	other.preparsedData = nullptr;
	other.stateBuffer = nullptr;
	other.reportBuffer = nullptr;
	other.valid = false;
}

Device& Device::operator=(Device&& other) noexcept
{
	valid = other.valid;
	riHandle = other.riHandle;
	ntHandle = other.ntHandle;
	name = Move(other.name);
	type = other.type;
	capabilities = other.capabilities;
	preparsedData = other.preparsedData;
	preparsedDataSize = other.preparsedDataSize;
	stateBuffer = other.stateBuffer;
	stateLength = other.stateLength;
	reportBuffer = other.reportBuffer;
	axes = Move(other.axes);
	buttons = Move(other.buttons);

	async = other.async;
	reading = other.reading;

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
	if (ntHandle) { CloseHandle(ntHandle); ntHandle = nullptr; }
	if (async.event) { CloseHandle(async.event); async.event = nullptr; }

	valid = false;

	if (preparsedData)
	{
		HidD_FreePreparsedData(preparsedData);
		preparsedData = nullptr;
	}

	if (stateBuffer) { Memory::Free(&stateBuffer); }
	if (reportBuffer) { Memory::Free(&reportBuffer); }

	name.Destroy();
	axes.Destroy();
	buttons.Destroy();
}

U8* Device::ReadInput(U32& size)
{
	bool overlapped = false;
	bool result;
	UL32 read;
	U8* buffer = nullptr;

	if (!reading)
	{
		reading = true;
		memset(reportBuffer, 0, capabilities.InputReportByteLength);
		ResetEvent(async.event);

		result = ReadFile(ntHandle, reportBuffer, capabilities.InputReportByteLength, &read, (LPOVERLAPPED)&async);

		if (!result)
		{
			UL32 err = GetLastError();
			if (err != ERROR_IO_PENDING)
			{
				CancelIo(ntHandle);
				reading = false;
				return buffer;
			}
			overlapped = true;
		}
	}
	else { overlapped = true; }

	if (overlapped)
	{
		if (WaitForSingleObject(async.event, 0) != WAIT_OBJECT_0) { return buffer; }

		result = GetOverlappedResult(ntHandle, (LPOVERLAPPED)&async, &read, true);
	}

	reading = false;

	if (result && read)
	{
		if (*reportBuffer == 0)
		{
			--read;
			buffer = reportBuffer + 1;
		}
		else { buffer = reportBuffer; }
	}

	size = read;

	return buffer;
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