#include "Input.hpp"

#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"
#include "Containers\WString.hpp"

#if defined PLATFORM_WINDOWS

#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

bool Input::Initialize()
{
	Logger::Trace("Initializing Input...");

	U32 expectedNumberOfHIDs = 0;
	if (GetRawInputDeviceList(nullptr, &expectedNumberOfHIDs, sizeof(RAWINPUTDEVICELIST)))
	{
		//TODO: Error, no device or no support for raw input
		return false;
	}

	RAWINPUTDEVICELIST arr[100];
	if (GetRawInputDeviceList(arr, &expectedNumberOfHIDs, sizeof(RAWINPUTDEVICELIST)) != expectedNumberOfHIDs)
	{
		//TODO: Error
		return false;
	}

	for (U32 i = 0; i < expectedNumberOfHIDs; ++i)
	{
		RAWINPUTDEVICELIST hidDescriptor = arr[i];
		switch (hidDescriptor.dwType)
		{
		case RIM_TYPEMOUSE: {//TODO:


		} break;
		case RIM_TYPEKEYBOARD: { //TODO:



		} break;
		case RIM_TYPEHID: {//TODO:

			String path;
			U32 len = (U32)path.Capacity();
			if (!(len = GetRawInputDeviceInfoA(hidDescriptor.hDevice, RIDI_DEVICENAME, path.Data(), &len))) { BreakPoint; }
			path.Resize(len);

			HANDLE ntHandle;
			if ((ntHandle = CreateFileA(path.Data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr)) == INVALID_HANDLE_VALUE) { BreakPoint; }

			WString manufacturer;
			if (!HidD_GetManufacturerString(ntHandle, manufacturer.Data(), manufacturer.Capacity())) { BreakPoint; }
			manufacturer.Resize();

			WString product;
			if (!HidD_GetProductString(ntHandle, product.Data(), product.Capacity())) { BreakPoint; }
			product.Resize();

			PHIDP_PREPARSED_DATA inputReportProtocol;
			if (!HidD_GetPreparsedData(ntHandle, &inputReportProtocol)) { BreakPoint; }

			HIDP_CAPS capabilities;
			if (HidP_GetCaps(inputReportProtocol, &capabilities) != HIDP_STATUS_SUCCESS) { BreakPoint; }

			if (capabilities.UsagePage != HID_USAGE_PAGE_GENERIC || (capabilities.Usage != HID_USAGE_GENERIC_GAMEPAD && capabilities.Usage != HID_USAGE_GENERIC_JOYSTICK))
			{
				//TODO: Don't care about this, scrap
				BreakPoint;
			}

			U64 sizeOfInputReport = sizeof(RAWINPUTHEADER) + sizeof(RAWHID) + capabilities.InputReportByteLength;

			Vector<HIDP_BUTTON_CAPS> buttonClasses(capabilities.NumberInputButtonCaps, {});
			if (HidP_GetButtonCaps(HidP_Input, buttonClasses.Data(), &capabilities.NumberInputButtonCaps, inputReportProtocol) != HIDP_STATUS_SUCCESS) { BreakPoint; }

			Vector<HIDP_VALUE_CAPS> axisClasses(capabilities.NumberInputValueCaps, {});
			if (HidP_GetValueCaps(HidP_Input, axisClasses.Data(), &capabilities.NumberInputValueCaps, inputReportProtocol) != HIDP_STATUS_SUCCESS) { BreakPoint; }

			BreakPoint;
		} break;
		default: break;
		}
	}

	return true;
}

void Input::Shutdown()
{

}

void Input::Update()
{

}

#endif