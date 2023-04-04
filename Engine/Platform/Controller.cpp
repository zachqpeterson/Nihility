#include "Controller.hpp"
#include "Core\Logger.hpp"
#include "Resources\Settings.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

//HKEY_CURRENT_USER: "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_{h}&PID_{h}\\Axes\\"
//HKEY_CURRENT_USER: "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_{h}&PID_{h}\\Buttons\\"
//HKEY_CURRENT_USER: "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\DirectInput\\VID_{h}&PID_{h}\\Calibration\\0\\Type\\Axes\\"

/*
Controller::Controller(void* handle) : dHandle{ handle }, ntHandle{nullptr}, openHandle{ false }, inputReportProtocol{ nullptr }, capabilities{}
{
	String path;
	U32 len = (U32)path.Capacity();
	if ((len = GetRawInputDeviceInfoA(dHandle, RIDI_DEVICENAME, path.Data(), &len)) == U32_MAX) { Logger::Trace("Failed to get path, skipping..."); return; }
	path.Resize(len);

	if ((ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)) == INVALID_HANDLE_VALUE) { Logger::Trace("Failed to open handle, skipping..."); return; }
	openHandle = true;

	if (!HidD_GetProductString(ntHandle, product.Data(), (UL32)product.Capacity())) { Logger::Trace("Failed to get product, {}", GetLastError()); }
	product.Resize();

	if (!HidD_GetManufacturerString(ntHandle, manufacturer.Data(), (UL32)manufacturer.Capacity())) { Logger::Trace("Failed to get manufacturer, {}", GetLastError()); }
	manufacturer.Resize();

	if (!HidD_GetPreparsedData(ntHandle, &inputReportProtocol)) { Logger::Trace("Failed to get data, {}, skipping...", GetLastError()); Destroy(); return; }

	if (HidP_GetCaps(inputReportProtocol, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

	if(!(inputReportSize = HidP_MaxDataListLength(HidP_Input, inputReportProtocol))) { Logger::Trace("Failed to get report size, skipping..."); Destroy(); return; }

	//stateBuffer = (PHIDP_DATA)Memory::Allocate(inputReportSize * sizeof(_HIDP_DATA));

	if (capabilities.UsagePage != HID_USAGE_PAGE_GENERIC || (capabilities.Usage != HID_USAGE_GENERIC_GAMEPAD && capabilities.Usage != HID_USAGE_GENERIC_JOYSTICK)) { Logger::Trace("Not a controller, skipping..."); Destroy(); return; }

	//TODO: HidP_Output and HidP_Feature capabilities
	Vector<HIDP_BUTTON_CAPS> buttonClasses(capabilities.NumberInputButtonCaps, {});
	if (HidP_GetButtonCaps(HidP_Input, buttonClasses.Data(), &capabilities.NumberInputButtonCaps, inputReportProtocol) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get button capabilities, skipping..."); Destroy(); return; }

	Vector<HIDP_VALUE_CAPS> axisClasses(capabilities.NumberInputValueCaps, {});
	if (HidP_GetValueCaps(HidP_Input, axisClasses.Data(), &capabilities.NumberInputValueCaps, inputReportProtocol) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get axis capabilities, skipping..."); Destroy(); return; }

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

	static constexpr U32 AXIS_MAPPING_COUNT = 7;
	Vector<HIDAxisMapping> axisMappings(AXIS_MAPPING_COUNT, {});

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
				axisMappings[index].usagePage = HID_USAGE_PAGE_GENERIC;
				axisMappings[index].usagePage = currentUsage;
			}
		}
	}

	if (!axisMappings[2].usagePage) //Map slider to Z
	{
		axisMappings[2] = axisMappings[6];
		axisMappings[6].usagePage = axisMappings[6].usage = 0;
	}

	HIDD_ATTRIBUTES vendorAndProductID;
	if (!HidD_GetAttributes(ntHandle, &vendorAndProductID)) { Destroy(); return; }

	String axisPath("System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_{h}&PID_{h}\\Axes\\",
		vendorAndProductID.VendorID, vendorAndProductID.ProductID);

	for (U64 i = 0; i < AXIS_MAPPING_COUNT; ++i)
	{
		axisPath.Overwrite(i, 103);
		HIDAttributes mapping;

		Settings::GetRegistryValue(HKEY_CURRENT_USER, axisPath, "", (U8*)axisMappings[i].name.Data());
		Settings::GetRegistryValue(HKEY_CURRENT_USER, axisPath, "Attributes", (U8*)(&mapping));

		if (mapping.wUsagePage < 0x15)
		{
			axisMappings[i].usagePage = mapping.wUsagePage;
			axisMappings[i].usage = mapping.wUsage;
		}
	}

	static constexpr U32 BUTTON_MAPPING_COUNT = 128;
	Vector<HIDButtonMapping> buttonMappings(BUTTON_MAPPING_COUNT, {});

	axisPath.Overwrite("Buttons\\", 98);
	for (U64 i = 0; i < BUTTON_MAPPING_COUNT; ++i)
	{
		axisPath.Overwrite(i, 106);
		HIDAttributes mapping;

		Settings::GetRegistryValue(HKEY_CURRENT_USER, axisPath, "", (U8*)buttonMappings[i].name.Data());
		Settings::GetRegistryValue(HKEY_CURRENT_USER, axisPath, "Attributes", (U8*)(&mapping));

		if (mapping.wUsagePage < 0x15)
		{
			buttonMappings[i].usagePage = mapping.wUsagePage;
			buttonMappings[i].usage = mapping.wUsage;
		}
	}

	String calibrationPath("System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\DirectInput\\VID_{h}&PID_{h}\\Calibration\\0\\Type\\Axes\\",
		vendorAndProductID.VendorID, vendorAndProductID.ProductID);

	for (size_t i = 0; i < AXIS_MAPPING_COUNT; ++i)
	{
		calibrationPath.Overwrite(i, 121);

		axisMappings[i].isCalibrated = Settings::GetRegistryValue(HKEY_CURRENT_USER, calibrationPath, "Calibration", (U8*)(&axisMappings[i].calibration));
	}

	for (const HIDP_VALUE_CAPS& currentClass : axisClasses)
	{
		const U16 firstUsage = currentClass.Range.UsageMin;
		const U16 lastUsage = currentClass.Range.UsageMax;
		for (U16 currentUsage = firstUsage, currentIndex = currentClass.Range.DataIndexMin; currentUsage <= lastUsage; ++currentUsage, ++currentIndex)
		{
			bool isCalibrated = false;
			I32 calibratedMinimum;
			I32 calibratedMaximum;
			I32 calibratedCenter;
			C8* toName = nullptr;

			for (HIDAxisMapping* it = axisMappings.begin(), *end = axisMappings.end(); it != end; ++it)
			{
				HIDAxisMapping mapping = *it;

				if (currentClass.UsagePage == mapping.usagePage && currentUsage == mapping.usage)
				{
					//toName = (C8*)mapping.name;
					isCalibrated = mapping.isCalibrated;
					if (mapping.isCalibrated)
					{
						calibratedMinimum = mapping.calibration.lMin;
						calibratedCenter = mapping.calibration.lCenter;
						calibratedMaximum = mapping.calibration.lMax;
					}

					//TODO: Remove with iterator
					mapping.usage = 0; //TODO Optimization: skip on future iterations
					break;
				}
			}

			HIDAxis axis;
			axis.usagePage = currentClass.UsagePage;
			axis.usage = currentUsage;
			axis.index = currentIndex;
			axis.logicalMinimum = currentClass.LogicalMin;
			axis.logicalMaximum = currentClass.LogicalMax;
			axis.isCalibrated = isCalibrated;
			if (isCalibrated)
			{
				axis.logicalCalibratedMinimum = calibratedMinimum;
				axis.logicalCalibratedMaximum = calibratedMaximum;
				axis.logicalCalibratedCenter = calibratedCenter;
			}
			axis.physicalMinimum = F32(currentClass.PhysicalMin);
			axis.physicalMaximum = F32(currentClass.PhysicalMax);
			axis.name = toName;

			axes.Push(axis);
		}
	}

	for (const HIDP_BUTTON_CAPS& currentClass : buttonClasses)
	{
		const U16 firstUsage = currentClass.Range.UsageMin;
		const U16 lastUsage = currentClass.Range.UsageMax;
		for (U16 currentUsage = firstUsage, currentIndex = currentClass.Range.DataIndexMin; currentUsage <= lastUsage; ++currentUsage, ++currentIndex)
		{
			C8* toName = nullptr;
			for (HIDButtonMapping& mapping : buttonMappings)
			{
				if (mapping.usagePage == currentClass.UsagePage && mapping.usage == currentUsage)
				{
					//toName = (C8*)mapping.name;

					mapping.usage = 0; //TODO Optimization: skip on future iterations
					break;
				}
			}

			HIDButton button;
			button.usagePage = currentClass.UsagePage;
			button.usage = currentUsage;
			button.index = currentIndex;
			button.name = toName;

			buttons.Push(button);
		}
	}

	Logger::Debug(product);
	
	for (auto const& axis : axes)
	{
		Logger::Debug("axis {} / {}", axis.usagePage, axis.usage);

		if (!axis.name.Blank())
		{
			Logger::Debug(axis.name);
		}
		if (axis.isCalibrated)
		{
			Logger::Debug("calibrated by {} about", axis.logicalCalibratedMinimum);
		}
		else
		{
			Logger::Debug("not calibrated from {} to {}", axis.logicalMinimum, axis.logicalMaximum);
		}
	}
	
	for (auto const& button : buttons)
	{
		Logger::Debug("button {} / {}", button.usagePage, button.usage);
		if (!button.name.Blank())
		{
			Logger::Debug(button.name);
		}
	}
}
*/

Controller::~Controller()
{
	Destroy();
}

void Controller::Destroy()
{
	if (inputReportProtocol) { HidD_FreePreparsedData((PHIDP_PREPARSED_DATA)inputReportProtocol); }
	if (openHandle) 
	{ 
		CloseHandle(ntHandle);
		openHandle = false; 
	}
}

void Controller::Update()
{

}

#endif