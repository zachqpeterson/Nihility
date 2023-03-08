#pragma once

#include "Device.hpp"
#include "Containers\String.hpp"
#include "Containers\Vector.hpp"

struct _HIDP_PREPARSED_DATA;
struct _HIDP_DATA;

struct Controller
{
private:
	Controller(void* handle);
	~Controller();
	void Destroy();

	void Update();

	bool Valid();

	void* dHandle;				//HANDLE
	void* ntHandle;				//HANDLE
	bool openHandle;
	WString manufacturer;
	WString product;
	_HIDP_PREPARSED_DATA* inputReportProtocol;
	HIDCapabilities capabilities;
	U64 inputReportSize;

	Vector<HIDAxis> axes;
	Vector<HIDButton> buttons;

	U8* inputBuffer;
	_HIDP_DATA* stateBuffer;

	friend class Input;
};

inline bool Controller::Valid() { return openHandle; }