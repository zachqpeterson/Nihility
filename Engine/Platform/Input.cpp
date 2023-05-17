#include "Input.hpp"

#include "Device.hpp"
#include "Platform.hpp"
#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"
#include "Resources\Settings.hpp"
#include "Math\Math.hpp"

Vector<Device> Input::devices;
I16 Input::mouseWheelDelta;
I16 Input::mouseHWheelDelta;
I32 Input::mousePosX;
I32 Input::mousePosY;
I32 Input::deltaMousePosX;
I32 Input::deltaMousePosY;

F32 Input::axisStates[AXIS_CODE_COUNT];
Input::ButtonState Input::buttonStates[BUTTON_CODE_COUNT];
bool Input::receiveInput;
bool Input::anyButtonDown;

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
	rid[3].dwFlags = RIDEV_DEVNOTIFY | RIDEV_NOLEGACY | RIDEV_APPKEYS;
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
	Logger::Trace("Cleaning Up Input...");

	devices.Destroy();
}

void Input::Update()
{
	//TODO: Reset values
	deltaMousePosX = 0;
	deltaMousePosY = 0;
	mouseWheelDelta = 0;
	mouseHWheelDelta = 0;

	anyButtonDown = false;
	receiveInput = true;
	for (ButtonState& state : buttonStates)
	{
		state.changed = false;
		state.doubleClicked = false;
		state.heldChanged = false;
	}
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

			//TODO: Sensitivity from windows
			//TODO: Pointer ballistics
			deltaMousePosX = relativeX;
			deltaMousePosY = relativeY;

			if (Settings::ConstrainCursor())
			{
				mousePosX = Math::Clamp(mousePosX += deltaMousePosX, Settings::WindowPositionX(), Settings::WindowWidth() + Settings::WindowPositionX());
				mousePosY = Math::Clamp(mousePosY += deltaMousePosY, Settings::WindowPositionY(), Settings::WindowHeight() + Settings::WindowPositionY());
			}
			else
			{
				mousePosX = Math::Clamp(mousePosX += deltaMousePosX, 0, Settings::ScreenWidth());
				mousePosY = Math::Clamp(mousePosY += deltaMousePosY, 0, Settings::ScreenHeight());
			}
		}

		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
		{
			buttonStates[BUTTON_CODE_LEFT_MOUSE].changed = true;
			buttonStates[BUTTON_CODE_LEFT_MOUSE].pressed = true;
			anyButtonDown = true;
		}
		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
		{
			buttonStates[BUTTON_CODE_LEFT_MOUSE].changed = true;
			buttonStates[BUTTON_CODE_LEFT_MOUSE].pressed = false;
		}
		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
		{
			buttonStates[BUTTON_CODE_RIGHT_MOUSE].changed = true;
			buttonStates[BUTTON_CODE_RIGHT_MOUSE].pressed = true;
			anyButtonDown = true;
		}
		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
		{
			buttonStates[BUTTON_CODE_RIGHT_MOUSE].changed = true;
			buttonStates[BUTTON_CODE_RIGHT_MOUSE].pressed = false;
		}
		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
		{
			buttonStates[BUTTON_CODE_MIDDLE_MOUSE].changed = true;
			buttonStates[BUTTON_CODE_MIDDLE_MOUSE].pressed = true;
			anyButtonDown = true;
		}
		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
		{
			buttonStates[BUTTON_CODE_MIDDLE_MOUSE].changed = true;
			buttonStates[BUTTON_CODE_MIDDLE_MOUSE].pressed = false;
		}
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
		{
			buttonStates[BUTTON_CODE_X_MOUSE_ONE].changed = true;
			buttonStates[BUTTON_CODE_X_MOUSE_ONE].pressed = true;
			anyButtonDown = true;
		}
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
		{
			buttonStates[BUTTON_CODE_X_MOUSE_ONE].changed = true;
			buttonStates[BUTTON_CODE_X_MOUSE_ONE].pressed = false;
		}
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
		{
			buttonStates[BUTTON_CODE_X_MOUSE_TWO].changed = true;
			buttonStates[BUTTON_CODE_X_MOUSE_TWO].pressed = true;
			anyButtonDown = true;
		}
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
		{
			buttonStates[BUTTON_CODE_X_MOUSE_TWO].changed = true;
			buttonStates[BUTTON_CODE_X_MOUSE_TWO].pressed = false;
		}

		if (mouse.usButtonFlags & RI_MOUSE_WHEEL) { mouseWheelDelta = (I16)((F32)(I16)mouse.usButtonData / WHEEL_DELTA); }
		if (mouse.usButtonFlags & RI_MOUSE_HWHEEL) { mouseHWheelDelta = (I16)((F32)(I16)mouse.usButtonData / WHEEL_DELTA); }

	} break;
	case RIM_TYPEKEYBOARD: {
		RAWKEYBOARD keyboard = input.data.keyboard;

		switch (keyboard.Message)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			ButtonState& state = buttonStates[keyboard.VKey];

			//if (HIWORD(lParam) & KF_EXTENDED)
			//{
			//	U8 code = (U8)LOWORD(MapVirtualKeyW(MAKEWORD(LOBYTE(HIWORD(lParam)), 0xE0), MAPVK_VSC_TO_VK_EX));
			//
			//	Input::buttonStates[code].heldChanged = Input::buttonStates[code].pressed && !Input::buttonStates[code].held;
			//	Input::buttonStates[code].held = Input::buttonStates[code].pressed;
			//	Input::buttonStates[code].changed = !Input::buttonStates[code].pressed;
			//	Input::buttonStates[code].pressed = true;
			//}

			state.heldChanged = state.pressed && !state.held;
			state.held = state.pressed;
			state.changed = !state.pressed;
			state.pressed = true;
			anyButtonDown |= state.changed;
		} break;

		case WM_KEYUP:
		case WM_SYSKEYUP: {
			ButtonState& state = buttonStates[keyboard.VKey];

			//if (HIWORD(lParam) & KF_EXTENDED)
			//{
			//	U8 code = (U8)LOWORD(MapVirtualKeyW(MAKEWORD(LOBYTE(HIWORD(lParam)), 0xE0), MAPVK_VSC_TO_VK_EX));
			//
			//	Input::buttonStates[code].changed = true;
			//	Input::buttonStates[code].pressed = false;
			//	Input::buttonStates[code].heldChanged = Input::buttonStates[code].held;
			//	Input::buttonStates[code].held = false;
			//}

			state.changed = true;
			state.pressed = false;
			state.heldChanged = state.held;
			state.held = false;
		} break;
		}
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

		HWND handle = WindowFromPoint(p);

		if (handle == Platform::GetWindowData().window)
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

