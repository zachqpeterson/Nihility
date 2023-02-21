#pragma once

#include "Devices.hpp"

#include "Containers\String.hpp"
#include "Containers\WString.hpp"

struct Controller
{
private:
	Controller(void* handle);
	~Controller();
	void Destroy();

	void* dHandle;				//HANDLE
	void* ntHandle;				//HANDLE
	bool openHandle;
	String path;
	WString manufacturer;
	WString product;
	void* inputReportProtocol;	//PHIDP_PREPARSED_DATA
	Capabilities capabilities;
	U64 inputReportSize;

	friend class Input;
};