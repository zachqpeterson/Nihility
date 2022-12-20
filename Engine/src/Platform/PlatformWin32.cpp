#include "Platform.hpp"

#ifdef PLATFORM_WINDOWS

#include "Core/Input.hpp"
#include "Core/Settings.hpp"
#include "Core/Logger.hpp"
#include "Core/Events.hpp"
#include "Math/Math.hpp"

#include "../resource.h"
#include <Windows.h>
#include <hidsdi.h>
#pragma comment(lib ,"hid.lib")

static U32(__stdcall* NtDelayExecution)(BOOL Alertable, PLARGE_INTEGER DelayInterval) = (U32(__stdcall*)(BOOL, PLARGE_INTEGER)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution");
static U32(__stdcall* ZwSetTimerResolution)(IN ULONG RequestedResolution, IN BOOLEAN Set, OUT PULONG ActualResolution) = (U32(__stdcall*)(ULONG, BOOLEAN, PULONG)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetTimerResolution");

I32 Platform::windowX;
I32 Platform::windowY;
I32 Platform::windowWidth;
I32 Platform::windowHeight;
I32 Platform::screenWidth;
I32 Platform::screenHeight;
bool Platform::running;

HINSTANCE Platform::instance;
HWND Platform::window;
U32 Platform::style;

HICON Platform::arrow;
HICON Platform::hand;
HICON Platform::sizeNS;
HICON Platform::sizeWE;
HICON Platform::sizeNESW;
HICON Platform::sizeNWSE;

#define MENU_NAME "Nihility Menu"
#define CLASS_NAME "Nihility Class"
#define WHEEL_MULTIPLIER 0.00833333333

bool Platform::Initialize(const String& applicationName)
{
	Logger::Info("Initializing platform...");

	ULONG actualResolution;
	ZwSetTimerResolution(1, true, &actualResolution);
	instance = GetModuleHandle(0);
	running = true;

	arrow = LoadCursor(nullptr, IDC_ARROW);
	hand = LoadCursor(nullptr, IDC_HAND);
	sizeNS = LoadCursor(nullptr, IDC_SIZENS);
	sizeWE = LoadCursor(nullptr, IDC_SIZEWE);
	sizeNESW = LoadCursor(nullptr, IDC_SIZENESW);
	sizeNWSE = LoadCursor(nullptr, IDC_SIZENWSE);

	// Setup and register window class.
	WNDCLASSEXA wc{};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = Win32MessageProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_ICON));
	wc.hCursor = arrow;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = MENU_NAME;
	wc.lpszClassName = CLASS_NAME;
	wc.hIconSm = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_ICON));

	if (!RegisterClassExA(&wc))
	{
		MessageBoxA(0, "Window registration failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (Settings::Fullscreen)
	{
		Settings::WINDOW_POSITION_X = 0;
		Settings::WINDOW_POSITION_Y = 0;
		Settings::WINDOW_WIDTH = screenWidth;
		Settings::WINDOW_HEIGHT = screenHeight;

		style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZE;
	}
	else
	{
		Settings::WINDOW_POSITION_X = Settings::WindowPositionXSmall;
		Settings::WINDOW_POSITION_Y = Settings::WindowPositionYSmall;
		Settings::WINDOW_WIDTH = Settings::WindowWidthSmall;
		Settings::WINDOW_HEIGHT = Settings::WindowHeightSmall;

		style = WS_OVERLAPPEDWINDOW;
	}

	RECT borderRect{};
	AdjustWindowRectEx(&borderRect, style, 0, 0);

	windowX = Settings::WindowPositionX + borderRect.left;
	windowY = Settings::WindowPositionY + borderRect.top;
	windowWidth = Settings::WindowWidth + borderRect.right - borderRect.left;
	windowHeight = Settings::WindowHeight + borderRect.bottom - borderRect.top;

	window = CreateWindowExA(0, CLASS_NAME, applicationName, style, windowX, windowY, windowWidth, windowHeight, 0, 0, instance, 0);

	if (!window)
	{
		MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	DEVMODEA monitorInfo{};
	EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &monitorInfo);
	if (Settings::TargetFrametime == 0.0) { Settings::TARGET_FRAMETIME = 1.0 / monitorInfo.dmDisplayFrequency; }

	ShowWindow(window, Settings::Fullscreen ? SW_SHOWMAXIMIZED : SW_SHOW);

	return true;
}

void Platform::Shutdown()
{
	if (window)
	{
		DestroyWindow(window);
		window = nullptr;
	}
}

bool Platform::Update()
{
	Input::Update();

	MSG message;

	while (PeekMessageA(&message, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	UpdateMouse();

	return running;
}

void Platform::SetFullscreen(bool fullscreen)
{
	Settings::FULLSCREEN = fullscreen;

	if (fullscreen)
	{
		Settings::WINDOW_POSITION_X = 0;
		Settings::WINDOW_POSITION_Y = 0;
		Settings::WINDOW_WIDTH = screenWidth;
		Settings::WINDOW_HEIGHT = screenHeight;

		style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZE;
	}
	else
	{
		Settings::WINDOW_POSITION_X = Settings::WindowPositionXSmall;
		Settings::WINDOW_POSITION_Y = Settings::WindowPositionYSmall;
		Settings::WINDOW_WIDTH = Settings::WindowWidthSmall;
		Settings::WINDOW_HEIGHT = Settings::WindowHeightSmall;

		style = WS_OVERLAPPEDWINDOW;
	}

	RECT borderRect{};
	AdjustWindowRectEx(&borderRect, style, 0, 0);

	windowX = Settings::WindowPositionX + borderRect.left;
	windowY = Settings::WindowPositionY + borderRect.top;
	windowWidth = Settings::WindowWidth + borderRect.right - borderRect.left;
	windowHeight = Settings::WindowHeight + borderRect.bottom - borderRect.top;

	SetWindowLongPtrA(window, GWL_STYLE, style);
	SetWindowPos(window, 0, windowX, windowY, windowWidth, windowHeight, SWP_SHOWWINDOW);
}

void Platform::SleepFor(U64 ns)
{
	LARGE_INTEGER interval;
	interval.QuadPart = -1 * (I32)(ns);
	NtDelayExecution(false, &interval);
}

void Platform::UpdateMouse()
{
	if (Settings::Focused)
	{
		if (Settings::ConstrainCursor)
		{
			RECT clip{};
			if (Settings::Fullscreen)
			{
				clip.left = Settings::WindowPositionX;
				clip.top = Settings::WindowPositionY;
				clip.right = Settings::WindowPositionX + Settings::WindowWidth;
				clip.bottom = Settings::WindowPositionY + Settings::WindowHeight;
				ClipCursor(&clip);
			}
			else
			{
				GetWindowRect(window, &clip);
				ClipCursor(&clip);
			}
		}
		else if (Settings::LockCursor)
		{
			RECT clip{};
			clip.left = Settings::WindowPositionX + (I32)(Settings::WindowWidth * 0.5f);
			clip.right = clip.left;
			clip.top = Settings::WindowPositionY + (I32)(Settings::WindowHeight * 0.5f);
			clip.bottom = clip.top;

			ClipCursor(&clip);
		}

		ShowCursor(!Settings::HideCursor);
	}
	else
	{
		ShowCursor(true);
		ClipCursor(nullptr);
	}
}

void* Platform::Allocate(U64 size, bool aligned)
{
	return malloc(size);
}

void Platform::Free(void* block, bool aligned)
{
	free(block);
}

void* Platform::Zero(void* block, U64 size)
{
	return memset(block, 0, size);
}

void* Platform::Copy(void* dest, const void* source, U64 size)
{
	return memcpy(dest, source, size);
}

void* Platform::Set(void* dest, I32 value, U64 size)
{
	return memset(dest, value, size);
}

void Platform::GetVulkanSurfaceInfo(void* surfaceInfo)
{
	HINSTANCE* arr = (HINSTANCE*)surfaceInfo;
	arr[0] = instance;
	arr[1] = *(HINSTANCE*)&window;
}

I64 __stdcall Platform::Win32MessageProc(HWND__* hwnd, U32 msg, U64 wParam, I64 lParam)
{
	switch (msg)
	{
	case WM_CREATE: {
		POINT point{};
		GetCursorPos(&point);
		Input::mousePos.x = point.x - windowX;
		Input::mousePos.y = point.y - windowY;

		RAWINPUTDEVICE rid[4];

		rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
		rid[0].dwFlags = 0;
		rid[0].hwndTarget = window;

		rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
		rid[1].dwFlags = 0;
		rid[1].hwndTarget = window;

		/*rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[2].usUsage = HID_USAGE_GENERIC_MOUSE;
		rid[2].dwFlags = 0;
		rid[2].hwndTarget = window;

		rid[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		rid[3].dwFlags = 0;
		rid[3].hwndTarget = window;*/

		if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) { return -1; }
	} return 0;
	case WM_INPUT: {
		U32 rawInputBufferSize{};

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 0, &rawInputBufferSize, sizeof(RAWINPUTHEADER));

		U8* rawInputBuffer = (U8*)malloc(rawInputBufferSize);
		if (!rawInputBuffer) { break; }
		I32 bytesCopied = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawInputBuffer, &rawInputBufferSize, sizeof(RAWINPUTHEADER));

		if (bytesCopied)
		{
			RAWINPUT* rawInput = (RAWINPUT*)rawInputBuffer;

			U32 preparsedDataBufferSize{};

			GetRawInputDeviceInfoA(rawInput->header.hDevice, RIDI_PREPARSEDDATA, 0, &preparsedDataBufferSize);
			PHIDP_PREPARSED_DATA preparsedData = (PHIDP_PREPARSED_DATA)malloc(preparsedDataBufferSize);
			if (!preparsedData)
			{
				free(rawInputBuffer);
				break;
			}
			GetRawInputDeviceInfoA(rawInput->header.hDevice, RIDI_PREPARSEDDATA, preparsedData, &preparsedDataBufferSize);

			RID_DEVICE_INFO di{};
			di.cbSize = sizeof(RID_DEVICE_INFO);
			U32 diSize = di.cbSize;

			GetRawInputDeviceInfoA(rawInput->header.hDevice, RIDI_DEVICEINFO, &di, &diSize);

			switch (rawInput->header.dwType)
			{
			case RIM_TYPEMOUSE: {
				/*RAWMOUSE& rawMouse = rawInput->data.mouse;

				if ((rawMouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
				{
					bool isVirtualDesktop = (rawMouse.usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP;

					I32 width = GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
					I32 height = GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);

					Input::mousePos.x = I32((rawMouse.lLastX / 65535.0f) * width);
					Input::mousePos.y = I32((rawMouse.lLastY / 65535.0f) * height);
				}
				else if (rawMouse.lLastX != 0 || rawMouse.lLastY != 0)
				{
					Input::mousePos.x += rawMouse.lLastX;
					Input::mousePos.y += rawMouse.lLastY;
				}

				if ((rawMouse.usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL ||
					(rawMouse.usButtonFlags & RI_MOUSE_HWHEEL) == RI_MOUSE_HWHEEL)
				{
					static const U32 defaultScrollLinesPerWheelDelta = 3;
					static const U32 defaultScrollCharsPerWheelDelta = 1;

					F32 wheelDelta = (F32)(U16)rawMouse.usButtonData;
					F32 numTicks = wheelDelta / WHEEL_DELTA;

					bool isHorizontalScroll = (rawMouse.usButtonFlags & RI_MOUSE_HWHEEL) == RI_MOUSE_HWHEEL;
					bool isScrollByPage = false;
					F32 scrollDelta = numTicks;

					if (isHorizontalScroll)
					{
						U32 scrollChars = defaultScrollCharsPerWheelDelta;
						SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &scrollChars, 0);
						scrollDelta *= scrollChars;
					}
					else
					{
						U32 scrollLines = defaultScrollLinesPerWheelDelta;
						SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0);
						if (scrollLines == U32_MAX) { isScrollByPage = true; }
						else { scrollDelta *= scrollLines; }
					}
				}

				printf("%d, %d\n", Input::mousePos.x, Input::mousePos.y);*/
			} break;
			case RIM_TYPEKEYBOARD: {

			} break;
			case RIM_TYPEHID: {
				HIDP_CAPS caps;
				if (HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS) { break; }

				if (caps.NumberInputButtonCaps)
				{
					PHIDP_BUTTON_CAPS buttonCaps = nullptr;
					if (!(buttonCaps = (PHIDP_BUTTON_CAPS)malloc(sizeof(HIDP_BUTTON_CAPS) * caps.NumberInputButtonCaps))) { break; }

					U16 buttonCapsLength = caps.NumberInputButtonCaps;
					if (HidP_GetButtonCaps(HidP_Input, buttonCaps, &buttonCapsLength, preparsedData) != HIDP_STATUS_SUCCESS || buttonCapsLength == 0) { free(buttonCaps); break; }
					U32 numberOfButtons = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;

					ULONG usageLength{};
					HidP_GetUsages(HidP_Input, buttonCaps->UsagePage, 0, nullptr, &usageLength, preparsedData,
						(PCHAR)rawInput->data.hid.bRawData, rawInput->data.hid.dwSizeHid);

					if (usageLength)
					{
						U16* usage = nullptr;
						if (!(usage = (U16*)malloc(usageLength))) { free(buttonCaps); break; }

						if (HidP_GetUsages(HidP_Input, buttonCaps->UsagePage, 0, usage, &usageLength, preparsedData,
							(PCHAR)rawInput->data.hid.bRawData, rawInput->data.hid.dwSizeHid) != HIDP_STATUS_SUCCESS || usageLength == 0)
						{
							free(usage);
							free(buttonCaps);
						}

						I32* buttonStates = nullptr;
						if (!(buttonStates = (I32*)malloc(sizeof(I32) * numberOfButtons)))
						{
							free(usage);
							free(buttonCaps);
						}

						for (U32 i = 0; i < usageLength; ++i)
						{
							I32 buttonIndex = usage[i] - buttonCaps->Range.UsageMin;
							buttonStates[buttonIndex] = true;
						}
					}

					free(buttonCaps);
				}

				if (caps.NumberInputValueCaps)
				{
					PHIDP_VALUE_CAPS valueCaps = nullptr;
					if (!(valueCaps = (PHIDP_VALUE_CAPS)malloc(sizeof(HIDP_VALUE_CAPS) * caps.NumberInputValueCaps))) { break; }
					U16 valueCapsLength = caps.NumberInputValueCaps;

					if (HidP_GetValueCaps(HidP_Input, valueCaps, &valueCapsLength, preparsedData) != HIDP_STATUS_SUCCESS || valueCapsLength == 0)
					{
						free(valueCaps);
						break;
					}

					if (di.hid.dwVendorId == 1118) //Xbox
					{
						for (U32 i = 0; i < caps.NumberInputValueCaps; ++i)
						{
							ULONG value{};
							if (HidP_GetUsageValue(HidP_Input, valueCaps[i].LinkUsagePage, 0, valueCaps[i].Range.UsageMin,
								&value, preparsedData, (PCHAR)rawInput->data.hid.bRawData, rawInput->data.hid.dwSizeHid) != HIDP_STATUS_SUCCESS)
							{
								continue;
							}

							switch (valueCaps[i].Range.UsageMin)
							{
							case 0x30: {
								//XBox: Left Axis X 0-65535
								F32 axis = (F32)value - 32768.0f;
								Input::axisStates[LEFT_JOYSTICK_X] = axis / (32767.0f + (axis < 0));
							} break;

							case 0x31: {
								//XBox: Left Axis Y 0-65535
								F32 axis = (F32)value - 32768.0f;
								Input::axisStates[LEFT_JOYSTICK_Y] = axis / (-32767.0f + (axis < 0));
							} break;

							case 0x32: {
								//XBox: Left and right trigger 0-65535
								//XBox X: Left and right trigger 128-65408
								Input::axisStates[LEFT_TRIGGER] = (F32)(value - 32768.0f) / 32768.0f;
								Input::axisStates[RIGHT_TRIGGER] = value / 65535.0f;
							} break;

							case 0x33: {
								//XBox: Right Axis X 0-65535
								F32 axis = (F32)value - 32768.0f;
								Input::axisStates[RIGHT_JOYSTICK_X] = axis / (32767.0f + (axis < 0));
							} break;

							case 0x34: {
								//XBox: Right Axis Y 0-65535
								F32 axis = (F32)value - 32768.0f;
								Input::axisStates[RIGHT_JOYSTICK_Y] = axis / (-32767.0f + (axis < 0));
							} break;

							case 0x39: { //D-Pad
								switch (value)
								{
								case 1: {
									Input::buttonStates[GAMEPAD_DPAD_UP].changed = Input::buttonStates[GAMEPAD_DPAD_UP].pressed;
									Input::buttonStates[GAMEPAD_DPAD_UP].pressed = true;
								} break;
								case 2: {
									Input::buttonStates[GAMEPAD_DPAD_UP].changed = Input::buttonStates[GAMEPAD_DPAD_UP].pressed;
									Input::buttonStates[GAMEPAD_DPAD_UP].pressed = true;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].changed = Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed = true;
								} break;
								case 3: {
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].changed = Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed = true;
								} break;
								case 4: {
									Input::buttonStates[GAMEPAD_DPAD_DOWN].changed = Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed;
									Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed = true;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].changed = Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed = true;
								} break;
								case 5: {
									Input::buttonStates[GAMEPAD_DPAD_DOWN].changed = Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed;
									Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed = true;
								} break;
								case 6: {
									Input::buttonStates[GAMEPAD_DPAD_DOWN].changed = Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed;
									Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed = true;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].changed = Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed = true;
								} break;
								case 7: {
									Input::buttonStates[GAMEPAD_DPAD_LEFT].changed = Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed = true;
								} break;
								case 8: {
									Input::buttonStates[GAMEPAD_DPAD_UP].changed = Input::buttonStates[GAMEPAD_DPAD_UP].pressed;
									Input::buttonStates[GAMEPAD_DPAD_UP].pressed = true;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].changed = Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed = true;
								} break;
								case 0:
								default: {} break;
								}
							} break;
							}
						}
					}
					else if (di.hid.dwVendorId == 1356) //Play Station
					{
						for (U32 i = 0; i < caps.NumberInputValueCaps; ++i)
						{
							ULONG value{};
							if (HidP_GetUsageValue(HidP_Input, valueCaps[i].LinkUsagePage, 0, valueCaps[i].Range.UsageMin,
								&value, preparsedData, (PCHAR)rawInput->data.hid.bRawData, rawInput->data.hid.dwSizeHid) != HIDP_STATUS_SUCCESS)
							{
								continue;
							}

							switch (valueCaps[i].Range.UsageMin)
							{
							case 0x30: {
								//PS: Left Axis X 0-255
								F32 lAxisX = (F32)value - 128.0f;
								Input::axisStates[LEFT_JOYSTICK_X] = lAxisX / (127.0f + (lAxisX < 0));
							} break;

							case 0x31: {
								//PS: Left Axis Y 0-255
								F32 lAxisY = (F32)value - 128.0f;
								Input::axisStates[LEFT_JOYSTICK_Y] = lAxisY / (-127.0f - (lAxisY < 0));
							} break;

							case 0x32: {
								//PS: Right Axis X 0-255
								F32 rAxisX = (F32)value - 128.0f;
								Input::axisStates[RIGHT_JOYSTICK_X] = rAxisX / (127.0f + (rAxisX < 0));
							} break;

							case 0x33: {
								//PS: Left Trigger 0-255
								Input::axisStates[LEFT_TRIGGER] = value / 255.0f;
							} break;

							case 0x34: {
								//PS: Right Trigger 0-255
								Input::axisStates[RIGHT_TRIGGER] = value / 255.0f;
							} break;

							case 0x35: {
								//PS: Right Axis Y 0-255
								F32 rAxisY = (F32)value - 128.0f;
								Input::axisStates[RIGHT_JOYSTICK_Y] = rAxisY / (-127.0f - (rAxisY < 0));
							} break;

							case 0x39: { //D-Pad
								switch (value)
								{
								case 0: {
									Input::buttonStates[GAMEPAD_DPAD_UP].changed = Input::buttonStates[GAMEPAD_DPAD_UP].pressed;
									Input::buttonStates[GAMEPAD_DPAD_UP].pressed = true;
								} break;
								case 1: {
									Input::buttonStates[GAMEPAD_DPAD_UP].changed = Input::buttonStates[GAMEPAD_DPAD_UP].pressed;
									Input::buttonStates[GAMEPAD_DPAD_UP].pressed = true;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].changed = Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed = true;
								} break;
								case 2: {
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].changed = Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed = true;
								} break;
								case 3: {
									Input::buttonStates[GAMEPAD_DPAD_DOWN].changed = Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed;
									Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed = true;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].changed = Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_RIGHT].pressed = true;
								} break;
								case 4: {
									Input::buttonStates[GAMEPAD_DPAD_DOWN].changed = Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed;
									Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed = true;
								} break;
								case 5: {
									Input::buttonStates[GAMEPAD_DPAD_DOWN].changed = Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed;
									Input::buttonStates[GAMEPAD_DPAD_DOWN].pressed = true;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].changed = Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed = true;
								} break;
								case 6: {
									Input::buttonStates[GAMEPAD_DPAD_LEFT].changed = Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed = true;
								} break;
								case 7: {
									Input::buttonStates[GAMEPAD_DPAD_UP].changed = Input::buttonStates[GAMEPAD_DPAD_UP].pressed;
									Input::buttonStates[GAMEPAD_DPAD_UP].pressed = true;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].changed = Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed;
									Input::buttonStates[GAMEPAD_DPAD_LEFT].pressed = true;
								} break;
								case 8:
								default: {} break;
								}
							} break;
							}
						}
					}
					else
					{
						Logger::Error("Unknown Gamepad");
						DebugBreak();
					}

					free(valueCaps);
				}
			} break;
			}

			free(preparsedData);
		}

		free(rawInputBuffer);
	} return 0;
	case WM_DEVICECHANGE: {
		//TODO: 
	} break;
	case WM_SETFOCUS: { Settings::FOCUSED = true; } return 0;
	case WM_KILLFOCUS: { Settings::FOCUSED = false; } return 0;
	case WM_ERASEBKGND: {} return 1;
	case WM_CLOSE: { Events::Notify("CLOSE", nullptr); running = false; } return 0;
	case WM_DESTROY: { PostQuitMessage(0); running = false; } return 0;
	case WM_SIZE: {
		RECT rect{};
		GetWindowRect(hwnd, &rect);

		windowX = rect.left;
		windowY = rect.top;
		windowWidth = rect.right - rect.left;
		windowHeight = rect.bottom - rect.top;

		RECT borderRect{};
		AdjustWindowRectEx(&borderRect, style, 0, 0);

		screenWidth = windowWidth - borderRect.right + borderRect.left;
		screenHeight = windowHeight - borderRect.bottom + borderRect.top;

		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_POSITION_X_SMALL = windowX - borderRect.left;
			Settings::WINDOW_POSITION_Y_SMALL = windowY - borderRect.top;
			Settings::WINDOW_WIDTH_SMALL = windowWidth - borderRect.right + borderRect.left;
			Settings::WINDOW_HEIGHT_SMALL = windowHeight - borderRect.bottom + borderRect.top;
		}

		Settings::WINDOW_POSITION_X = windowX - borderRect.left;
		Settings::WINDOW_POSITION_Y = windowY - borderRect.top;
		Settings::WINDOW_WIDTH = windowWidth - borderRect.right + borderRect.left;
		Settings::WINDOW_HEIGHT = windowHeight - borderRect.bottom + borderRect.top;

		Events::Notify("Resize", NULL);
	} return 0;
	case WM_MOVE: {
		if (!Settings::Fullscreen)
		{
			Settings::WINDOW_POSITION_X_SMALL = LOWORD(lParam);
			Settings::WINDOW_POSITION_Y_SMALL = HIWORD(lParam);
		}

		Settings::WINDOW_POSITION_X = LOWORD(lParam);
		Settings::WINDOW_POSITION_Y = HIWORD(lParam);

		RECT borderRect{};
		AdjustWindowRectEx(&borderRect, style, 0, 0);

		windowX = Settings::WindowPositionX + borderRect.left;
		windowY = Settings::WindowPositionY + borderRect.top;
	} return 0;

		//INPUT
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN: {
		if (HIWORD(lParam) & KF_EXTENDED)
		{
			U8 code = (U8)LOWORD(MapVirtualKeyW(MAKEWORD(LOBYTE(HIWORD(lParam)), 0xE0), MAPVK_VSC_TO_VK_EX));

			Input::buttonStates[code].heldChanged = Input::buttonStates[code].pressed && !Input::buttonStates[code].held;
			Input::buttonStates[code].held = Input::buttonStates[code].pressed;
			Input::buttonStates[code].changed = !Input::buttonStates[code].pressed;
			Input::buttonStates[code].pressed = true;
		}
		else
		{
			Input::buttonStates[wParam].heldChanged = Input::buttonStates[wParam].pressed && !Input::buttonStates[wParam].held;
			Input::buttonStates[wParam].held = Input::buttonStates[wParam].pressed;
			Input::buttonStates[wParam].changed = !Input::buttonStates[wParam].pressed;
			Input::buttonStates[wParam].pressed = true;
			Input::anyButtonDown |= Input::buttonStates[wParam].changed;
		}
	} return 0;
	case WM_LBUTTONDOWN: {
		Input::buttonStates[LEFT_CLICK].heldChanged = Input::buttonStates[LEFT_CLICK].pressed && !Input::buttonStates[LEFT_CLICK].held;
		Input::buttonStates[LEFT_CLICK].held = Input::buttonStates[LEFT_CLICK].pressed;
		Input::buttonStates[LEFT_CLICK].changed = !Input::buttonStates[LEFT_CLICK].pressed;
		Input::buttonStates[LEFT_CLICK].pressed = true;
		Input::anyButtonDown |= Input::buttonStates[LEFT_CLICK].changed;
	} return 0;
	case WM_RBUTTONDOWN: {
		Input::buttonStates[RIGHT_CLICK].heldChanged = Input::buttonStates[RIGHT_CLICK].pressed && !Input::buttonStates[RIGHT_CLICK].held;
		Input::buttonStates[RIGHT_CLICK].held = Input::buttonStates[RIGHT_CLICK].pressed;
		Input::buttonStates[RIGHT_CLICK].changed = !Input::buttonStates[RIGHT_CLICK].pressed;
		Input::buttonStates[RIGHT_CLICK].pressed = true;
		Input::anyButtonDown |= Input::buttonStates[RIGHT_CLICK].changed;
	} return 0;
	case WM_MBUTTONDOWN: {
		Input::buttonStates[MIDDLE_CLICK].heldChanged = Input::buttonStates[MIDDLE_CLICK].pressed && !Input::buttonStates[MIDDLE_CLICK].held;
		Input::buttonStates[MIDDLE_CLICK].held = Input::buttonStates[MIDDLE_CLICK].pressed;
		Input::buttonStates[MIDDLE_CLICK].changed = !Input::buttonStates[MIDDLE_CLICK].pressed;
		Input::buttonStates[MIDDLE_CLICK].pressed = true;
		Input::anyButtonDown |= Input::buttonStates[MIDDLE_CLICK].changed;
	} return 0;
	case WM_XBUTTONDOWN: {
		if (wParam == MK_XBUTTON1)
		{
			Input::buttonStates[XBUTTON_ONE].heldChanged = Input::buttonStates[XBUTTON_ONE].pressed && !Input::buttonStates[XBUTTON_ONE].held;
			Input::buttonStates[XBUTTON_ONE].held = Input::buttonStates[XBUTTON_ONE].pressed;
			Input::buttonStates[XBUTTON_ONE].changed = !Input::buttonStates[XBUTTON_ONE].pressed;
			Input::buttonStates[XBUTTON_ONE].pressed = true;
			Input::anyButtonDown |= Input::buttonStates[XBUTTON_ONE].changed;
		}
		else
		{
			Input::buttonStates[XBUTTON_TWO].heldChanged = Input::buttonStates[XBUTTON_TWO].pressed && !Input::buttonStates[XBUTTON_TWO].held;
			Input::buttonStates[XBUTTON_TWO].held = Input::buttonStates[XBUTTON_TWO].pressed;
			Input::buttonStates[XBUTTON_TWO].changed = !Input::buttonStates[XBUTTON_TWO].pressed;
			Input::buttonStates[XBUTTON_TWO].pressed = true;
			Input::anyButtonDown |= Input::buttonStates[XBUTTON_TWO].changed;
		}
	} return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP: {
		if (HIWORD(lParam) & KF_EXTENDED)
		{
			U8 code = (U8)LOWORD(MapVirtualKeyW(MAKEWORD(LOBYTE(HIWORD(lParam)), 0xE0), MAPVK_VSC_TO_VK_EX));

			Input::buttonStates[code].changed = true;
			Input::buttonStates[code].pressed = false;
			Input::buttonStates[code].heldChanged = Input::buttonStates[code].held;
			Input::buttonStates[code].held = false;
		}
		else
		{
			Input::buttonStates[wParam].changed = true;
			Input::buttonStates[wParam].pressed = false;
			Input::buttonStates[wParam].heldChanged = Input::buttonStates[wParam].held;
			Input::buttonStates[wParam].held = false;
		}
	} return 0;
	case WM_LBUTTONUP: {
		Input::buttonStates[LEFT_CLICK].changed = true;
		Input::buttonStates[LEFT_CLICK].pressed = false;
		Input::buttonStates[LEFT_CLICK].heldChanged = Input::buttonStates[LEFT_CLICK].held;
		Input::buttonStates[LEFT_CLICK].held = false;
	} return 0;
	case WM_RBUTTONUP: {
		Input::buttonStates[RIGHT_CLICK].changed = true;
		Input::buttonStates[RIGHT_CLICK].pressed = false;
		Input::buttonStates[RIGHT_CLICK].heldChanged = Input::buttonStates[RIGHT_CLICK].held;
		Input::buttonStates[RIGHT_CLICK].held = false;
	} return 0;
	case WM_MBUTTONUP: {
		Input::buttonStates[MIDDLE_CLICK].changed = true;
		Input::buttonStates[MIDDLE_CLICK].pressed = false;
		Input::buttonStates[MIDDLE_CLICK].heldChanged = Input::buttonStates[MIDDLE_CLICK].held;
		Input::buttonStates[MIDDLE_CLICK].held = false;
	} return 0;
	case WM_XBUTTONUP: {
		if (wParam == MK_XBUTTON1)
		{
			Input::buttonStates[XBUTTON_ONE].changed = true;
			Input::buttonStates[XBUTTON_ONE].pressed = false;
			Input::buttonStates[XBUTTON_ONE].heldChanged = Input::buttonStates[XBUTTON_ONE].held;
			Input::buttonStates[XBUTTON_ONE].held = false;
		}
		else
		{
			Input::buttonStates[XBUTTON_TWO].changed = true;
			Input::buttonStates[XBUTTON_TWO].pressed = false;
			Input::buttonStates[XBUTTON_TWO].heldChanged = Input::buttonStates[XBUTTON_TWO].held;
			Input::buttonStates[XBUTTON_TWO].held = false;
		}
	} return 0;
	case WM_LBUTTONDBLCLK: {
		Input::buttonStates[LEFT_CLICK].changed = true;
		Input::buttonStates[LEFT_CLICK].pressed = true;
		Input::buttonStates[LEFT_CLICK].doubleClicked = true;
	} return 0;
	case WM_RBUTTONDBLCLK: {
		Input::buttonStates[RIGHT_CLICK].changed = true;
		Input::buttonStates[RIGHT_CLICK].pressed = true;
		Input::buttonStates[RIGHT_CLICK].doubleClicked = true;
	} return 0;
	case WM_MBUTTONDBLCLK: {
		Input::buttonStates[MIDDLE_CLICK].changed = true;
		Input::buttonStates[MIDDLE_CLICK].pressed = true;
		Input::buttonStates[MIDDLE_CLICK].doubleClicked = true;
	} return 0;
	case WM_XBUTTONDBLCLK: {
		if (wParam == MK_XBUTTON1)
		{
			Input::buttonStates[XBUTTON_ONE].changed = true;
			Input::buttonStates[XBUTTON_ONE].pressed = true;
			Input::buttonStates[XBUTTON_ONE].doubleClicked = true;
		}
		else
		{
			Input::buttonStates[XBUTTON_TWO].changed = true;
			Input::buttonStates[XBUTTON_TWO].pressed = true;
			Input::buttonStates[XBUTTON_TWO].doubleClicked = true;
		}
	} return 0;
	case WM_MOUSEWHEEL: {
		Input::mouseWheelDelta = HIWORD((I16)((F32)HIWORD(wParam) * WHEEL_MULTIPLIER));
	} return 0;
	case WM_MOUSEMOVE: {
		Input::mousePos = { LOWORD(lParam), HIWORD(lParam) };
	} return 0;

		//CURSOR
	case WM_SETCURSOR: {
		if (Settings::Fullscreen)
		{
			SetCursor(arrow);
			return 1;
		}
		else
		{
			switch (LOWORD(lParam))
			{
			case HTBOTTOM:
			case HTTOP: { SetCursor(sizeNS); } return 1;
			case HTRIGHT:
			case HTLEFT: { SetCursor(sizeWE); } return 1;
			case HTBOTTOMLEFT:
			case HTTOPRIGHT: { SetCursor(sizeNESW); } return 1;
			case HTBOTTOMRIGHT:
			case HTTOPLEFT: { SetCursor(sizeNWSE); } return 1;
			case HTCLOSE:
			case HTMAXBUTTON:
			case HTMINBUTTON: { SetCursor(hand); } return 1;
			case HTCAPTION:
			case HTBORDER:
			case HTNOWHERE:
			case HTOBJECT:
			case HTHELP:
			case HTGROWBOX:
			case HTMENU:
			case HTHSCROLL:
			case HTVSCROLL:
			case HTCLIENT: { SetCursor(arrow); } return 1;
			default: {} return 0;
			}
		}
	} break;
	}

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#endif