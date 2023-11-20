#pragma once

#include "Defines.hpp"
#include "Containers\String.hpp"
#include "Core\File.hpp"

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

struct Async {
	U64 internal;
	U64 internalHigh;
	union {
		struct {
			UL32 offset;
			UL32 offsetHigh;
		};
		void* pointer;
	};

	void* event;
};

struct _HIDP_PREPARSED_DATA;
struct _HIDP_DATA;

struct Device
{
public:
	Device(void* handle);
	Device(Device&& other) noexcept;
	Device& operator=(Device&& other) noexcept;
	~Device();
	void Destroy();

	U8* ReadInput(U32& size);

	bool valid{ false };

private:
	bool SetupMouse();
	bool SetupKeyboard();
	bool SetupController();

	void* riHandle;				//HRAWINPUT
	void* ntHandle{ nullptr };	//HANDLE
	String16 name;
	DeviceType type{ DEVICE_TYPE_COUNT };

	HIDCapabilities capabilities{};
	_HIDP_PREPARSED_DATA* preparsedData{ nullptr };
	U32 preparsedDataSize{ 0 };
	_HIDP_DATA* stateBuffer{ nullptr };
	UL32 stateLength{ 0 };
	U8* reportBuffer{ nullptr };

	Vector<HIDAxis> axes;
	Vector<HIDButton> buttons;

	Async async{};
	bool reading{ false };

	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
};