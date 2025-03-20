#include "Input.hpp"

#include "Platform.hpp"
#include "Memory.hpp"

#include "Core/Logger.hpp"

#ifdef NH_PLATFORM_WINDOWS

#include "WindowsInclude.hpp"

#include <Xinput.h>
#include <hidsdi.h>
#include <hidpi.h>
#pragma comment(lib ,"xinput.lib")
#pragma comment(lib ,"hid.lib")

enum class JoystickType
{
	Generic,
	Xbox,
	Dualshock4,
	Dualsense,
};

enum class TriggerEffectType
{
	None,
	Resistance,
	Weapon,
	Vibration
};

struct TriggerEffect
{
	TriggerEffectType type;

	union {
		struct {
			F32 zoneStrengths[10];
		} resistance;
		struct {
			F32 startPosition;
			F32 endPosition;
			F32 strength;
		} weapon;
		struct {
			F32 zoneStrengths[10];
			F32 frequency;
		} vibration;
	};
};

struct JoystickState
{
	static const U32 inputCount = 64;
	static const U32 maxNameLength = 128;

	// Inputs
	bool connected;
	JoystickType type;
	HANDLE deviceHandle;
	F32 currentInputs[inputCount];
	F32 previousInputs[inputCount];
	CW deviceName[maxNameLength];
	CW productName[maxNameLength];
	CW manufacturerName[maxNameLength];

	// Outputs
	F32 lightRumble;
	F32 heavyRumble;
	F32 ledRed;
	F32 ledGreen;
	F32 ledBlue;
	TriggerEffect leftTriggerEffect;
	TriggerEffect rightTriggerEffect;

	U8 outputBuffer[96];
	HANDLE outputFile;
	OVERLAPPED overlapped;
};

struct Joysticks
{
	static const U32 maxXinputControllers = 4;
	U32 count;
	JoystickState* states;
	HWND hwnd;
};

enum class XboxInputs
{
	A,
	B,
	X,
	Y,
	DpadLeft,
	DpadRight,
	DpadUp,
	DpadDown,
	LeftBumper,
	RightBumper,
	LeftStickButton,
	RightStickButton,
	Back,
	Start,
	LeftTrigger,
	RightTrigger,
	LeftStickLeft,
	LeftStickRight,
	LeftStickUp,
	LeftStickDown,
	RightStickLeft,
	RightStickRight,
	RightStickUp,
	RightStickDown
};

enum class DS4Inputs
{
	Square,
	X,
	Circle,
	Triangle,
	L1,
	R1,
	L3,
	R3,
	Options,
	Share,
	PS,
	TouchPadButton,
	LeftStickLeft,
	LeftStickRight,
	LeftStickUp,
	LeftStickDown,
	RightStickLeft,
	RightStickRight,
	RightStickUp,
	RightStickDown,
	L2,
	R2,
	DpadLeft,
	DpadRight,
	DpadUp,
	DpadDown
};

enum class DS5Inputs
{
	Square,
	X,
	Circle,
	Triangle,
	L1,
	R1,
	L3,
	R3,
	Options,
	Share,
	PS,
	TouchPadButton,
	LeftStickLeft,
	LeftStickRight,
	LeftStickUp,
	LeftStickDown,
	RightStickLeft,
	RightStickRight,
	RightStickUp,
	RightStickDown,
	L2,
	R2,
	DpadLeft,
	DpadRight,
	DpadUp,
	DpadDown,
	MicrophoneButton,
};

enum class GenericInputs
{
	Button0,
	Button1,
	Button2,
	Button3,
	Button4,
	Button5,
	Button6,
	Button7,
	Button8,
	Button9,
	Button10,
	Button11,
	Button12,
	Button13,
	Button14,
	Button15,
	Button16,
	Button17,
	Button18,
	Button19,
	Button20,
	Button21,
	Button22,
	Button23,
	Button24,
	Button25,
	Button26,
	Button27,
	Button28,
	Button29,
	Button30,
	Button31,
	Axis0Positive,
	Axis0Negative,
	Axis1Positive,
	Axis1Negative,
	Axis2Positive,
	Axis2Negative,
	Axis3Positive,
	Axis3Negative,
	Axis4Positive,
	Axis4Negative,
	Axis5Positive,
	Axis5Negative,
	Axis6Positive,
	Axis6Negative,
	Axis7Positive,
	Axis7Negative,
	HatLeft,
	HatRight,
	HatUp,
	HatDown,
};

Joysticks joysticks = {};

