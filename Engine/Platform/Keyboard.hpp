#pragma once

#include "Device.hpp"
#include "Containers\String.hpp"
#include "Containers\WString.hpp"
#include "Containers\Vector.hpp"

struct Keyboard
{
private:
	Keyboard(void* handle);
	~Keyboard();
	void Destroy();

	void Update();

	Device device;

	Vector<HIDAxis> axes;
	Vector<HIDButton> buttons;

	friend class Input;
};