#include "Controller.hpp"
#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

Controller::Controller(void* handle) : dHandle{ handle }, ntHandle{nullptr}, openHandle{ false }, inputReportProtocol{ nullptr }, capabilities{}
{
	String path;
	U32 len = (U32)path.Capacity();
	if ((len = GetRawInputDeviceInfoA(dHandle, RIDI_DEVICENAME, path.Data(), &len)) == U32_MAX) { Logger::Trace("Failed to get path, skipping..."); return; }
	path.Resize(len);

	if ((ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)) == INVALID_HANDLE_VALUE) { Logger::Trace("Failed to open handle, skipping..."); return; }
	openHandle = true;

	if (HidD_GetProductString(ntHandle, product.Data(), product.Capacity())) { product.Resize(); }

	if (HidD_GetManufacturerString(ntHandle, manufacturer.Data(), manufacturer.Capacity())) { manufacturer.Resize(); }

	if (!HidD_GetPreparsedData(ntHandle, &inputReportProtocol)) { Logger::Trace("Failed to get data, skipping..."); Destroy(); return; }

	if (HidP_GetCaps(inputReportProtocol, (PHIDP_CAPS)&capabilities) != HIDP_STATUS_SUCCESS) { Logger::Trace("Failed to get capabilities, skipping..."); Destroy(); return; }

	if(!(inputReportSize = HidP_MaxDataListLength(HidP_Input, inputReportProtocol))) { Logger::Trace("Failed to get report size, skipping..."); Destroy(); return; }

	stateBuffer = (PHIDP_DATA)Memory::Allocate(inputReportSize * sizeof(_HIDP_DATA));

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

	static constexpr U32 BUTTON_MAPPING_COUNT = 128;
	Vector<HIDButtonMapping> buttonMappings(BUTTON_MAPPING_COUNT, {});

	HIDD_ATTRIBUTES vendorAndProductID;
	if (!HidD_GetAttributes(ntHandle, &vendorAndProductID)) { Destroy(); return; }

	//TODO: {h} to automatically format hex
	String axisPath("System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\VID_{}&PID_{}\\Axes\\",
		String(vendorAndProductID.VendorID, HEX), String(vendorAndProductID.ProductID, HEX));

	for (U64 i = 0; i < AXIS_MAPPING_COUNT; ++i)
	{
		axisPath.Overwrite(i, 103);

		HKEY key = nullptr;
		if (RegOpenKeyExA(HKEY_CURRENT_USER, axisPath.Data(), 0, KEY_READ, &key)) { continue; }

		UL32 valueType = REG_NONE;
		UL32 valueSize = 0;
		RegQueryValueExA(key, "", nullptr, &valueType, nullptr, &valueSize);
		if (valueType && sizeof(axisMappings[i].name) > valueSize == REG_SZ)
		{
			RegQueryValueExA(key, "", nullptr, &valueType, (U8*)(axisMappings[i].name), &valueSize);
		}

		HIDAttributes mapping;
		valueType = REG_NONE;
		valueSize = 0;
		RegQueryValueExA(key, "Attributes", nullptr, &valueType, nullptr, &valueSize);
		if (REG_BINARY == valueType && valueSize == sizeof(HIDAttributes))
		{
			RegQueryValueExA(key, "Attributes", nullptr, &valueType, (U8*)(&mapping), &valueSize);
			if (0x15 > mapping.wUsagePage)
			{
				axisMappings[i].usagePage = mapping.wUsagePage;
				axisMappings[i].usage = mapping.wUsage;
			}
		}

		RegCloseKey(key);
	}

	axisPath.Overwrite("Buttons\\", 98);
	for (U64 i = 0; i < BUTTON_MAPPING_COUNT; ++i)
	{
		axisPath.Overwrite(i, 106);

		HKEY key = nullptr;
		if (0 != RegOpenKeyExA(HKEY_CURRENT_USER, axisPath.Data(), 0, KEY_READ, &key)) { continue; }

		UL32 valueType = REG_NONE;
		UL32 valueSize = 0;
		RegQueryValueExA(key, "", nullptr, &valueType, nullptr, &valueSize);
		if (REG_SZ == valueType && sizeof(buttonMappings[i].name) > valueSize)
		{
			RegQueryValueExA(key, "", nullptr, &valueType, (U8*)buttonMappings[i].name, &valueSize);
		}

		HIDAttributes mapping;
		valueType = REG_NONE;
		valueSize = 0;
		RegQueryValueExA(key, "Attributes", nullptr, &valueType, nullptr, &valueSize);
		if (REG_BINARY == valueType && sizeof(HIDAttributes) == valueSize)
		{
			RegQueryValueExA(key, "Attributes", nullptr, &valueType, (U8*)(&mapping), &valueSize);
			if (0x15 > mapping.wUsagePage)
			{
				buttonMappings[i].usagePage = mapping.wUsagePage;
				buttonMappings[i].usage = mapping.wUsage;
			}
		}

		RegCloseKey(key);
	}

	String calibrationPath("System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\DirectInput\\VID_{}&PID_{}\\Calibration\\0\\Type\\Axes\\",
		String(vendorAndProductID.VendorID, HEX), String(vendorAndProductID.ProductID, HEX));

	for (size_t i = 0; i < AXIS_MAPPING_COUNT; ++i)
	{
		calibrationPath.Overwrite(i, 121);

		U8 success = false;

		HKEY key = nullptr;
		if (!RegOpenKeyExA(HKEY_CURRENT_USER, calibrationPath.Data(), 0, KEY_READ, &key))
		{
			HIDCalibration& calibration = axisMappings[i].calibration;
			UL32 valueType = REG_NONE;
			UL32 valueSize = 0;
			RegQueryValueExA(key, "Calibration", nullptr, &valueType, nullptr, &valueSize);
			if (valueType == REG_BINARY && valueSize == sizeof(HIDCalibration))
			{
				if (!RegQueryValueExA(key, "Calibration", nullptr, &valueType, (U8*)(&calibration), &valueSize))
				{
					axisMappings[i].isCalibrated = true;
				}
			}

			RegCloseKey(key);
		}
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
			char* toName = nullptr;

			for (HIDAxisMapping* it = axisMappings.begin(), *end = axisMappings.end(); it != end; ++it)
			{
				HIDAxisMapping mapping = *it;

				if (currentClass.UsagePage == mapping.usagePage && currentUsage == mapping.usage)
				{
					toName = (char*)mapping.name;
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
			char* toName = nullptr;
			for (HIDButtonMapping& mapping : buttonMappings)
			{
				if (mapping.usagePage == currentClass.UsagePage && mapping.usage == currentUsage)
				{
					toName = (char*)mapping.name;

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

void Controller::Update()
{
	UL32 count = 0;
	HidP_GetData(HidP_Input, nullptr, &count, inputReportProtocol, nullptr, capabilities.InputReportByteLength);
	HidP_GetData(HidP_Input, stateBuffer, &count, inputReportProtocol, nullptr, capabilities.InputReportByteLength);
}

#endif