#include "Device.hpp"

#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

Device::Device(WString path) : path{ path }, ntHandle{ nullptr }, type{ DEVICE_TYPE_COUNT }, capabilities{},
preparsedData{ nullptr }, preparsedDataSize{ 0 }, stateBuffer{ nullptr }, stateLength{ 0 }, reportBuffer{ nullptr }, openHandle{ false }
{
	if ((ntHandle = CreateFileW(path.Data(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr)) == INVALID_HANDLE_VALUE) { Logger::Trace("Failed to open handle, skipping..."); return; }
	openHandle = true;

	if (!HidD_GetProductString(ntHandle, product.Data(), (UL32)product.Capacity())) { Logger::Trace("Failed to get product, {}", GetLastError()); }
	product.Resize();

	if (!HidD_GetManufacturerString(ntHandle, manufacturer.Data(), (UL32)manufacturer.Capacity())) { Logger::Trace("Failed to get manufacturer, {}", GetLastError()); }
	manufacturer.Resize();

	if (!HidD_GetPreparsedData(ntHandle, &preparsedData)) { Logger::Trace("Failed to get data, {}, skipping...", GetLastError()); Destroy(); return; }

	if (HidP_GetCaps(preparsedData, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

	reportBuffer = (char*)Memory::Allocate(capabilities.InputReportByteLength);

	//TODO: we may want HIDs that don't have input
	if (!(stateLength = HidP_MaxDataListLength(HidP_Input, preparsedData))) { Logger::Trace("Device has no capabilities, skipping..."); Destroy(); return; }

	stateBuffer = (PHIDP_DATA)Memory::Allocate(stateLength * sizeof(HIDP_DATA));

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
		Logger::Trace("Unkown device type, skipping...");
		Destroy();
		return;
	}

	//KEYBOARD LAYOUT: "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Keyboard Layout"
}

Device::Device(Device&& other) noexcept : path{ Move(other.path) }, ntHandle{ other.ntHandle }, type{ other.type }, capabilities{ other.capabilities }, preparsedData{ other.preparsedData },
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
	path = Move(other.path);
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
		Memory::Free(reportBuffer);
		reportBuffer = nullptr;
	}
}

void Device::Update()
{
	static OVERLAPPED overlap{};
	UL32 error;
	UL32 read;

	while (overlap.Internal != ERROR_NO_MORE_ITEMS)
	{
		if (!ReadFile(ntHandle, reportBuffer, capabilities.InputReportByteLength, &read, &overlap) && (error = GetLastError()) != ERROR_IO_PENDING)
		{
			Logger::Error("Failed to get data, {}!", GetLastError());
			BreakPoint;
			break;
		}


	}

	if (overlap.Internal == 0 && ! && )
	{
		
	}
	else if (overlap.Internal && overlap.Internal != STATUS_PENDING)
	{
		NTSTATUS s;
		if (s = HidP_GetData(HidP_Input, stateBuffer, &stateLength, preparsedData, reportBuffer, capabilities.InputReportByteLength) != HIDP_STATUS_SUCCESS)
		{
			Logger::Error("Failed to get data, {}!", s);
			BreakPoint;
			return;
		}

		if (!ReadFile(ntHandle, reportBuffer, capabilities.InputReportByteLength, &read, &overlap) && (error = GetLastError()) != ERROR_IO_PENDING)
		{
			Logger::Error("Failed to get data, {}!", GetLastError());
			BreakPoint;
			return;
		}
	}
}

bool Device::SetupMouse()
{
	type = DEVICE_TYPE_MOUSE;

	Logger::Trace("Mouse detected");

	return true;
}

bool Device::SetupKeyboard()
{
	type = DEVICE_TYPE_KEYBOARD;

	Logger::Trace("Keyboard detected");

	return true;
}

bool Device::SetupController()
{
	type = DEVICE_TYPE_CONTROLLER;

	Logger::Trace("Controller detected");

	return true;
}

#endif