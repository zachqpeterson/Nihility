#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Containers/Vector.hpp"
#include "Math/Math.hpp"

enum class NH_API ButtonCode
{
	/*Mouse Buttons*/
	LeftMouse = 0x01,
	RightMouse = 0x02,
	MiddleMouse = 0x04,
	XMouseOne = 0x05,
	XMouseTwo = 0x06,
	ScrollWheel = 0xFF,

	/*Keyboard Buttons*/
	Cancel = 0x03,
	Back = 0x08,
	Tab = 0x09,
	Clear = 0x0C,
	Return = 0x0D,
	Shift = 0x10, //Either left or right shift
	Ctrl = 0x11, //Either left or right ctrl
	Alt = 0x12, //Either left or right alt
	Pause = 0x13,
	Capital = 0x14,
	Kana = 0x15,
	Hangeul = 0x15,
	Hangul = 0x15,
	Junja = 0x17,
	Final = 0x18,
	Hanja = 0x19,
	Kanja = 0x19,
	Escape = 0x1B,
	Convert = 0x1C,
	Nonconvert = 0x1D,
	Accept = 0x1E,
	ModeChange = 0x1F,
	Space = 0x20,
	Prior = 0x21,
	Next = 0x22,
	End = 0x23,
	Home = 0x24,
	Left = 0x25,
	Up = 0x26,
	Right = 0x27,
	Down = 0x28,
	Select = 0x29,
	Print = 0x2A,
	Execute = 0x2B,
	Snapshot = 0x2C,
	Insert = 0x2D,
	Delete = 0x2E,
	Help = 0x2F,
	Zero = 0x30,
	One = 0x31,
	Two = 0x32,
	Three = 0x33,
	Four = 0x34,
	Five = 0x35,
	Six = 0x36,
	Seven = 0x37,
	Eight = 0x38,
	Nine = 0x39,
	A = 0x41,
	B = 0x42,
	C = 0x43,
	D = 0x44,
	E = 0x45,
	F = 0x46,
	G = 0x47,
	H = 0x48,
	I = 0x49,
	J = 0x4A,
	K = 0x4B,
	L = 0x4C,
	M = 0x4D,
	N = 0x4E,
	O = 0x4F,
	P = 0x50,
	Q = 0x51,
	R = 0x52,
	S = 0x53,
	T = 0x54,
	U = 0x55,
	V = 0x56,
	W = 0x57,
	X = 0x58,
	Y = 0x59,
	Z = 0x5A,
	LWin = 0x5B,
	RWin = 0x5C,
	Apps = 0x5D,
	Sleep = 0x5F,
	Numpad0 = 0x60,
	Numpad1 = 0x61,
	Numpad2 = 0x62,
	Numpad3 = 0x63,
	Numpad4 = 0x64,
	Numpad5 = 0x65,
	Numpad6 = 0x66,
	Numpad7 = 0x67,
	Numpad8 = 0x68,
	Numpad9 = 0x69,
	NumpadMultiply = 0x6A,
	NumpadAdd = 0x6B,
	NumpadSeparator = 0x6C,
	NumpadSubtract = 0x6D,
	NumpadDecimal = 0x6E,
	NumpadDivide = 0x6F,
	F1 = 0x70,
	F2 = 0x71,
	F3 = 0x72,
	F4 = 0x73,
	F5 = 0x74,
	F6 = 0x75,
	F7 = 0x76,
	F8 = 0x77,
	F9 = 0x78,
	F10 = 0x79,
	F11 = 0x7A,
	F12 = 0x7B,
	F13 = 0x7C,
	F14 = 0x7D,
	F15 = 0x7E,
	F16 = 0x7F,
	F17 = 0x80,
	F18 = 0x81,
	F19 = 0x82,
	F20 = 0x83,
	F21 = 0x84,
	F22 = 0x85,
	F23 = 0x86,
	F24 = 0x87,
	NavigationView = 0x88,
	NavigationMenu = 0x89,
	NavigationUp = 0x8A,
	NavigationDown = 0x8B,
	NavigationLeft = 0x8C,
	NavigationRight = 0x8D,
	NavigationAccept = 0x8E,
	NavigationCancel = 0x8F,
	Numlock = 0x90,
	Scroll = 0x91,
	OemNecEqual = 0x92,
	OemFjJisho = 0x92,
	OemFjMasshou = 0x93,
	OemFjTouroku = 0x94,
	OemFjLoya = 0x95,
	OemFjRoya = 0x96,
	LShift = 0xA0,
	RShift = 0xA1,
	LCtrl = 0xA2,
	RCtrl = 0xA3,
	LAlt = 0xA4,
	RAlt = 0xA5,
	BrowserBack = 0xA6,
	BrowserForward = 0xA7,
	BrowserRefresh = 0xA8,
	BrowserStop = 0xA9,
	BrowserSearch = 0xAA,
	BrowserFavorites = 0xAB,
	BrowserHome = 0xAC,
	VolumeMute = 0xAD,
	VolumeDown = 0xAE,
	VolumeUp = 0xAF,
	MediaNextTrack = 0xB0,
	MediaPrevTrack = 0xB1,
	MediaStop = 0xB2,
	MediaPlayPause = 0xB3,
	LaunchMail = 0xB4,
	LaunchMediaSelect = 0xB5,
	LaunchApp1 = 0xB6,
	LaunchApp2 = 0xB7,
	OemSemicolon = 0xBA,
	OemPlus = 0xBB,
	OemComma = 0xBC,
	OemMinus = 0xBD,
	OemPeriod = 0xBE,
	OemForwardSlash = 0xBF,
	OemTilde = 0xC0,
	GamepadA = 0xC3,
	GamepadB = 0xC4,
	GamepadX = 0xC5,
	GamepadY = 0xC6,
	GamepadRightShoulder = 0xC7,
	GamepadLeftShoulder = 0xC8,
	GamepadLeftTrigger = 0xC9,
	GamepadRightTrigger = 0xCA,
	GamepadDPadUp = 0xCB,
	GamepadDPadDown = 0xCC,
	GamepadDPadLeft = 0xCD,
	GamepadDPadRight = 0xCE,
	GamepadMenu = 0xCF,
	GamepadView = 0xD0,
	GamepadLeftThumbstickButton = 0xD1,
	GamepadRightThumbstickButton = 0xD2,
	GamepadLeftThumbstickUp = 0xD3,
	GamepadLeftThumbstickDown = 0xD4,
	GamepadLeftThumbstickRight = 0xD5,
	GamepadLeftThumbstickLeft = 0xD6,
	GamepadRightThumbstickUp = 0xD7,
	GamepadRightThumbstickDown = 0xD8,
	GamepadRightThumbstickRight = 0xD9,
	GamepadRightThumbstickLeft = 0xDA,
	OemOBracket = 0xDB,
	OemBackSlash = 0xDC,
	OemCBracket = 0xDD,
	OemQuotes = 0xDE,
	Oem8 = 0xDF,
	OemAX = 0xE1,  //  'AX' key on Japanese AX kbd
	Oem102 = 0xE2,  //  "<>" or "\|" on RT 102-key kbd.
	IcoHelp = 0xE3,  //  Help key on ICO
	Ico00 = 0xE4,  //  00 key on ICO
	ProcessKey = 0xE5,
	IcoClear = 0xE6,
	Packet = 0xE7,
	OemReset = 0xE9,
	OemJump = 0xEA,
	OemPA1 = 0xEB,
	OemPA2 = 0xEC,
	OemPA3 = 0xED,
	OemWsctrl = 0xEE,
	OemCusel = 0xEF,
	OemAttn = 0xF0,
	OemFinish = 0xF1,
	OemCopy = 0xF2,
	OemAuto = 0xF3,
	OemEnlw = 0xF4,
	OemBacktab = 0xF5,
	Attn = 0xF6,
	Crsel = 0xF7,
	Exsel = 0xF8,
	Ereof = 0xF9,
	Play = 0xFA,
	Zoom = 0xFB,
	Noname = 0xFC,
	PA1 = 0xFD,
	OemClear = 0xFE,

