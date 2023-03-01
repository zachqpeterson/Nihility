#include "String.hpp"
#include "WString.hpp"

String::String(const WString& other) : size{ other.Size() }, capacity{ other.Capacity() }, str{ (char*)Memory::Allocate(capacity) }
{
	const W16* w = other.Data();
	char* c = str;

	W16 val;
	while ((val = *w++) != WString::NULL_CHAR)
	{
		if (val < 128) { *c++ = (char)val; }
		else { *c++ = (char)219; if (val >= 0xD800 && val <= 0xD8FF) { --size; ++w; } }
	}
}