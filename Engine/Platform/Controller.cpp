#include "Controller.hpp"
#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

Controller::Controller(void* handle) : dHandle{ handle }, inputReportProtocol{ nullptr }
{
	U32 len = (U32)path.Capacity();
	if (!(len = GetRawInputDeviceInfoA(dHandle, RIDI_DEVICENAME, path.Data(), &len))) { Logger::Error("Failed to get path"); return; }
	path.Resize(len);

	if ((ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)) == INVALID_HANDLE_VALUE) { return; }
	openHandle = true;

	if (HidD_GetProductString(ntHandle, product.Data(), product.Capacity())) { product.Resize(); }

	if (HidD_GetManufacturerString(ntHandle, manufacturer.Data(), manufacturer.Capacity())) { manufacturer.Resize(); }

	if (!HidD_GetPreparsedData(ntHandle, (PHIDP_PREPARSED_DATA*)&inputReportProtocol)) { Destroy(); Logger::Error("Failed to get data"); return; }

	if (HidP_GetCaps((PHIDP_PREPARSED_DATA)inputReportProtocol, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Destroy(); return; }

	if (capabilities.UsagePage != HID_USAGE_PAGE_GENERIC || (capabilities.Usage != HID_USAGE_GENERIC_GAMEPAD && capabilities.Usage != HID_USAGE_GENERIC_JOYSTICK)) { Destroy(); return; }

	U64 sizeOfInputReport = sizeof(RAWINPUTHEADER) + sizeof(RAWHID) + capabilities.InputReportByteLength;

	Vector<HIDP_BUTTON_CAPS> buttonClasses(capabilities.NumberInputButtonCaps, {});
	if (HidP_GetButtonCaps(HidP_Input, buttonClasses.Data(), &capabilities.NumberInputButtonCaps, (PHIDP_PREPARSED_DATA)inputReportProtocol) != HIDP_STATUS_SUCCESS) { Destroy(); return; }

	Vector<HIDP_VALUE_CAPS> axisClasses(capabilities.NumberInputValueCaps, {});
	if (HidP_GetValueCaps(HidP_Input, axisClasses.Data(), &capabilities.NumberInputValueCaps, (PHIDP_PREPARSED_DATA)inputReportProtocol) != HIDP_STATUS_SUCCESS) { Destroy(); return; }

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

	size_t numberOfAxes = 0;
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

	static constexpr U32 AXIS_MAPPING_COUNT = 7;
	struct MappingAndCalibration
	{
		U16 usagePage;
		U16 usage;
		bool isCalibrated;
		Calibration calibration;
		U8 name[32];
	} dInputAxisMapping[AXIS_MAPPING_COUNT]{};

	for (const HIDP_VALUE_CAPS& currentClass : axisClasses)
	{
		if (currentClass.UsagePage != HID_USAGE_PAGE_GENERIC) { continue; }

		const U16 firstUsage = currentClass.Range.UsageMin;
		const U16 lastUsage = currentClass.Range.UsageMax;
		for (U16 currentUsage = firstUsage; currentUsage <= lastUsage; ++currentUsage)
		{
			const U32 index = (U32)(currentUsage - HID_USAGE_GENERIC_X);
			if (index < 7)
			{
				dInputAxisMapping[index].usagePage = HID_USAGE_PAGE_GENERIC;
				dInputAxisMapping[index].usagePage = currentUsage;
			}
		}
	}

	if (!dInputAxisMapping[2].usagePage) //Map slider to Z
	{
		dInputAxisMapping[2] = dInputAxisMapping[6];
		dInputAxisMapping[6].usagePage = dInputAxisMapping[6].usage = 0;
	}

	static constexpr U32 BUTTON_MAPPING_COUNT = 128;
	struct Mapping
	{
		U16 usagePage;
		U16 usage;
		U8 name[32];
	} dInputButtonMapping[BUTTON_MAPPING_COUNT]{};

	HIDD_ATTRIBUTES vendorAndProductID;
	if (!HidD_GetAttributes(ntHandle, &vendorAndProductID)) { Destroy(); return; }

	WString path(L"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_{}&PID_{}\\Axes\\?",
		WString(vendorAndProductID.VendorID, WHEX), WString(vendorAndProductID.ProductID, WHEX));

	BreakPoint;
}

Controller::~Controller()
{
	Destroy();
}

void Controller::Destroy()
{
	if (inputReportProtocol) { HidD_FreePreparsedData((PHIDP_PREPARSED_DATA)inputReportProtocol); }
	if (openHandle) { CloseHandle(ntHandle); openHandle = false; }
}

#endif