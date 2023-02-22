#pragma once

#include "Defines.hpp"
#include "Containers\String.hpp"

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