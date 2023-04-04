#include "Input.hpp"

#include "Device.hpp"
#include "Platform.hpp"
#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"
#include "Resources\Settings.hpp"

Vector<Device> Input::devices;
I16 Input::mouseWheelDelta;
I16 Input::mouseHWheelDelta;
I32 Input::mousePosX;
I32 Input::mousePosY;
I32 Input::deltaMousePosX;
I32 Input::deltaMousePosY;
bool Input::scrollFocus;

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#pragma comment(lib ,"setupapi.lib")

bool Input::Initialize()
{
	Logger::Trace("Initializing Input...");

	const WindowData& wd = Platform::GetWindowData();

	RAWINPUTDEVICE rid[7];

	rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[0].usUsage = HID_USAGE_GENERIC_POINTER;
	rid[0].dwFlags = RIDEV_DEVNOTIFY;
	rid[0].hwndTarget = nullptr;

	rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
	rid[1].dwFlags = RIDEV_DEVNOTIFY | RIDEV_NOLEGACY | RIDEV_INPUTSINK;
	rid[1].hwndTarget = wd.window;

	rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[2].usUsage = HID_USAGE_GENERIC_KEYPAD;
	rid[2].dwFlags = RIDEV_DEVNOTIFY;
	rid[2].hwndTarget = nullptr;

	rid[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	rid[3].dwFlags = RIDEV_DEVNOTIFY | RIDEV_NOLEGACY;
	rid[3].hwndTarget = nullptr;

	rid[4].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[4].usUsage = HID_USAGE_GENERIC_JOYSTICK;
	rid[4].dwFlags = RIDEV_DEVNOTIFY | RIDEV_REMOVE;
	rid[4].hwndTarget = nullptr;

	rid[5].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[5].usUsage = HID_USAGE_GENERIC_GAMEPAD;
	rid[5].dwFlags = RIDEV_DEVNOTIFY | RIDEV_REMOVE;
	rid[5].hwndTarget = nullptr;

	rid[6].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid[6].usUsage = HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER;
	rid[6].dwFlags = RIDEV_DEVNOTIFY | RIDEV_REMOVE;
	rid[6].hwndTarget = nullptr;

	if (!RegisterRawInputDevices(rid, 7, sizeof(RAWINPUTDEVICE))) { Logger::Error("Failed to register devices, {}!", GetLastError()); return false; }

	POINT p;
	GetCursorPos(&p);

	mousePosX = p.x;
	mousePosY = p.y;

	//U32 deviceCount = 0;
	//RAWINPUTDEVICELIST* deviceList = nullptr;
	//
	//if (GetRawInputDeviceList(nullptr, &deviceCount, sizeof(RAWINPUTDEVICELIST))) { return false; }
	//if (deviceCount == 0) { return true; }
	//
	//Memory::AllocateArray(&deviceList, deviceCount);
	//
	//deviceCount = GetRawInputDeviceList(deviceList, &deviceCount, sizeof(RAWINPUTDEVICELIST));
	//
	//for (U32 i = 0; i < deviceCount; ++i) { AddDevice(deviceList[i].hDevice); }
	//
	//Memory::FreeArray(&deviceList);

	return true;
}

void Input::Shutdown()
{
	devices.Destroy();
}

void Input::Update()
{
	//TODO: Reset values
}

void Input::ReceiveInput(HRAWINPUT handle)
{
	RAWINPUT input{};
	U32 size = 0;
	if (GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0) { return; }
	if (GetRawInputData(handle, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) < 1) { return; }

	switch (input.header.dwType)
	{
	case RIM_TYPEMOUSE: {

		RAWMOUSE mouse = input.data.mouse;

		if (mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			bool isVirtualDesktop = mouse.usFlags & MOUSE_VIRTUAL_DESKTOP;

			I32 width = GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
			I32 height = GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);

			I32 absoluteX = I32((mouse.lLastX / 65535.0f) * width);
			I32 absoluteY = I32((mouse.lLastY / 65535.0f) * height);

			deltaMousePosX = absoluteX - mousePosX;
			deltaMousePosY = absoluteY - mousePosY;
			mousePosX = absoluteX;
			mousePosY = absoluteY;
		}
		else if (mouse.lLastX != 0 || mouse.lLastY != 0)
		{
			I32 relativeX = mouse.lLastX;
			I32 relativeY = mouse.lLastY;

			//TODO: clamp
			mousePosX += relativeX;
			mousePosY += relativeY;
			deltaMousePosX = relativeX;
			deltaMousePosY = relativeY;
		}

		if (mouse.usButtonFlags & RI_MOUSE_WHEEL) { mouseWheelDelta = (F32)(I16)mouse.usButtonData / WHEEL_DELTA; }
		if (mouse.usButtonFlags & RI_MOUSE_HWHEEL) { mouseHWheelDelta = (F32)(I16)mouse.usButtonData / WHEEL_DELTA; }

	} break;
	case RIM_TYPEKEYBOARD: {

	} break;
	case RIM_TYPEHID: {

	} break;
	}
}

void Input::InputSink(HRAWINPUT handle)
{
	RAWINPUT input{};
	U32 size = 0;
	if (GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0) { return; }
	if (GetRawInputData(handle, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) < 1) { return; }

	if (input.header.dwType == RIM_TYPEMOUSE && input.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
	{
		POINT p;
		GetCursorPos(&p);

		if (p.x >= Settings::WindowPositionX() && p.x <= Settings::WindowWidth() + Settings::WindowPositionX() && 
			p.y >= Settings::WindowPositionY() && p.y <= Settings::WindowHeight() + Settings::WindowPositionY())
		{
			SetFocus(Platform::GetWindowData().window);

			mousePosX = p.x;
			mousePosY = p.y;
		}
	}
}

void Input::AddDevice(void* handle)
{
	Device device(handle);
	if (device.valid) { devices.Push(Move(device)); } //TODO: better data structure, probably HashMap
}

void Input::RemoveDevice(void* handle)
{

}

#endif