bool IsXboxController(CW* deviceName)
{
	return wcsstr(deviceName, L"IG_");
}

bool IsDualshock4(RID_DEVICE_INFO_HID info)
{
	static const UL32 sonyVendorID = 0x054C;
	static const UL32 ds4Gen1ProductID = 0x05C4;
	static const UL32 ds4Gen2ProductID = 0x09CC;

	return info.dwVendorId == sonyVendorID && (info.dwProductId == ds4Gen1ProductID || info.dwProductId == ds4Gen2ProductID);
}

bool IsDualsense(RID_DEVICE_INFO_HID info)
{
	static const UL32 sonyVendorID = 0x054C;
	static const UL32 dualsenseProductID = 0x0CE6;
	static const UL32 dualsenseEdgeProductID = 0x0DF2;

	return info.dwVendorId == sonyVendorID && (info.dwProductId == dualsenseProductID || info.dwProductId == dualsenseEdgeProductID);
}

U32 MakeReflectedCRC32(BYTE* data, U32 byteCount)
{
	static const U32 crcTable[] = {
		0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,
		0x7EB17CBD,0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,
		0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,
		0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,
		0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,0x76DC4190,
		0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,
		0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
		0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
		0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,
		0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,
		0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,
		0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,
		0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,
		0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
		0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,
		0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
		0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,
		0xF862AE69,0x616BFFD3,0x166CCF45,0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,
		0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,
		0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
	};

	U32 crc = ~0;
	while (byteCount--)
	{
		crc = (crc >> 8) ^ crcTable[(U8)crc ^ *data];
		++data;
	}
	return ~crc;
}

