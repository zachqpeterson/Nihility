#pragma once

#include "Defines.hpp"
#include "Containers\String.hpp"

enum DeviceType
{
	DEVICE_TYPE_MOUSE,
	DEVICE_TYPE_KEYBOARD,
	DEVICE_TYPE_CONTROLLER,

	DEVICE_TYPE_COUNT
};

struct HIDCapabilities
{
	U16 Usage;
	U16 UsagePage;
	U16 InputReportByteLength;
	U16 OutputReportByteLength;
	U16 FeatureReportByteLength;
	U16 Reserved[17];

	U16 NumberLinkCollectionNodes;

	U16 NumberInputButtonCaps;
	U16 NumberInputValueCaps;
	U16 NumberInputDataIndices;

	U16 NumberOutputButtonCaps;
	U16 NumberOutputValueCaps;
	U16 NumberOutputDataIndices;

	U16 NumberFeatureButtonCaps;
	U16 NumberFeatureValueCaps;
	U16 NumberFeatureDataIndices;
};

struct HIDCalibration
{
	L32 lMin;
	L32 lCenter;
	L32 lMax;
};

struct HIDAttributes {
	UL32 dwFlags;
	U16 wUsagePage;
	U16 wUsage;
};

struct HIDAxis
{
	U16 usagePage;
	U16 usage;
	U16 index;
	String name;

	I32 value;
	I32 logicalMinimum;
	I32 logicalMaximum;
	I32 logicalCalibratedMinimum;
	I32 logicalCalibratedMaximum;
	I32 logicalCalibratedCenter;
	F32 physicalMinimum;
	F32 physicalMaximum;
	bool isCalibrated;
};

struct HIDButton
{
	U16 usagePage;
	U16 usage;
	U16 index;
	String name;

	I32 value;
};

struct HIDInfo
{

};

struct HIDAxisMapping
{
	U16 usagePage;
	U16 usage;
	bool isCalibrated;
	HIDCalibration calibration;
	String name;
};

struct HIDButtonMapping
{
	U16 usagePage;
	U16 usage;
	String name;
};

struct Overlapped {
	U64 Internal;
	U64 InternalHigh;
	union {
		struct {
			UL32 Offset;
			UL32 OffsetHigh;
		};
		void* Pointer;
	};

	void* hEvent;
};

struct _HIDP_PREPARSED_DATA;
struct _HIDP_DATA;

struct Device
{
public:
	Device(WString path);
	Device(Device&& other) noexcept;
	Device& operator=(Device&& other) noexcept;
	~Device();
	void Destroy();

	void Update();

	bool openHandle;

private:
	bool SetupMouse();
	bool SetupKeyboard();
	bool SetupController();

	void __stdcall DeviceRead(UL32 dwErrorCode, UL32 dwNumberOfBytesTransfered, struct _OVERLAPPED* lpOverlapped);

	WString path;
	void* ntHandle;				//HANDLE
	WString manufacturer;
	WString product;
	DeviceType type;

	HIDCapabilities capabilities;
	_HIDP_PREPARSED_DATA* preparsedData;
	U32 preparsedDataSize;
	_HIDP_DATA* stateBuffer;
	UL32 stateLength;
	char* reportBuffer;

	Overlapped overlap{};
	UL32 read;

	Vector<HIDAxis> axes;
	Vector<HIDButton> buttons;

	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
};