bool Input::OnAnyButtonDown() { return anyButtonDown; }

bool Input::ButtonUp(ButtonCode code) { return !buttonStates[code].pressed && receiveInput; }

bool Input::ButtonDown(ButtonCode code) { return buttonStates[code].pressed && receiveInput; }

bool Input::ButtonHeld(ButtonCode code) { return buttonStates[code].held && receiveInput; }

bool Input::OnButtonUp(ButtonCode code) { return !buttonStates[code].pressed && buttonStates[code].changed && receiveInput; }

bool Input::OnButtonDown(ButtonCode code) { return buttonStates[code].pressed && buttonStates[code].changed && receiveInput; }

bool Input::OnButtonChange(ButtonCode code) { return buttonStates[code].changed && receiveInput; }

bool Input::OnButtonDoubleClick(ButtonCode code) { return buttonStates[code].doubleClicked && receiveInput; }

bool Input::OnButtonHold(ButtonCode code) { return buttonStates[code].held && buttonStates[code].heldChanged && receiveInput; }

bool Input::OnButtonRelease(ButtonCode code) { return !buttonStates[code].held && buttonStates[code].heldChanged && receiveInput; }

void Input::MousePos(U32& x, U32& y) { x = mousePosX; y = mousePosY; }

void Input::PreviousMousePos(U32& x, U32& y) { x = mousePosX - deltaMousePosX; y = mousePosY - deltaMousePosY; }

void Input::ConsumeInput() { receiveInput = false; }

I16 Input::MouseWheelDelta() { return mouseWheelDelta; }

I16 Input::MouseHWheelDelta() { return mouseHWheelDelta; }

#endif