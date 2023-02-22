#pragma once

#include "Devices.hpp"
#include "Containers\String.hpp"
#include "Containers\WString.hpp"

struct Keyboard
{
	Keyboard(void* handle);
	~Keyboard();
	void Destroy();

	void* dHandle;				//HANDLE
	void* ntHandle;				//HANDLE
	bool openHandle;
	WString manufacturer;
	WString product;
	void* inputReportProtocol;	//PHIDP_PREPARSED_DATA
	HIDCapabilities capabilities;
	U64 inputReportSize;

	friend class Input;
};