void UpdateDualshock4(JoystickState* state, U8 rawData[], UL32 byteCount)
{
	static const UL32 usbInputByteCount = 64;
	static const UL32 bluetoothInputByteCount = 547;

	U32 offset = 0;
	if (byteCount == bluetoothInputByteCount) { offset = 2; }
	else if (byteCount != usbInputByteCount) { return; }

	U8 leftStickX = *(rawData + offset + 1);
	U8 leftStickY = *(rawData + offset + 2);
	U8 rightStickX = *(rawData + offset + 3);
	U8 rightStickY = *(rawData + offset + 4);
	U8 leftTrigger = *(rawData + offset + 8);
	U8 rightTrigger = *(rawData + offset + 9);
	U8 dpad = 0b1111 & *(rawData + offset + 5);

	state->currentInputs[*DS4Inputs::Square] = (F32)(1 & (*(rawData + offset + 5) >> 4));
	state->currentInputs[*DS4Inputs::X] = (F32)(1 & (*(rawData + offset + 5) >> 5));
	state->currentInputs[*DS4Inputs::Circle] = (F32)(1 & (*(rawData + offset + 5) >> 6));
	state->currentInputs[*DS4Inputs::Triangle] = (F32)(1 & (*(rawData + offset + 5) >> 7));
	state->currentInputs[*DS4Inputs::L1] = (F32)(1 & (*(rawData + offset + 6) >> 0));
	state->currentInputs[*DS4Inputs::R1] = (F32)(1 & (*(rawData + offset + 6) >> 1));
	state->currentInputs[*DS4Inputs::Share] = (F32)(1 & (*(rawData + offset + 6) >> 4));
	state->currentInputs[*DS4Inputs::Options] = (F32)(1 & (*(rawData + offset + 6) >> 5));
	state->currentInputs[*DS4Inputs::L3] = (F32)(1 & (*(rawData + offset + 6) >> 6));
	state->currentInputs[*DS4Inputs::R3] = (F32)(1 & (*(rawData + offset + 6) >> 7));
	state->currentInputs[*DS4Inputs::PS] = (F32)(1 & (*(rawData + offset + 7) >> 0));
	state->currentInputs[*DS4Inputs::TouchPadButton] = (F32)(1 & (*(rawData + offset + 7) >> 1));
	state->currentInputs[*DS4Inputs::LeftStickLeft] = -(leftStickX / 255.0f * 2 - 1);
	state->currentInputs[*DS4Inputs::LeftStickRight] = (leftStickX / 255.0f * 2 - 1);
	state->currentInputs[*DS4Inputs::LeftStickUp] = -(leftStickY / 255.0f * 2 - 1);
	state->currentInputs[*DS4Inputs::LeftStickDown] = (leftStickY / 255.0f * 2 - 1);
	state->currentInputs[*DS4Inputs::RightStickLeft] = -(rightStickX / 255.0f * 2 - 1);
	state->currentInputs[*DS4Inputs::RightStickRight] = (rightStickX / 255.0f * 2 - 1);
	state->currentInputs[*DS4Inputs::RightStickUp] = -(rightStickY / 255.0f * 2 - 1);
	state->currentInputs[*DS4Inputs::RightStickDown] = (rightStickY / 255.0f * 2 - 1);
	state->currentInputs[*DS4Inputs::L2] = leftTrigger / 255.0f;
	state->currentInputs[*DS4Inputs::R2] = rightTrigger / 255.0f;
	state->currentInputs[*DS4Inputs::DpadUp] = (dpad == 0 || dpad == 1 || dpad == 7) ? 1.0f : 0.0f;
	state->currentInputs[*DS4Inputs::DpadRight] = (dpad == 1 || dpad == 2 || dpad == 3) ? 1.0f : 0.0f;
	state->currentInputs[*DS4Inputs::DpadDown] = (dpad == 3 || dpad == 4 || dpad == 5) ? 1.0f : 0.0f;
	state->currentInputs[*DS4Inputs::DpadLeft] = (dpad == 5 || dpad == 6 || dpad == 7) ? 1.0f : 0.0f;
	state->type = JoystickType::Dualshock4;

	I32 headerSize = 0;
	I32 outputByteCount = 0;
	if (byteCount == usbInputByteCount)
	{
		outputByteCount = 32;
		state->outputBuffer[0] = 0x05;
		state->outputBuffer[1] = 0xFF;
	}
	if (byteCount == bluetoothInputByteCount)
	{
		outputByteCount = 78;
		state->outputBuffer[0] = 0x11;
		state->outputBuffer[1] = 0XC0;
		state->outputBuffer[3] = 0x07;
		headerSize = 1;
	}
	state->outputBuffer[4 + offset + headerSize] = (U8)(state->lightRumble * 0xFF);
	state->outputBuffer[5 + offset + headerSize] = (U8)(state->heavyRumble * 0xFF);
	state->outputBuffer[6 + offset + headerSize] = (U8)(state->ledRed * 0xFF);
	state->outputBuffer[7 + offset + headerSize] = (U8)(state->ledGreen * 0xFF);
	state->outputBuffer[8 + offset + headerSize] = (U8)(state->ledBlue * 0xFF);

	if (byteCount == bluetoothInputByteCount)
	{
		U32 crc = MakeReflectedCRC32(state->outputBuffer, 75);
		memcpy(state->outputBuffer + 75, &crc, sizeof(crc));
	}

	UL32 bytesTransferred;
	if (GetOverlappedResult(state->outputFile, &state->overlapped, &bytesTransferred, false))
	{
		if (state->outputFile != INVALID_HANDLE_VALUE)
		{
			WriteFile(state->outputFile, (void*)(state->outputBuffer + headerSize), outputByteCount, 0, &state->overlapped);
		}
	}
}

void SetDualsenseTriggerEffect(BYTE* dst, TriggerEffect effect)
{
	if (effect.type == TriggerEffectType::Weapon)
	{
		//TODO: Replace with Math
		U16 startAndEndZones = (1 << (U8)roundf(effect.weapon.startPosition * 8)) | (1 << (U8)roundf(effect.weapon.endPosition * 8));
		dst[0] = 0x25;
		*((U16*)(&dst[1])) = startAndEndZones;
		dst[3] = (U8)roundf(effect.weapon.strength * 7);
	}
	if (effect.type == TriggerEffectType::Resistance)
	{
		U32 forceZones = 0;
		U16 activeZones = 0;
		for (U32 i = 0; i < 10; ++i)
		{
			if (effect.vibration.zoneStrengths[i] > 0)
			{
				activeZones |= 1 << i;
				forceZones |= ((U8)roundf(effect.vibration.zoneStrengths[i] * 7)) << (3 * i);
			}
		}
		dst[0] = 0x21;
		*((U16*)(&dst[1])) = activeZones;
		*((U32*)(&dst[3])) = forceZones;
	}
	if (effect.type == TriggerEffectType::Vibration)
	{
		U32 forceZones = 0;
		U16 activeZones = 0;
		for (U32 i = 0; i < 10; ++i)
		{
			if (effect.vibration.zoneStrengths[i] > 0)
			{
				activeZones |= 1 << i;
				forceZones |= ((U8)roundf(effect.vibration.zoneStrengths[i] * 7)) << (3 * i);
			}
		}
		dst[0] = 0x26;
		*((U16*)(&dst[1])) = activeZones;
		*((U32*)(&dst[3])) = forceZones;
		dst[9] = (U8)(effect.vibration.frequency * 255);
	}
}

