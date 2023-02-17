#include "Input.hpp"

#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"

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

			String manufacturer;
			if (!HidD_GetManufacturerString(ntHandle, manufacturer.Data(), manufacturer.Capacity())) { BreakPoint; }

			String product;
			if (!HidD_GetProductString(ntHandle, product.Data(), product.Capacity())) { BreakPoint; }

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