#include "Input.hpp"

#include "Device.hpp"
#include "Platform.hpp"
#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Containers\String.hpp"
#include "Resources\Settings.hpp"
#include "Resources\ResourceDefines.hpp"
#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"

Vector<Device> Input::devices;
F32 Input::mouseSensitivity;
I16 Input::mouseWheelDelta;
I16 Input::mouseHWheelDelta;
F32 Input::mousePosX;
F32 Input::mousePosY;
F32 Input::deltaMousePosX;
F32 Input::deltaMousePosY;
F32 Input::deltaRawMousePosX;
F32 Input::deltaRawMousePosY;

F32 Input::axisStates[AXIS_CODE_COUNT];
Input::ButtonState Input::buttonStates[BUTTON_CODE_COUNT];
bool Input::receiveInput;
bool Input::anyButtonDown;
bool Input::anyButtonChanged;

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#pragma comment(lib ,"setupapi.lib")

constexpr I32 ANY_MOUSE_DOWN = RI_MOUSE_LEFT_BUTTON_DOWN | RI_MOUSE_RIGHT_BUTTON_DOWN | RI_MOUSE_MIDDLE_BUTTON_DOWN | RI_MOUSE_BUTTON_4_DOWN | RI_MOUSE_BUTTON_5_DOWN;

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
	rid[1].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK; //RIDEV_NOLEGACY
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

	mousePosX = (F32)(p.x - Settings::WindowPositionX());
	mousePosY = (F32)(p.y - Settings::WindowPositionY());

	int sensitivity;
	SystemParametersInfoA(SPI_GETMOUSESPEED, 0, &sensitivity, 0);

	mouseSensitivity = sensitivity / 10.0f;

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
	//Memory::Free(&deviceList);

	return true;
}

void Input::Shutdown()
{
	Logger::Trace("Shutting Down Input...");

	devices.Destroy();
}

