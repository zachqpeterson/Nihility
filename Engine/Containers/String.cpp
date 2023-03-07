#include "String.hpp"
#include "WString.hpp"

String::String(const WString& other) : size{ other.Size() }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);
	const C16* w = other.Data();
	C8* c = string;

	C16 val;
	while ((val = *w++) != WString::NULL_CHAR)
	{
		if (val < 128) { *c++ = (C8)val; }
		else { *c++ = (C8)219; if (val >= 0xD800 && val <= 0xD8FF) { --size; ++w; } }
	}
}

String::String(WString&& other) noexcept : size{ other.Size() }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);
	const C16* w = other.Data();
	C8* c = string;

	C16 val;
	while ((val = *w++) != WString::NULL_CHAR)
	{
		if (val < 128) { *c++ = (C8)val; }
		else { *c++ = (C8)219; if (val >= 0xD800 && val <= 0xD8FF) { --size; ++w; } }
	}

	other.Destroy();
}

String& String::operator=(const WString& other)
{
	hashed = false;
	if (!other.Data()) { Destroy(); return *this; }

	size = other.Size();
	capacity = size;

	Memory::AllocateArray(&string, capacity);
	const C16* w = other.Data();
	C8* c = string;

	C16 val;
	while ((val = *w++) != WString::NULL_CHAR)
	{
		if (val < 128) { *c++ = (C8)val; }
		else { *c++ = (C8)219; if (val >= 0xD800 && val <= 0xD8FF) { --size; ++w; } }
	}

	return *this;
}

String& String::operator=(WString&& other) noexcept
{
	hashed = false;
	if (!other.Data()) { Destroy(); return *this; }

	size = other.Size();
	capacity = size;

	Memory::AllocateArray(&string, capacity);
	const C16* w = other.Data();
	C8* c = string;

	C16 val;
	while ((val = *w++) != WString::NULL_CHAR)
	{
		if (val < 128) { *c++ = (C8)val; }
		else { *c++ = (C8)219; if (val >= 0xD800 && val <= 0xD8FF) { --size; ++w; } }
	}

	other.Destroy();

	return *this;
}