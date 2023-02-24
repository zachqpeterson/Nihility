#include "Device.hpp"

#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

Device::Device(void* handle, DeviceType type) : dHandle{ handle }, ntHandle{ nullptr }, preparsedData{ nullptr }, stateBuffer{ nullptr }, reportBuffer{ nullptr }
{
	LoadDevice(type);
}

Device::~Device()
{
	Destroy();
}

void Device::Destroy()
{
	if (preparsedData) { HidD_FreePreparsedData(preparsedData); }
	if (openHandle) { CloseHandle(ntHandle); openHandle = false; }
}

void Device::LoadDevice(DeviceType type)
{
	String path;
	U32 len = (U32)path.Capacity();
	if ((len = GetRawInputDeviceInfoA(dHandle, RIDI_DEVICENAME, path.Data(), &len)) == U32_MAX) { Logger::Trace("Failed to get path, skipping..."); return; }
	path.Resize(len);

	if ((ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)) == INVALID_HANDLE_VALUE) { Logger::Trace("Failed to open handle, skipping..."); return; }
	openHandle = true;

	if (!HidD_GetProductString(ntHandle, product.Data(), product.Capacity())) { Logger::Trace("Failed to get product, {}", GetLastError()); }
	product.Resize();

	if (!HidD_GetManufacturerString(ntHandle, manufacturer.Data(), manufacturer.Capacity())) { Logger::Trace("Failed to get manufacturer, {}", GetLastError()); }
	manufacturer.Resize();

	if (!HidD_GetPreparsedData(ntHandle, &preparsedData)) { Logger::Trace("Failed to get data, {}, skipping...", GetLastError()); Destroy(); return; }

	if (HidP_GetCaps(preparsedData, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

	if (!(stateLength = HidP_MaxDataListLength(HidP_Input, preparsedData))) { Logger::Trace("Failed to get report size, skipping..."); Destroy(); return; }

	switch (type)
	{
	case DEVICE_TYPE_MOUSE: {
		if (capabilities.UsagePage != HID_USAGE_PAGE_GENERIC || (capabilities.Usage != HID_USAGE_GENERIC_MOUSE && capabilities.Usage != HID_USAGE_GENERIC_POINTER)) { Logger::Trace("Not a mouse, skipping..."); Destroy(); return; }
	} break;
	case DEVICE_TYPE_KEYBOARD: {
		if (capabilities.UsagePage != HID_USAGE_PAGE_GENERIC || (capabilities.Usage != HID_USAGE_GENERIC_KEYBOARD && capabilities.Usage != HID_USAGE_GENERIC_KEYPAD)) { Logger::Trace("Not a keyboard, skipping..."); Destroy(); return; }
	} break;
	case DEVICE_TYPE_CONTROLLER: {
		if (capabilities.UsagePage != HID_USAGE_PAGE_GENERIC || (capabilities.Usage != HID_USAGE_GENERIC_GAMEPAD && capabilities.Usage != HID_USAGE_GENERIC_JOYSTICK)) { Logger::Trace("Not a controller, skipping..."); Destroy(); return; }
	} break;
	}

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

void Device::UnloadDevice()
{

}

#endif