void Input::Update()
{
	deltaMousePosX = 0;
	deltaMousePosY = 0;
	mouseWheelDelta = 0;
	mouseHWheelDelta = 0;

	anyButtonDown = false;
	anyButtonChanged = false;
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

		if (mouse.lLastX != 0 || mouse.lLastY != 0)
		{
			I32 relativeX = mouse.lLastX;
			I32 relativeY = mouse.lLastY;

			deltaRawMousePosX = relativeX * mouseSensitivity;
			deltaRawMousePosY = relativeY * mouseSensitivity;
		}

		POINT p;
		GetCursorPos(&p);
		
		HWND handle = WindowFromPoint(p);
		
		if (handle == Platform::GetWindowData().window)
		{
			if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
			{
				ButtonState& state = buttonStates[BUTTON_CODE_LEFT_MOUSE];

				state.heldChanged = state.pressed && !state.held;
				state.held = state.pressed;
				state.changed = !state.pressed;
				state.pressed = true;
				anyButtonDown |= state.changed;
				anyButtonChanged |= state.changed;
			}
			if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
			{
				ButtonState& state = buttonStates[BUTTON_CODE_RIGHT_MOUSE];

				state.heldChanged = state.pressed && !state.held;
				state.held = state.pressed;
				state.changed = !state.pressed;
				state.pressed = true;
				anyButtonDown |= state.changed;
				anyButtonChanged |= state.changed;
			}
			if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
			{
				ButtonState& state = buttonStates[BUTTON_CODE_MIDDLE_MOUSE];

				state.heldChanged = state.pressed && !state.held;
				state.held = state.pressed;
				state.changed = !state.pressed;
				state.pressed = true;
				anyButtonDown |= state.changed;
				anyButtonChanged |= state.changed;
			}
			if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
			{
				ButtonState& state = buttonStates[BUTTON_CODE_X_MOUSE_ONE];

				state.heldChanged = state.pressed && !state.held;
				state.held = state.pressed;
				state.changed = !state.pressed;
				state.pressed = true;
				anyButtonDown |= state.changed;
				anyButtonChanged |= state.changed;
			}
			if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
			{
				ButtonState& state = buttonStates[BUTTON_CODE_X_MOUSE_TWO];

				state.heldChanged = state.pressed && !state.held;
				state.held = state.pressed;
				state.changed = !state.pressed;
				state.pressed = true;
				anyButtonDown |= state.changed;
				anyButtonChanged |= state.changed;
			}

			if (mouse.usButtonFlags & RI_MOUSE_WHEEL) { mouseWheelDelta = (I16)((F32)(I16)mouse.usButtonData / WHEEL_DELTA); }
			if (mouse.usButtonFlags & RI_MOUSE_HWHEEL) { mouseHWheelDelta = (I16)((F32)(I16)mouse.usButtonData / WHEEL_DELTA); }
		}

		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
		{
			ButtonState& state = buttonStates[BUTTON_CODE_LEFT_MOUSE];

			state.changed = true;
			state.pressed = false;
			state.heldChanged = state.held;
			state.held = false;
			anyButtonChanged |= state.changed;
		}
		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
		{
			ButtonState& state = buttonStates[BUTTON_CODE_RIGHT_MOUSE];

			state.changed = true;
			state.pressed = false;
			state.heldChanged = state.held;
			state.held = false;
			anyButtonChanged |= state.changed;
		}
		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
		{
			ButtonState& state = buttonStates[BUTTON_CODE_MIDDLE_MOUSE];

			state.changed = true;
			state.pressed = false;
			state.heldChanged = state.held;
			state.held = false;
			anyButtonChanged |= state.changed;
		}
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
		{
			ButtonState& state = buttonStates[BUTTON_CODE_X_MOUSE_ONE];

			state.changed = true;
			state.pressed = false;
			state.heldChanged = state.held;
			state.held = false;
			anyButtonChanged |= state.changed;
		}
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
		{
			ButtonState& state = buttonStates[BUTTON_CODE_X_MOUSE_TWO];

			state.changed = true;
			state.pressed = false;
			state.heldChanged = state.held;
			state.held = false;
			anyButtonChanged |= state.changed;
		}

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
			anyButtonChanged |= state.changed;
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
			anyButtonChanged |= state.changed;
		} break;
		}
	} break;
	case RIM_TYPEHID: {

	} break;
	}
}

void Input::InputSink(HRAWINPUT handle)
{
	//RAWINPUT input{};
	//U32 size = 0;
	//if (GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0) { return; }
	//if (GetRawInputData(handle, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) < 1) { return; }
	//
	//if (input.header.dwType == RIM_TYPEMOUSE && input.data.mouse.usButtonFlags & ANY_MOUSE_DOWN)
	//{
	//	POINT p;
	//	GetCursorPos(&p);
	//
	//	HWND handle = WindowFromPoint(p);
	//
	//	if (handle == Platform::GetWindowData().window)
	//	{
	//		SetFocus(Platform::GetWindowData().window);
	//	}
	//}
}

