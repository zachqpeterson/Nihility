#include "Keyboard.hpp"
#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

Keyboard::Keyboard(void* handle) : dHandle{ handle }, ntHandle{ nullptr }, openHandle{ false }, inputReportProtocol{ nullptr }, capabilities{}
{
	String path;
	U32 len = (U32)path.Capacity();
	if ((len = GetRawInputDeviceInfoA(dHandle, RIDI_DEVICENAME, path.Data(), &len)) == U32_MAX) { Logger::Trace("Failed to get path, skipping..."); return; }
	path.Resize(len);

	if ((ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)) == INVALID_HANDLE_VALUE) { Logger::Trace("Failed to open handle, skipping..."); return; }
	openHandle = true;

	if (HidD_GetProductString(ntHandle, product.Data(), product.Capacity())) { product.Resize(); }

	if (HidD_GetManufacturerString(ntHandle, manufacturer.Data(), manufacturer.Capacity())) { manufacturer.Resize(); }

	if (!HidD_GetPreparsedData(ntHandle, (PHIDP_PREPARSED_DATA*)&inputReportProtocol)) { Logger::Trace("Failed to get data, skipping..."); Destroy(); return; }

	if (HidP_GetCaps((PHIDP_PREPARSED_DATA)inputReportProtocol, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

	if (capabilities.UsagePage != HID_USAGE_PAGE_GENERIC || (capabilities.Usage != HID_USAGE_GENERIC_KEYBOARD && capabilities.Usage != HID_USAGE_GENERIC_KEYPAD)) { Logger::Trace("Not a keyboard, skipping..."); Destroy(); return; }

	Vector<HIDP_BUTTON_CAPS> buttonClasses(capabilities.NumberInputButtonCaps, {});
	if (HidP_GetButtonCaps(HidP_Input, buttonClasses.Data(), &capabilities.NumberInputButtonCaps, (PHIDP_PREPARSED_DATA)inputReportProtocol) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get button capabilities, skipping..."); Destroy(); return; }

	Vector<HIDP_VALUE_CAPS> axisClasses(capabilities.NumberInputValueCaps, {});
	HidP_GetValueCaps(HidP_Input, axisClasses.Data(), &capabilities.NumberInputValueCaps, (PHIDP_PREPARSED_DATA)inputReportProtocol);

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

	BreakPoint;
}

Keyboard::~Keyboard()
{
	Destroy();
}

void Keyboard::Destroy()
{
	if (inputReportProtocol) { HidD_FreePreparsedData((PHIDP_PREPARSED_DATA)inputReportProtocol); }
	if (openHandle) { CloseHandle(ntHandle); openHandle = false; }
}
#endif