void UpdateDualsense(JoystickState* state, U8 rawData[], UL32 byteCount)
{
	bool bluetooth = rawData[0] == 0x31;
	if (rawData[0] != 0x01 && rawData[0] != 0x31) { return; }

	U32 offset = (bluetooth ? 2 : 0);
	state->currentInputs[*DS5Inputs::LeftStickLeft] = -(rawData[1 + offset] / 255.0f * 2 - 1);
	state->currentInputs[*DS5Inputs::LeftStickRight] = +(rawData[1 + offset] / 255.0f * 2 - 1);
	state->currentInputs[*DS5Inputs::LeftStickUp] = -(rawData[2 + offset] / 255.0f * 2 - 1);
	state->currentInputs[*DS5Inputs::LeftStickDown] = +(rawData[2 + offset] / 255.0f * 2 - 1);
	state->currentInputs[*DS5Inputs::RightStickLeft] = -(rawData[3 + offset] / 255.0f * 2 - 1);
	state->currentInputs[*DS5Inputs::RightStickRight] = +(rawData[3 + offset] / 255.0f * 2 - 1);
	state->currentInputs[*DS5Inputs::RightStickUp] = -(rawData[4 + offset] / 255.0f * 2 - 1);
	state->currentInputs[*DS5Inputs::RightStickDown] = +(rawData[4 + offset] / 255.0f * 2 - 1);
	state->currentInputs[*DS5Inputs::L2] = rawData[5 + offset] / 255.0f;
	state->currentInputs[*DS5Inputs::R2] = rawData[6 + offset] / 255.0f;
	I32 hat = rawData[8 + offset] & 0x0F;
	state->currentInputs[*DS5Inputs::DpadLeft] = hat == 5 || hat == 6 || hat == 7;
	state->currentInputs[*DS5Inputs::DpadRight] = hat == 1 || hat == 2 || hat == 3;
	state->currentInputs[*DS5Inputs::DpadUp] = hat == 7 || hat == 0 || hat == 1;
	state->currentInputs[*DS5Inputs::DpadDown] = hat == 3 || hat == 4 || hat == 5;
	state->currentInputs[*DS5Inputs::Square] = (F32)(rawData[8 + offset] & 0x10);
	state->currentInputs[*DS5Inputs::X] = (F32)(rawData[8 + offset] & 0x20);
	state->currentInputs[*DS5Inputs::Circle] = (F32)(rawData[8 + offset] & 0x40);
	state->currentInputs[*DS5Inputs::Triangle] = (F32)(rawData[8 + offset] & 0x80);
	state->currentInputs[*DS5Inputs::L1] = (F32)(rawData[9 + offset] & 0x01);
	state->currentInputs[*DS5Inputs::R1] = (F32)(rawData[9 + offset] & 0x02);
	state->currentInputs[*DS5Inputs::Share] = (F32)(rawData[9 + offset] & 0x10);
	state->currentInputs[*DS5Inputs::Options] = (F32)(rawData[9 + offset] & 0x20);
	state->currentInputs[*DS5Inputs::L3] = (F32)(rawData[9 + offset] & 0x40);
	state->currentInputs[*DS5Inputs::R3] = (F32)(rawData[9 + offset] & 0x80);
	state->currentInputs[*DS5Inputs::PS] = (F32)(rawData[10 + offset] & 0x01);
	state->currentInputs[*DS5Inputs::TouchPadButton] = (F32)(rawData[10 + offset] & 0x02);
	state->currentInputs[*DS5Inputs::MicrophoneButton] = (F32)(rawData[10 + offset] & 0x04);
	state->type = JoystickType::Dualsense;

	memset(state->outputBuffer, 0, sizeof(state->outputBuffer));
	I32 headerSize = 0;
	I32 outputByteCount = 0;
	if (bluetooth)
	{
		offset = 2;
		headerSize = 1;
		outputByteCount = 78;
		state->outputBuffer[0] = 0xA2;
		state->outputBuffer[1] = 0x31;
		state->outputBuffer[2] = 0x02;
	}
	else
	{
		offset = 1;
		headerSize = 0;
		outputByteCount = 48;
		state->outputBuffer[0] = 0x02;
	}

	// Enable rumble, disable audio haptics, enable right and left trigger effect
	state->outputBuffer[0 + offset + headerSize] |= 0x0F;
	// Enable LED color
	state->outputBuffer[1 + offset + headerSize] |= 0x04;
	// Enable rumble low pass filter
	state->outputBuffer[38 + offset + headerSize] |= 0x04;

	state->outputBuffer[2 + offset + headerSize] = (BYTE)(state->lightRumble * 0xFF);
	state->outputBuffer[3 + offset + headerSize] = (BYTE)(state->heavyRumble * 0xFF);

	SetDualsenseTriggerEffect(state->outputBuffer + 10 + offset + headerSize, state->rightTriggerEffect);
	SetDualsenseTriggerEffect(state->outputBuffer + 21 + offset + headerSize, state->leftTriggerEffect);

	state->outputBuffer[44 + offset + headerSize] = (BYTE)(state->ledRed * 0xFF);
	state->outputBuffer[45 + offset + headerSize] = (BYTE)(state->ledGreen * 0xFF);
	state->outputBuffer[46 + offset + headerSize] = (BYTE)(state->ledBlue * 0xFF);

	if (bluetooth)
	{
		U32 crc = MakeReflectedCRC32(state->outputBuffer, 74);
		memcpy(state->outputBuffer + 74, &crc, sizeof(crc));
	}

	UL32 bytesTransferred;
	if (GetOverlappedResult(state->outputFile, &state->overlapped, &bytesTransferred, false))
	{
		if (state->outputFile != INVALID_HANDLE_VALUE)
		{
			WriteFile(state->outputFile, (void*)(state->outputBuffer + headerSize), outputByteCount, 0, &state->overlapped);
		}
	}
}

