#pragma once

#include "Devices.hpp"

#include "Containers\String.hpp"
#include "Containers\WString.hpp"

struct Controller
{
public:


private:
	void* dHandle;				//HANDLE
	void* ntHandle;				//HANDLE
	String path;
	WString manufacturer;
	WString product;
	void* inputReportProtocol;	//PHIDP_PREPARSED_DATA
	Capabilities caps;
	U64 inputReportSize;
};