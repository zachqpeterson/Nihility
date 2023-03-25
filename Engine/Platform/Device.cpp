#include "Device.hpp"

#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

Device::Device(void* handle) : ntHandle{ handle }, type{ DEVICE_TYPE_COUNT }, capabilities{},
preparsedData{ nullptr }, preparsedDataSize{ 0 }, stateBuffer{ nullptr }, stateLength{ 0 }, reportBuffer{ nullptr }, openHandle{ false }
{
	if (!HidD_GetProductString(ntHandle, product.Data(), (UL32)product.Capacity())) { Logger::Trace("Failed to get product, {}", GetLastError()); }
	product.Resize();

	if (!HidD_GetManufacturerString(ntHandle, manufacturer.Data(), (UL32)manufacturer.Capacity())) { Logger::Trace("Failed to get manufacturer, {}", GetLastError()); }
	manufacturer.Resize();

	if (!HidD_GetPreparsedData(ntHandle, &preparsedData)) { Logger::Trace("Failed to get data, {}, skipping...", GetLastError()); Destroy(); return; }

	if (HidP_GetCaps(preparsedData, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

	U64 cap = capabilities.InputReportByteLength;
	Memory::AllocateArray(&reportBuffer, cap);

	//TODO: we may want HIDs that don't have input
	if (!(stateLength = HidP_MaxDataListLength(HidP_Input, preparsedData))) { Logger::Trace("Device has no capabilities, skipping..."); Destroy(); return; }

	cap = stateLength * sizeof(HIDP_DATA);
	Memory::AllocateArray(&stateBuffer, cap);

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

	//KEYBOARD LAYOUT: "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Keyboard Layout"

	//if (!ReadFileEx(ntHandle, reportBuffer, capabilities.InputReportByteLength, (LPOVERLAPPED)&overlap, Device::DeviceRead))
	//{
	//	Logger::Error(GetLastError());
	//}
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

void __stdcall Device::DeviceRead(UL32 dwErrorCode, UL32 dwNumberOfBytesTransfered, _OVERLAPPED* lpOverlapped)
{
	BreakPoint;

	NTSTATUS s;
	if (s = HidP_GetData(HidP_Input, stateBuffer, &stateLength, preparsedData, reportBuffer, capabilities.InputReportByteLength) != HIDP_STATUS_SUCCESS)
	{
		Logger::Error("Failed to get data, {}!", s);
		BreakPoint;
		return;
	}

	//if (!ReadFileEx(ntHandle, reportBuffer, capabilities.InputReportByteLength, (LPOVERLAPPED)&overlap, Device::DeviceRead))
	//{
	//	Logger::Error(GetLastError());
	//}
}

void Device::Update()
{
	//TODO: Move to job system
	

	//NTSTATUS s;
	//if (s = HidP_GetData(HidP_Input, stateBuffer, &stateLength, preparsedData, reportBuffer, capabilities.InputReportByteLength) != HIDP_STATUS_SUCCESS)
	//{
	//	Logger::Error("Failed to get data, {}!", s);
	//	BreakPoint;
	//	return;
	//}

	//do
	//{
	//	if (read == capabilities.InputReportByteLength) //TODO: Check previous read
	//	{
	//		NTSTATUS s;
	//		if (s = HidP_GetData(HidP_Input, stateBuffer, &stateLength, preparsedData, reportBuffer, capabilities.InputReportByteLength) != HIDP_STATUS_SUCCESS)
	//		{
	//			Logger::Error("Failed to get data, {}!", s);
	//			BreakPoint;
	//			return;
	//		}
	//
	//		//TODO: Process data
	//		BreakPoint;
	//
	//		read = 0;
	//
	//		ReadFile(ntHandle, reportBuffer, capabilities.InputReportByteLength, &read, (LPOVERLAPPED)&overlap);
	//	}
	//	else
	//	{
	//		ReadFile(ntHandle, reportBuffer, capabilities.InputReportByteLength, &read, (LPOVERLAPPED)&overlap);
	//	}
	//
	//} while (overlap.Internal != ERROR_NO_MORE_ITEMS);
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

	return true;
}

#endif