void ParseGenericController(JoystickState* out, U8 rawData[], UL32 dataSize, _HIDP_PREPARSED_DATA* preparsedData)
{
	memset(out->currentInputs, 0, sizeof(out->currentInputs));

	HIDP_CAPS caps;
	HidP_GetCaps(preparsedData, &caps);

	HIDP_VALUE_CAPS* valueCaps;
	Memory::Allocate(&valueCaps, caps.NumberInputValueCaps);
	HidP_GetValueCaps(HidP_Input, valueCaps, &caps.NumberInputValueCaps, preparsedData);
	for (U32 i = 0; i < caps.NumberInputValueCaps; ++i)
	{
		UL32 value;
		NTSTATUS status = HidP_GetUsageValue(HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &value, preparsedData, (PCHAR)rawData, dataSize);
		F32 maxValue = (F32)(1 << (valueCaps[i].BitSize)) - 1;
		F32 normalizedValue = (value / maxValue) * 2 - 1;
		U32 usage = valueCaps[i].Range.UsageMin;
		if (usage >= 0x30 && usage <= 0x37)
		{
			int axisIndex = usage - 0x30;
			out->currentInputs[*GenericInputs::Axis0Positive + 2 * axisIndex] = normalizedValue;
			out->currentInputs[*GenericInputs::Axis0Negative + 2 * axisIndex] = -normalizedValue;
		}
		if (usage == 0x39)
		{
			L32 hat = value - valueCaps[i].LogicalMin;
			out->currentInputs[*GenericInputs::HatUp] = (hat == 0 || hat == 1 || hat == 7) ? 1.0f : 0.1f;
			out->currentInputs[*GenericInputs::HatRight] = (hat == 1 || hat == 2 || hat == 3) ? 1.0f : 0.1f;
			out->currentInputs[*GenericInputs::HatDown] = (hat == 3 || hat == 4 || hat == 5) ? 1.0f : 0.1f;
			out->currentInputs[*GenericInputs::HatLeft] = (hat == 5 || hat == 6 || hat == 7) ? 1.0f : 0.1f;
		}
	}
	Memory::Free(&valueCaps);

	HIDP_BUTTON_CAPS* buttonCaps;
	Memory::Allocate(&buttonCaps, caps.NumberInputButtonCaps);
	HidP_GetButtonCaps(HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, preparsedData);
	for (U32 i = 0; i < caps.NumberInputButtonCaps; ++i)
	{
		U32 buttonCount = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
		USAGE* usages;
		Memory::Allocate(&usages, buttonCount);
		HidP_GetUsages(HidP_Input, buttonCaps[i].UsagePage, 0, usages, (PULONG)&buttonCount, preparsedData, (PCHAR)rawData, dataSize);
		for (U32 usagesIndex = 0; usagesIndex < buttonCount; ++usagesIndex)
		{
			U32 buttonIndex = usages[usagesIndex] - 1;
			if (buttonIndex < 32) out->currentInputs[*GenericInputs::Button0 + buttonIndex] = 1.0f;
		}
		Memory::Free(&usages);
	}

	Memory::Free(&buttonCaps);
	out->type = JoystickType::Generic;
}

