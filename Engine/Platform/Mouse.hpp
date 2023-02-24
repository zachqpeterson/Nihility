#pragma once

#include "Device.hpp"
#include "Containers\String.hpp"
#include "Containers\WString.hpp"
#include "Containers\Vector.hpp"

struct _HIDP_PREPARSED_DATA;
struct _HIDP_DATA;

struct Mouse
{
private:
	Mouse(void* handle);
	~Mouse();
	void Destroy();

	void Update();

	Device device;

	Vector<HIDAxis> axes;
	Vector<HIDButton> buttons;

	friend class Input;
};