	COUNT = 0x100
};

enum class NH_API AxisCode
{
	LeftJoystickX,
	LeftJoystickY,
	RightJoystickX,
	RightJoystickY,
	LeftTrigger,
	RightTrigger,

	COUNT
};

enum class NH_API InputType
{
	Press,
	Release,
	DoublePress,
	Hold
};

enum class NH_API TriggerEffectType
{
	None,
	Resistance,
	Weapon,
	Vibration
};

enum class NH_API Trigger
{
	Left,
	Right,
	Both
};

enum class NH_API LedColor
{
	Off,
	Red,
	Green,
	Blue,
	Magenta,
	Yellow,
	Cyan,
	White,
};

struct NH_API ButtonEvent
{
	ButtonCode code;
	InputType type;
	F64 timestamp;
};

struct _HIDP_PREPARSED_DATA;
struct TriggerEffect;

class NH_API Input
{
	struct ButtonState
	{
		bool pressed;
		bool changed;
		bool doubleClicked;
		bool held;
		bool heldChanged;
		F64 lastPressed;
	};
		
public:
	static void SetControllerRumbleStrength(F32 left, F32 right);
	static void SetControllerLedColor(F32 red, F32 green, F32 blue);
	static void SetControllerLedColor(LedColor color, F32 strength);
	static void SetControllerTriggerEffect(TriggerEffectType type, Trigger trigger);