void ConnectHIDJoystick(HANDLE deviceHandle)
{
	WCHAR deviceName[1024] = { 0 };
	UINT deviceNameLength = sizeof(deviceName) / sizeof(*deviceName);
	GetRawInputDeviceInfoW(deviceHandle, RIDI_DEVICENAME, deviceName, &deviceNameLength);
	if (!IsXboxController(deviceName))
	{
		U32 joystickIndex = Joysticks::maxXinputControllers;
		while (joystickIndex < joysticks.count && wcscmp(deviceName, joysticks.states[joystickIndex].deviceName) != 0)
		{
			++joystickIndex;
		}
		if (joystickIndex == joysticks.count)
		{
			joysticks.count += 1;
			Memory::Reallocate(&joysticks.states, joysticks.count);
			JoystickState newState = { 0 };
			joysticks.states[joystickIndex] = newState;
		}

		JoystickState* state = &joysticks.states[joystickIndex];
		state->deviceHandle = deviceHandle;
		wcscpy_s(state->deviceName, deviceName);
		state->outputFile = CreateFileW(deviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		HidD_GetProductString(state->outputFile, state->productName, JoystickState::maxNameLength);
		HidD_GetManufacturerString(state->outputFile, state->manufacturerName, JoystickState::maxNameLength);
		joysticks.states[joystickIndex].connected = true;
	}
}

void DisconnectHIDJoystick(HANDLE deviceHandle)
{
	for (uint32_t i = Joysticks::maxXinputControllers; i < joysticks.count; ++i)
	{
		if (deviceHandle == joysticks.states[i].deviceHandle && !IsXboxController(joysticks.states[i].deviceName))
		{
			JoystickState* state = &joysticks.states[i];
			state->connected = false;
			if (state->outputFile != INVALID_HANDLE_VALUE)
			{
				DWORD bytesTransferred;
				GetOverlappedResult(state->outputFile, &state->overlapped, &bytesTransferred, true);
				CloseHandle(state->outputFile);
			}
		}
	}
}

bool Input::Initialize()
{
	Logger::Trace("Initializing Input...");

	WindowInfo info = Platform::GetWindowInfo();

	RAWINPUTDEVICE deviceList[2];
	deviceList[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	deviceList[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
	deviceList[0].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
	deviceList[0].hwndTarget = info.window;
	deviceList[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
	deviceList[1].usUsage = HID_USAGE_GENERIC_GAMEPAD;
	deviceList[1].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
	deviceList[1].hwndTarget = info.window;

	if (!RegisterRawInputDevices(deviceList, 2, sizeof(RAWINPUTDEVICE))) { return false; }

	joysticks.hwnd = info.window;
	joysticks.count = Joysticks::maxXinputControllers;
	Memory::Allocate(&joysticks.states, joysticks.count);
	for (U32 i = 0; i < Joysticks::maxXinputControllers; ++i) { joysticks.states[i].type = JoystickType::Xbox; }

	return true;
}

void Input::Shutdown()
{
	Logger::Trace("Cleaning Up Input...");

	Memory::Free(&joysticks.states);
}

void Input::Reset()
{
	for (U32 i = 0; i < joysticks.count; ++i)
	{
		memcpy(joysticks.states[i].previousInputs, joysticks.states[i].currentInputs, sizeof(joysticks.states[i].previousInputs));
	}
}

void Input::Update()
{
	// Xbox controllers
	for (U32 playerIndex = 0; playerIndex < Joysticks::maxXinputControllers; ++playerIndex)
	{
		JoystickState* state = &joysticks.states[playerIndex];
		XINPUT_STATE xinput;
		if (state->connected && XInputGetState(playerIndex, &xinput) == ERROR_SUCCESS)
		{
			state->currentInputs[*XboxInputs::A] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::B] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::X] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::Y] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::DpadLeft] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::DpadRight] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::DpadUp] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::DpadDown] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::LeftBumper] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::RightBumper] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::LeftStickButton] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::RightStickButton] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::Back] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::Start] = (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 1.0f : 0.0f;
			state->currentInputs[*XboxInputs::LeftTrigger] = xinput.Gamepad.bLeftTrigger / 255.0f;
			state->currentInputs[*XboxInputs::RightTrigger] = xinput.Gamepad.bRightTrigger / 255.0f;
			state->currentInputs[*XboxInputs::LeftStickLeft] = -xinput.Gamepad.sThumbLX / 32767.0f;
			state->currentInputs[*XboxInputs::LeftStickRight] = xinput.Gamepad.sThumbLX / 32767.0f;
			state->currentInputs[*XboxInputs::LeftStickUp] = xinput.Gamepad.sThumbLY / 32767.0f;
			state->currentInputs[*XboxInputs::LeftStickDown] = -xinput.Gamepad.sThumbLY / 32767.0f;
			state->currentInputs[*XboxInputs::RightStickLeft] = -xinput.Gamepad.sThumbRX / 32767.0f;
			state->currentInputs[*XboxInputs::RightStickRight] = xinput.Gamepad.sThumbRX / 32767.0f;
			state->currentInputs[*XboxInputs::RightStickUp] = xinput.Gamepad.sThumbRY / 32767.0f;
			state->currentInputs[*XboxInputs::RightStickDown] = -xinput.Gamepad.sThumbRY / 32767.0f;

			XINPUT_VIBRATION vibration;
			vibration.wLeftMotorSpeed = (WORD)(state->lightRumble * 0xFFFF);
			vibration.wRightMotorSpeed = (WORD)(state->heavyRumble * 0xFFFF);
			XInputSetState(playerIndex, &vibration);
		}
		else
		{
			state->connected = false;
		}
	}

	// Clamp negative inputs to 0
	for (U32 joystickIndex = 0; joystickIndex < joysticks.count; ++joystickIndex)
	{
		for (U32 inputIndex = 0; inputIndex < JoystickState::inputCount; ++inputIndex)
		{
			float* input = &joysticks.states[joystickIndex].currentInputs[inputIndex];
			if (*input < 0) *input = 0;
		}
	}

	for (U32 joystickIndex = 0; joystickIndex < joysticks.count; ++joystickIndex)
	{
		JoystickState* state = &joysticks.states[joystickIndex];
		for (U32 inputIndex = 0; inputIndex < state->inputCount; ++inputIndex)
		{
			//if (state->currentInputs[inputIndex] > 0.5 && state->previousInputs[inputIndex] <= 0.5f)
			//{
			//	//const C8* inputName = getInputName(joysticks, joystickIndex, inputIndex);
			//
			//	if (joystickIndex >= 0 && joystickIndex <= 3)
			//	{
			//		printf("XBox controller %d: ", joystickIndex);
			//	}
			//	else
			//	{
			//		wprintf(L"%s by %s: ", state->productName, state->manufacturerName);
			//	}
			//
			//	//printf("%s\n", inputName);
			//}

			F32 rumbleLeft = 0;
			F32 rumbleRight = 0;

			if (state->type == JoystickType::Xbox)
			{
				state->lightRumble = state->currentInputs[*XboxInputs::LeftTrigger];
				state->heavyRumble = state->currentInputs[*XboxInputs::RightTrigger];
			}
			else if (state->type == JoystickType::Dualshock4)
			{
				state->heavyRumble = state->currentInputs[*DS4Inputs::L2];
				state->lightRumble = state->currentInputs[*DS4Inputs::R2];
				state->ledRed = (state->currentInputs[*DS4Inputs::Circle] || state->currentInputs[*DS4Inputs::Square]) ? 1.0f : 0.0f;
				state->ledGreen = state->currentInputs[*DS4Inputs::Triangle];
				state->ledBlue = (state->currentInputs[*DS4Inputs::X] || state->currentInputs[*DS4Inputs::Square]) ? 1.0f : 0.0f;
			}
			else if (state->type == JoystickType::Dualsense)
			{
				state->heavyRumble = state->currentInputs[*DS5Inputs::LeftStickUp] + state->currentInputs[*DS5Inputs::LeftStickDown];
				state->lightRumble = state->currentInputs[*DS5Inputs::LeftStickLeft] + state->currentInputs[*DS5Inputs::LeftStickRight];
				state->ledRed = (state->currentInputs[*DS5Inputs::Circle] || state->currentInputs[*DS5Inputs::Square]) ? 1.0f : 0.0f;
				state->ledGreen = state->currentInputs[*DS5Inputs::Triangle];
				state->ledBlue = (state->currentInputs[*DS5Inputs::X] || state->currentInputs[*DS5Inputs::Square]) ? 1.0f : 0.0f;

				if (state->currentInputs[*DS5Inputs::Square] > 0.5f)
				{
					state->rightTriggerEffect = {
						.type = TriggerEffectType::Resistance,
						.resistance = {
							.zoneStrengths = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 1.0f}
						}
					};
				}
				else if (state->currentInputs[*DS5Inputs::Triangle] > 0.5f)
				{
					state->rightTriggerEffect.type = TriggerEffectType::Weapon;
					state->rightTriggerEffect.weapon.startPosition = 0.2f;
					state->rightTriggerEffect.weapon.endPosition = 0.4f;
					state->rightTriggerEffect.weapon.strength = 0.8f;
				}
				else if (state->currentInputs[*DS5Inputs::Circle] > 0.5f)
				{
					state->rightTriggerEffect = {
						.type = TriggerEffectType::Vibration,
						.vibration = {
							.zoneStrengths = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 1.0f},
							.frequency = 0.3f
						}
					};
				}
				else
				{
					state->rightTriggerEffect.type = TriggerEffectType::None;
				}

				if (state->currentInputs[*DS5Inputs::DpadLeft] > 0.5f)
				{
					state->leftTriggerEffect = {
						.type = TriggerEffectType::Resistance,
						.resistance = {
							.zoneStrengths = {1.0f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, 0.0f}
						}
					};
				}
				else if (state->currentInputs[*DS5Inputs::DpadUp] > 0.5f)
				{
					state->leftTriggerEffect.type = TriggerEffectType::Weapon;
					state->leftTriggerEffect.weapon.startPosition = 0.7f;
					state->leftTriggerEffect.weapon.endPosition = 0.9f;
					state->leftTriggerEffect.weapon.strength = 0.2f;
				}
				else if (state->currentInputs[*DS5Inputs::DpadRight] > 0.5f)
				{
					state->leftTriggerEffect = {
						.type = TriggerEffectType::Vibration,
						.vibration = {
							.zoneStrengths = {1.0f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, 0.0f},
							.frequency = 0.8f
						}
					};
				}
				else
				{
					state->leftTriggerEffect.type = TriggerEffectType::None;
				}
			}

			if (state->lightRumble < 0.1) state->lightRumble = 0;
			if (state->heavyRumble < 0.1) state->heavyRumble = 0;
		}
	}
}