void Input::Focus()
{
	//POINT p;
	//GetCursorPos(&p);
	//
	//if (Settings::CursorLocked())
	//{
	//	mousePosX = Settings::WindowWidth() / 2.0f;
	//	mousePosY = Settings::WindowHeight() / 2.0f;
	//}
	//else if (Settings::CursorConstrained())
	//{
	//	mousePosX = Math::Clamp((F32)p.x - Settings::WindowPositionX(), (F32)Settings::WindowPositionX(), (F32)Settings::WindowWidth() + (F32)Settings::WindowPositionX());
	//	mousePosY = Math::Clamp((F32)p.y - Settings::WindowPositionY(), (F32)Settings::WindowPositionY(), (F32)Settings::WindowHeight() + (F32)Settings::WindowPositionY());
	//}
	//else
	//{
	//	mousePosX = Math::Clamp((F32)p.x - Settings::WindowPositionX(), (F32)-Settings::WindowPositionX(), (F32)Settings::VirtualScreenWidth() - (F32)Settings::WindowPositionX());
	//	mousePosY = Math::Clamp((F32)p.y - Settings::WindowPositionY(), (F32)-Settings::WindowPositionY(), (F32)Settings::VirtualScreenHeight() - (F32)Settings::WindowPositionY());
	//}
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

bool Input::OnAnyButtonChanged() { return anyButtonChanged; }

bool Input::ButtonUp(ButtonCode code) { return receiveInput && !buttonStates[code].pressed; }

bool Input::ButtonDown(ButtonCode code) { return receiveInput && buttonStates[code].pressed; }

bool Input::ButtonHeld(ButtonCode code) { return receiveInput && buttonStates[code].held; }

bool Input::ButtonDragging(ButtonCode code) { return receiveInput && buttonStates[code].pressed && (deltaMousePosX || deltaMousePosY); }

bool Input::OnButtonUp(ButtonCode code) { return !buttonStates[code].pressed && buttonStates[code].changed && receiveInput; }

bool Input::OnButtonDown(ButtonCode code) { return buttonStates[code].pressed && buttonStates[code].changed && receiveInput; }

bool Input::OnButtonChange(ButtonCode code) { return buttonStates[code].changed && receiveInput; }

bool Input::OnButtonDoubleClick(ButtonCode code) { return buttonStates[code].doubleClicked && receiveInput; }

bool Input::OnButtonHold(ButtonCode code) { return buttonStates[code].held && buttonStates[code].heldChanged && receiveInput; }

bool Input::OnButtonRelease(ButtonCode code) { return !buttonStates[code].held && buttonStates[code].heldChanged && receiveInput; }

Vector2 Input::MousePosition() { return { mousePosX, mousePosY }; }

Vector2 Input::PreviousMousePos() { return { mousePosX - deltaMousePosX, mousePosY - deltaMousePosY }; }

Vector2 Input::MouseDelta() { return { deltaMousePosX, deltaMousePosY }; }

Vector2 Input::MouseToWorld(const Camera& camera)
{
	Vector4 area = Renderer::RenderArea();
	F32 scale = 0.125f * (1920.0f / area.z) * camera.Zoom();

	return { (mousePosX - area.x - area.z * 0.5f) * scale + camera.Eye().x, ((Settings::WindowHeight() - mousePosY) - area.y - area.w * 0.5f) * scale + camera.Eye().y };
}

void Input::ConsumeInput() { receiveInput = false; }

I16 Input::MouseWheelDelta() { return mouseWheelDelta; }

I16 Input::MouseHWheelDelta() { return mouseHWheelDelta; }

void Input::SetMousePosition(I32 x, I32 y)
{
	if (Settings::CursorLocked())
	{
		mousePosX = Settings::WindowWidth() / 2.0f;
		mousePosY = Settings::WindowHeight() / 2.0f;
	}
	else if (Settings::CursorConstrained())
	{
		mousePosX = Math::Clamp((F32)x, (F32)Settings::WindowPositionX(), (F32)Settings::WindowWidth() + (F32)Settings::WindowPositionX());
		mousePosY = Math::Clamp((F32)y, (F32)Settings::WindowPositionY(), (F32)Settings::WindowHeight() + (F32)Settings::WindowPositionY());
	}
	else
	{
		mousePosX = Math::Clamp((F32)x, (F32)-Settings::WindowPositionX(), (F32)Settings::VirtualScreenWidth() - (F32)Settings::WindowPositionX());
		mousePosY = Math::Clamp((F32)y, (F32)-Settings::WindowPositionY(), (F32)Settings::VirtualScreenHeight() - (F32)Settings::WindowPositionY());
	}

	SetCursorPos((I32)(Input::mousePosX + Settings::WindowPositionX()), (I32)(Input::mousePosY + Settings::WindowPositionY()));
}

void Input::ShowCursor(bool show)
{
	Settings::showCursor = show;
}

void Input::ConstrainCursor(bool constrain)
{
	Settings::constrainCursor = constrain;
}

void Input::LockCursor(bool lock)
{
	Settings::lockCursor = lock;
}

#endif