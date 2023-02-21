#pragma once

#include "Defines.hpp"

struct Capabilities
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

struct Calibration
{
	L32 lMin;
	L32 lCenter;
	L32 lMax;
};