	static F32 GetAxis(AxisCode code);
	static const Vector<ButtonEvent>& GetInputEvents();

	static bool ButtonUp(ButtonCode code);
	static bool ButtonDown(ButtonCode code);
	static bool ButtonHeld(ButtonCode code);
	static bool ButtonDragging(ButtonCode code);
	static bool OnButtonUp(ButtonCode code);
	static bool OnButtonDown(ButtonCode code);
	static bool OnButtonChange(ButtonCode code);
	static bool OnButtonDoubleClick(ButtonCode code);
	static bool OnButtonHold(ButtonCode code);
	static bool OnButtonRelease(ButtonCode code);
	static Vector2 MousePosition();
	static Vector2 PreviousMousePos();
	static Vector2 MouseDelta();
	static void ConsumeInput();
	static I16 MouseWheelDelta();
	static I16 MouseHWheelDelta();

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();
	static void UpdateRawInput(I64 lParam);
	static void UpdateConnectionStatus(void* deviceHandle, U64 status);
	static void ConnectHIDJoystick(void* deviceHandle);
	static void DisconnectHIDJoystick(void* deviceHandle);
	static void UpdateDualshock4(U32 index, U8 rawData[], UL32 byteCount);
	static void UpdateDualsense(U32 index, U8 rawData[], UL32 byteCount);
	static void UpdateXboxControllers();
	static void ParseGenericController(U32 index, U8 rawData[], UL32 dataSize, _HIDP_PREPARSED_DATA* preparsedData);

	static void SetDualsenseTriggerEffect(U8* dst, TriggerEffect effect);
	static void UpdateButtonState(ButtonCode code, bool value);
	static void UpdateAxisState(AxisCode code, F32 value);

	static bool useController;
	static U32 activeController;

	static ButtonState buttonStates[*ButtonCode::COUNT];
	static F32 axisStates[*AxisCode::COUNT];
	static Vector<ButtonEvent> events; //TODO: Clear events older than x frames
	static F64 currentTimestamp;
	static F64 holdThreshold;
	static F64 doublePressThreshold;

	static F32 mouseSensitivity;
	static I16 mouseWheelDelta;
	static I16 mouseHWheelDelta;
	static F32 mousePosX;
	static F32 mousePosY;
	static F32 deltaMousePosX;
	static F32 deltaMousePosY;
	static F32 deltaRawMousePosX;
	static F32 deltaRawMousePosY;

	static bool inputConsumed;

	friend class Engine;
	friend class Platform;

	STATIC_CLASS(Input);
};