void Input::UpdateRawInput(I64 lParam)
{
	U32 size = 0;
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
	RAWINPUT* input;
	Memory::Allocate(&input, size / sizeof(RAWINPUTHEADER));
	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER)) > 0)
	{
		RID_DEVICE_INFO deviceInfo;
		UINT deviceInfoSize = sizeof(deviceInfo);
		bool gotInfo = GetRawInputDeviceInfo(input->header.hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize) > 0;

		GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, 0, &size);
		_HIDP_PREPARSED_DATA* data;
		Memory::Allocate((U8**)&data, size);

		bool gotPreparsedData = GetRawInputDeviceInfo(input->header.hDevice, RIDI_PREPARSEDDATA, data, &size) > 0;

		if (gotInfo && gotPreparsedData)
		{
			for (UINT i = Joysticks::maxXinputControllers; i < joysticks.count; ++i)
			{
				if (input->header.hDevice == joysticks.states[i].deviceHandle)
				{
					JoystickState* state = &joysticks.states[i];
					if (IsDualshock4(deviceInfo.hid))
					{
						UpdateDualshock4(state, input->data.hid.bRawData, input->data.hid.dwSizeHid);
					}
					else if (IsDualsense(deviceInfo.hid))
					{
						UpdateDualsense(state, input->data.hid.bRawData, input->data.hid.dwSizeHid);
					}
					else
					{
						ParseGenericController(state, input->data.hid.bRawData, input->data.hid.dwSizeHid, data);
					}
				}
			}
		}
		Memory::Free(&data);
	}
	Memory::Free(&input);
}

void Input::UpdateConnectionStatus(void* deviceHandle, U64 status)
{
	// Check all XInput devices
	for (U32 playerIndex = 0; playerIndex < Joysticks::maxXinputControllers; ++playerIndex)
	{
		XINPUT_STATE state;
		joysticks.states[playerIndex].connected = (XInputGetState(playerIndex, &state) == ERROR_SUCCESS);
	}

	if (status == GIDC_ARRIVAL) { ConnectHIDJoystick(deviceHandle); }
	else if (status == GIDC_REMOVAL) { DisconnectHIDJoystick(deviceHandle); }
}

#endif