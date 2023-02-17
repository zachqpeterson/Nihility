#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"
#include "Vector.hpp"

struct NH_API WString
{
public:
	WString();
	WString(I8 value);
	WString(U8 value);
	WString(I16 value);
	WString(U16 value);
	WString(I32 value);
	WString(U32 value);
	WString(I64 value);
	WString(U64 value);
	WString(F32 value);
	WString(F64 value);
	WString(bool value);
	WString(W16* str);
	WString(const W16* str);
	WString(const WString& other);
	WString(WString&& other) noexcept;
	template<typename... Types> WString(const W16* fmt, const Types& ... args);
	template<typename... Types> WString(const Types& ... args);

	WString& operator=(I8 value);
	WString& operator=(U8 value);
	WString& operator=(I16 value);
	WString& operator=(U16 value);
	WString& operator=(I32 value);
	WString& operator=(U32 value);
	WString& operator=(I64 value);
	WString& operator=(U64 value);
	WString& operator=(F32 value);
	WString& operator=(F64 value);
	WString& operator=(bool value);
	WString& operator=(W16* str);
	WString& operator=(const W16* str);
	WString& operator=(const WString& other);
	WString& operator=(WString&& other) noexcept;

	~WString();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);

	I8  ToI8() const;
	U8  ToU8() const;
	I16 ToI16() const;
	U16 ToU16() const;
	I32 ToI32() const;
	U32 ToU32() const;
	I64 ToI64() const;
	U64 ToU64() const;
	F32 ToF32() const;
	F64 ToF64() const;
	bool ToBool() const;

	WString& operator+=(I8  value);
	WString& operator+=(U8  value);
	WString& operator+=(I16 value);
	WString& operator+=(U16 value);
	WString& operator+=(I32 value);
	WString& operator+=(U32 value);
	WString& operator+=(I64 value);
	WString& operator+=(U64 value);
	WString& operator+=(F32 value);
	WString& operator+=(F64 value);
	WString& operator+=(bool value);
	WString& operator+=(W16* other);
	WString& operator+=(const W16* other);
	WString& operator+=(const WString& other);

	explicit operator W16* ();
	explicit operator const W16* () const;
	explicit operator I8() const;
	explicit operator U8() const;
	explicit operator I16() const;
	explicit operator U16() const;
	explicit operator I32() const;
	explicit operator U32() const;
	explicit operator I64() const;
	explicit operator U64() const;
	explicit operator F32() const;
	explicit operator F64() const;
	explicit operator bool() const;

	W16* operator*();
	const W16* operator*() const;
	W16& operator[](U32 i);
	const W16& operator[](U32 i) const;

	bool operator==(W16* other) const;
	bool operator==(const W16* other) const;
	bool operator==(const WString& other) const;
	bool operator!=(W16* other) const;
	bool operator!=(const W16* other) const;
	bool operator!=(const WString& other) const;

	bool Compare(W16* other) const;
	bool Compare(const W16* other) const;
	bool Compare(const WString& other) const;
	bool CompareN(W16* other, U32 nLength, U32 start = 0) const;
	bool CompareN(const W16* other, U32 nLength, U32 start = 0) const;
	bool CompareN(const WString& other, U32 nLength, U32 start = 0) const;

	const U64& Size() const;
	const U64& Capacity() const;
	U64 Hash();
	W16* Data();
	const W16* Data() const;
	bool Blank() const;
	I32 IndexOf(const W16& c, U64 start = 0) const;
	I32 LastIndexOf(const W16& c, U64 start = 0) const;
	WString& Trim();
	WString& SubString(WString& newStr, U64 start, U64 nLength = I64_MAX) const;
	WString& Append(const WString& append);
	WString& Prepend(const WString& prepend);
	WString& Surround(const WString& prepend, const WString& append);
	WString& Insert(const WString& string, U32 i);
	WString& ReplaceAll(const WString& find, const WString& replace, U64 start = 0);
	WString& ReplaceN(const WString& find, const WString& replace, U64 count, U64 start = 0);
	WString& ReplaceFirst(const WString& find, const WString& replace, U64 start = 0);
	void Split(Vector<WString>& list, U8 delimiter, bool trimEntries) const;

	W16* begin();
	W16* end();
	const W16* begin() const;
	const W16* end() const;

	W16* rbegin();
	W16* rend();
	const W16* rbegin() const;
	const W16* rend() const;

private:
	void Format(U64& start, const WString& replace);

	bool hashed{ false };
	U64 hash{ 0 };
	U64 size{ 0 };
	U64 capacity{ 1024 };
	W16* str;
};

inline WString::WString() : str{ (W16*)Memory::Allocate1kb() } {}

inline WString::WString(I8 value) : str{ (W16*)Memory::Allocate1kb() }
{
	U32 i = L'0';

	if (value < 0)
	{
		str[0] = '-';
		U8 abs = (U8)-value;
		W16* c = str + 4;
		const W16* threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
		*--c = threeDigits[2];
		if (abs > 9) { *--c = threeDigits[1]; }
		if (abs > 99) { *--c = threeDigits[0]; }

		size = 5 - (c - str);
		memcpy(str + 1, c, size + 1);
	}
	else
	{
		W16* c = str + 3;
		const W16* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
		*--c = threeDigits[2];
		if (value > 9) { *--c = threeDigits[1]; }
		if (value > 99) { *--c = threeDigits[0]; }

		size = 3 - (c - str);
		memcpy(str, c, size + 1);
	}
}

inline WString::WString(U8 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 3;
	const W16* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	size = 3 - (c - str);
	memcpy(str, c, size + 1);
}

inline WString::WString(I16 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 6;
	const W16* threeDigits;
	U8 neg = 0;

	U16 abs = (U16)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U16)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U16 newVal = abs / 1000;
		U16 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	size = 6 + neg - (c - str);

	memcpy(str + neg, c, size + 1);
}

inline WString::WString(U16 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 5;
	const W16* threeDigits;

	while (value > 999)
	{
		U16 newVal = value / 1000;
		U16 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	size = 5 - (c - str);

	memcpy(str, c, size + 1);
}

inline WString::WString(I32 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 11;
	const W16* threeDigits;
	U8 neg = 0;

	U32 abs = (U32)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U32)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U32 newVal = abs / 1000;
		U32 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	size = 11 + neg - (c - str);

	memcpy(str + neg, c, size + 1);
}

inline WString::WString(U32 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 10;
	const W16* threeDigits;

	while (value > 999)
	{
		U32 newVal = value / 1000;
		U32 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	size = 10 - (c - str);

	memcpy(str, c, size + 1);
}

inline WString::WString(I64 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 20;
	const W16* threeDigits;
	U8 neg = 0;

	U64 abs = (U64)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U64)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U64 newVal = abs / 1000;
		U64 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	size = 20 + neg - (c - str);

	memcpy(str + neg, c, size + 1);
}

inline WString::WString(U64 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 20;
	const W16* threeDigits;

	while (value > 999)
	{
		U64 newVal = value / 1000;
		U64 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	size = 20 - (c - str);

	memcpy(str, c, size + 1);
}

inline WString::WString(F32 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 27;
	const W16* threeDigits;
	U8 neg = 0;

	F32 abs = value;

	if (value < 0)
	{
		str[0] = '-';
		abs = -value;
		neg = 1;
	}

	U64 dec = (U64)((abs - (F32)(U64)abs) * 100000.0f);

	U64 newVal = dec / 1000;
	U64 remainder = dec % 1000;
	threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = threeDigits[0];
	dec = newVal;

	threeDigits = THREE_DIGIT_NUMBERS + (dec * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = '.';

	U64 whole = (U64)abs;

	while (whole > 999)
	{
		U64 newVal = whole / 1000;
		U64 remainder = whole % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		whole = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (whole * 3);
	*--c = threeDigits[2];
	if (whole > 9) { *--c = threeDigits[1]; }
	if (whole > 99) { *--c = threeDigits[0]; }

	size = 27 + neg - (c - str);

	memcpy(str + neg, c, size + 1);
}

inline WString::WString(F64 value) : str{ (W16*)Memory::Allocate1kb() }
{
	W16* c = str + 27;
	const W16* threeDigits;
	U8 neg = 0;

	F64 abs = value;

	if (value < 0)
	{
		str[0] = '-';
		abs = -value;
		neg = 1;
	}

	U64 dec = (U64)((abs - (F64)(U64)abs) * 100000.0);

	U64 newVal = dec / 1000;
	U64 remainder = dec % 1000;
	threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = threeDigits[0];
	dec = newVal;

	threeDigits = THREE_DIGIT_NUMBERS + (dec * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = '.';

	U64 whole = (U64)abs;

	while (whole > 999)
	{
		U64 newVal = whole / 1000;
		U64 remainder = whole % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		whole = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (whole * 3);
	*--c = threeDigits[2];
	if (whole > 9) { *--c = threeDigits[1]; }
	if (whole > 99) { *--c = threeDigits[0]; }

	size = 27 + neg - (c - str);

	memcpy(str + neg, c, size + 1);
}

inline WString::WString(bool value) : str{ (W16*)Memory::Allocate1kb() }
{
	if (value)
	{
		size = 4;
		memcpy(str, "true", 4);
	}
	else
	{
		size = 5;
		memcpy(str, "false", 5);
	}
}

inline WString::WString(W16* str) : size{ wcslen(str) }, capacity{ size }, str{ (W16*)Memory::Allocate(capacity, capacity) }
{
	memcpy(this->str, str, size + 1);
}

inline WString::WString(const W16* str) : size{ wcslen(str) }, capacity{ size }, str{ (W16*)Memory::Allocate(capacity, capacity) }
{
	memcpy(this->str, str, size + 1);
}

inline WString::WString(const WString& other) : size{ other.size }, capacity{ other.capacity }, str{ (W16*)Memory::Allocate(capacity) }
{
	memcpy(str, other.str, size + 1);
}

inline WString::WString(WString&& other) noexcept : size{ other.size }, capacity{ other.capacity }, str{ other.str }
{
	other.size = 0;
	other.capacity = 0;
	other.str = nullptr;
}

template<typename... Types> inline WString::WString(const W16* fmt, const Types& ... args) : size{ wcslen(fmt) }, capacity{ size }, str{ (W16*)Memory::Allocate(capacity, capacity) }
{
	memcpy(str, fmt, size + 1);
	U64 start = 0;
	(Format(start, args), ...);
}

template<typename... Types> inline WString::WString(const Types& ... args) : str{ (W16*)Memory::Allocate1kb() }
{
	(Append(args), ...);
}

inline WString& WString::operator=(I8 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	if (value < 0)
	{
		str[0] = '-';
		U8 abs = (U8)-value;
		W16* c = str + 4;
		const W16* threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
		*--c = threeDigits[2];
		if (abs > 9) { *--c = threeDigits[1]; }
		if (abs > 99) { *--c = threeDigits[0]; }

		size = 5 - (c - str);
		memcpy(str + 1, c, size);
		str[size] = '\0';
	}
	else
	{
		W16* c = str + 3;
		const W16* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
		*--c = threeDigits[2];
		if (value > 9) { *--c = threeDigits[1]; }
		if (value > 99) { *--c = threeDigits[0]; }

		size = 3 - (c - str);
		memcpy(str, c, size);
		str[size] = '\0';
	}

	return *this;
}

inline WString& WString::operator=(U8 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 3;
	const W16* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	size = 3 - (c - str);
	memcpy(str, c, size);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(I16 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 6;
	const W16* threeDigits;
	U8 neg = 0;

	U16 abs = (U16)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U16)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U16 newVal = abs / 1000;
		U16 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	size = 6 + neg - (c - str);

	memcpy(str + neg, c, size);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(U16 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 5;
	const W16* threeDigits;

	while (value > 999)
	{
		U16 newVal = value / 1000;
		U16 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	size = 5 - (c - str);

	memcpy(str, c, size);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(I32 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 11;
	const W16* threeDigits;
	U8 neg = 0;

	U32 abs = (U32)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U32)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U32 newVal = abs / 1000;
		U32 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	size = 11 + neg - (c - str);

	memcpy(str + neg, c, size);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(U32 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 10;
	const W16* threeDigits;

	while (value > 999)
	{
		U32 newVal = value / 1000;
		U32 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	size = 10 - (c - str);

	memcpy(str, c, size);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(I64 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 20;
	const W16* threeDigits;
	U8 neg = 0;

	U64 abs = (U64)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U64)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U64 newVal = abs / 1000;
		U64 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	size = 20 + neg - (c - str);

	memcpy(str + neg, c, size);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(U64 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 20;
	const W16* threeDigits;

	while (value > 999)
	{
		U64 newVal = value / 1000;
		U64 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	size = 20 - (c - str);

	memcpy(str, c, size);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(F32 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 27;
	const W16* threeDigits;
	U8 neg = 0;

	F64 abs = value;

	if (value < 0)
	{
		str[0] = '-';
		abs = -value;
		neg = 1;
	}

	U64 dec = (U64)((abs - (F64)(U64)abs) * 100000.0f);

	U64 newVal = dec / 1000;
	U64 remainder = dec % 1000;
	threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = threeDigits[0];
	dec = newVal;

	threeDigits = THREE_DIGIT_NUMBERS + (dec * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = '.';

	U64 whole = (U64)abs;

	while (whole > 999)
	{
		U64 newVal = whole / 1000;
		U64 remainder = whole % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		whole = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (whole * 3);
	*--c = threeDigits[2];
	if (whole > 9) { *--c = threeDigits[1]; }
	if (whole > 99) { *--c = threeDigits[0]; }

	size = 27 + neg - (c - str);

	memcpy(str + neg, c, size + 1);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(F64 value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	W16* c = str + 27;
	const W16* threeDigits;
	U8 neg = 0;

	F64 abs = value;

	if (value < 0)
	{
		str[0] = '-';
		abs = -value;
		neg = 1;
	}

	U64 dec = (U64)((abs - (F64)(U64)abs) * 100000.0);

	U64 newVal = dec / 1000;
	U64 remainder = dec % 1000;
	threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = threeDigits[0];
	dec = newVal;

	threeDigits = THREE_DIGIT_NUMBERS + (dec * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = '.';

	U64 whole = (U64)abs;

	while (whole > 999)
	{
		U64 newVal = whole / 1000;
		U64 remainder = whole % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		whole = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (whole * 3);
	*--c = threeDigits[2];
	if (whole > 9) { *--c = threeDigits[1]; }
	if (whole > 99) { *--c = threeDigits[0]; }

	size = 27 + neg - (c - str);

	memcpy(str + neg, c, size + 1);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator=(bool value)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate1kb(); capacity = 1024; }

	if (value)
	{
		size = 4;
		memcpy(str, "true", 4);
		str[size] = '\0';
	}
	else
	{
		size = 5;
		memcpy(str, "false", 5);
		str[size] = '\0';
	}

	return *this;
}

inline WString& WString::operator=(W16* str)
{
	hashed = false;
	size = wcslen(str);
	if (!str) { str = (W16*)Memory::Allocate(size, capacity); }

	memcpy(this->str, str, size + 1);

	return *this;
}

inline WString& WString::operator=(const W16* str)
{
	hashed = false;
	size = wcslen(str);
	if (!str) { str = (W16*)Memory::Allocate(size, capacity); }

	memcpy(this->str, str, size + 1);

	return *this;
}

inline WString& WString::operator=(const WString& other)
{
	hashed = false;
	if (!str) { str = (W16*)Memory::Allocate(other.capacity); }

	size = other.size;
	capacity = other.capacity;
	memcpy(this->str, other.str, size + 1);

	return *this;
}

inline WString& WString::operator=(WString&& other) noexcept
{
	hashed = false;
	if (str) { Memory::Free(str); }

	size = other.size;
	capacity = other.capacity;
	str = other.str;
	other.size = 0;
	other.capacity = 0;
	other.str = nullptr;

	return *this;
}

inline WString::~WString()
{
	Destroy();
}

inline void WString::Destroy()
{
	hashed = false;
	if (str)
	{
		size = 0;
		capacity = 0;
		Memory::Free(str);
		str = nullptr;
	}
}

inline void WString::Clear()
{
	hashed = false;
	str[0] = '\0';
	size = 0;
}

inline void WString::Reserve(U64 size)
{
	if (size > capacity)
	{
		W16* temp = (W16*)Memory::Allocate(size, capacity);
		memcpy(temp, str, this->size);
		Memory::Free(str);
		str = temp;
	}
}

inline void WString::Resize(U64 size)
{
	if (size > this->capacity) { Reserve(size); }
	this->size = size;
	str[size] = '\0';
}

inline I8 WString::ToI8() const
{
	W16* it = str;
	W16 c;
	I8 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline U8 WString::ToU8() const
{
	W16* it = str;
	W16 c;
	U8 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline I16 WString::ToI16() const
{
	W16* it = str;
	W16 c;
	I16 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline U16 WString::ToU16() const
{
	W16* it = str;
	W16 c;
	U16 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline I32 WString::ToI32() const
{
	W16* it = str;
	W16 c;
	I32 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline U32 WString::ToU32() const
{
	W16* it = str;
	W16 c;
	U32 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline I64 WString::ToI64() const
{
	W16* it = str;
	W16 c;
	I64 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline U64 WString::ToU64() const
{
	W16* it = str;
	W16 c;
	U64 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline F32 WString::ToF32() const
{
	W16* it = str;
	W16 c;
	F32 value = 0.0f;
	F32 mul = 0.1f;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value -= c - '0'; }
		while ((c = *it++) != '\0') { value -= (c - '0') * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value += c - '0'; }
		while ((c = *it++) != '\0') { value += (c - '0') * mul; mul *= 0.1f; }
	}

	return value;
}

inline F64 WString::ToF64() const
{
	W16* it = str;
	W16 c;
	F64 value = 0.0f;
	F64 mul = 0.1f;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value -= c - '0'; }
		while ((c = *it++) != '\0') { value -= (c - '0') * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value += c - '0'; }
		while ((c = *it++) != '\0') { value += (c - '0') * mul; mul *= 0.1f; }
	}

	return value;
}

inline bool WString::ToBool() const
{
	return str[0] == 't' && str[1] == 'r' && str[2] == 'u' && str[3] == 'e';
}

inline WString& WString::operator+=(I8 value)
{
	hashed = false;
	W16* start = str + size;

	if (value < 0)
	{
		start[0] = '-';
		U8 abs = (U8)-value;
		W16* c = start + 4;
		const W16* threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
		*--c = threeDigits[2];
		if (abs > 9) { *--c = threeDigits[1]; }
		if (abs > 99) { *--c = threeDigits[0]; }

		U64 addLength = 5 - (c - start);
		size += addLength;
		memcpy(start + 1, c, addLength);
		str[size] = '\0';
	}
	else
	{
		W16* c = start + 3;
		const W16* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
		*--c = threeDigits[2];
		if (value > 9) { *--c = threeDigits[1]; }
		if (value > 99) { *--c = threeDigits[0]; }

		U64 addLength = 3 - (c - start);
		size += addLength;
		memcpy(start, c, addLength);
		str[size] = '\0';
	}

	return *this;
}

inline WString& WString::operator+=(U8 value)
{
	hashed = false;
	W16* start = str + size;
	W16* c = start + 3;
	const W16* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	U64 addLength = 3 - (c - start);
	size += addLength;
	memcpy(start, c, addLength);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(I16 value)
{
	hashed = false;
	W16* start = str + size;
	W16* c = start + 6;
	const W16* threeDigits;
	U8 neg = 0;

	U16 abs = (U16)value;

	if (value < 0)
	{
		start[0] = '-';
		abs = (U16)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U16 newVal = abs / 1000;
		U16 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	U64 addLength = 6 + neg - (c - start);
	size += addLength;

	memcpy(start + neg, c, addLength);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(U16 value)
{
	hashed = false;
	W16* start = str + size;
	W16* c = start + 5;
	const W16* threeDigits;

	while (value > 999)
	{
		U16 newVal = value / 1000;
		U16 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	U64 addLength = 5 - (c - start);
	size += addLength;

	memcpy(start, c, addLength);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(I32 value)
{
	hashed = false;
	W16* start = str + size;
	W16* c = start + 11;
	const W16* threeDigits;
	U8 neg = 0;

	U32 abs = (U32)value;

	if (value < 0)
	{
		start[0] = '-';
		abs = (U32)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U32 newVal = abs / 1000;
		U32 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	U64 addLength = 11 + neg - (c - start);
	size += addLength;

	memcpy(start + neg, c, addLength);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(U32 value)
{
	hashed = false;
	W16* start = str + size;
	W16* c = start + 10;
	const W16* threeDigits;

	while (value > 999)
	{
		U32 newVal = value / 1000;
		U32 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	U64 addLength = 10 - (c - start);
	size += addLength;

	memcpy(start, c, addLength);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(I64 value)
{
	hashed = false;
	W16* start = str + size;
	W16* c = start + 20;
	const W16* threeDigits;
	U8 neg = 0;

	U64 abs = (U64)value;

	if (value < 0)
	{
		start[0] = '-';
		abs = (U64)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U64 newVal = abs / 1000;
		U64 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	U64 addLength = 20 + neg - (c - start);
	size += addLength;

	memcpy(start + neg, c, addLength);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(U64 value)
{
	hashed = false;
	W16* start = str + size;
	W16* c = start + 20;
	const W16* threeDigits;

	while (value > 999)
	{
		U64 newVal = value / 1000;
		U64 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	U64 addLength = 20 - (c - start);
	size += addLength;

	memcpy(start, c, addLength);
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(F32 value)
{
	hashed = false;
	return *this;
}

inline WString& WString::operator+=(F64 value)
{
	hashed = false;
	return *this;
}

inline WString& WString::operator+=(bool value)
{
	hashed = false;
	if (value)
	{
		memcpy(str + size, "true", 4);
		size += 4;
		str[size] = '\0';
	}
	else
	{
		memcpy(str + size, "false", 5);
		size += 5;
		str[size] = '\0';
	}

	return *this;
}

inline WString& WString::operator+=(W16* other)
{
	hashed = false;
	U64 addLength = wcslen(other);
	memcpy(str + size, other, addLength);
	size += addLength;
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(const W16* other)
{
	hashed = false;
	U64 addLength = wcslen(other);
	memcpy(str + size, other, addLength);
	size += addLength;
	str[size] = '\0';

	return *this;
}

inline WString& WString::operator+=(const WString& other)
{
	hashed = false;
	U64 addLength = wcslen(other.str);
	memcpy(str + size, other.str, addLength);
	size += addLength;
	str[size] = '\0';

	return *this;
}

inline WString::operator W16* () { return str; }

inline WString::operator const W16* () const { return str; }

inline WString::operator I8() const
{
	W16* it = str;
	W16 c;
	I8 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline WString::operator U8() const
{
	W16* it = str;
	W16 c;
	U8 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline WString::operator I16() const
{
	W16* it = str;
	W16 c;
	I16 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline WString::operator U16() const
{
	W16* it = str;
	W16 c;
	U16 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline WString::operator I32() const
{
	W16* it = str;
	W16 c;
	I32 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline WString::operator U32() const
{
	W16* it = str;
	W16 c;
	U32 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline WString::operator I64() const
{
	W16* it = str;
	W16 c;
	I64 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline WString::operator U64() const
{
	W16* it = str;
	W16 c;
	U64 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline WString::operator F32() const
{
	W16* it = str;
	W16 c;
	F32 value = 0.0f;
	F32 mul = 0.1f;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value -= c - '0'; }
		while ((c = *it++) != '\0') { value -= (c - '0') * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value += c - '0'; }
		while ((c = *it++) != '\0') { value += (c - '0') * mul; mul *= 0.1f; }
	}

	return value;
}

inline WString::operator F64() const
{
	W16* it = str;
	W16 c;
	F64 value = 0.0f;
	F64 mul = 0.1f;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value -= c - '0'; }
		while ((c = *it++) != '\0') { value -= (c - '0') * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value += c - '0'; }
		while ((c = *it++) != '\0') { value += (c - '0') * mul; mul *= 0.1f; }
	}

	return value;
}

inline WString::operator bool() const
{
	return str[0] == 't' && str[1] == 'r' && str[2] == 'u' && str[3] == 'e';
}

inline W16* WString::operator*() { return str; }

inline const W16* WString::operator*() const { return str; }

inline W16& WString::operator[](U32 i) { return str[i]; }

inline const W16& WString::operator[](U32 i) const { return str[i]; }

inline bool WString::operator==(W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size) == 0;
}

inline bool WString::operator==(const W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size) == 0;
}

inline bool WString::operator==(const WString& other) const
{
	if (other.size != size) { return false; }

	return memcmp(str, other.str, size) == 0;
}

inline bool WString::operator!=(W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return true; }

	return memcmp(str, other, size);
}

inline bool WString::operator!=(const W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return true; }

	return memcmp(str, other, size);
}

inline bool WString::operator!=(const WString& other) const
{
	if (other.size != size) { return true; }

	return memcmp(str, other.str, size);
}

inline bool WString::Compare(W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size) == 0;
}

inline bool WString::Compare(const W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size) == 0;
}

inline bool WString::Compare(const WString& other) const
{
	if (other.size != size) { return false; }

	return memcmp(str, other.str, size) == 0;
}

inline bool WString::CompareN(W16* other, U32 nLength, U32 start) const
{
	U64 len = wcslen(other);
	if (len != nLength) { return false; }

	return memcmp(str + start, other, nLength) == 0;
}

inline bool WString::CompareN(const W16* other, U32 nLength, U32 start) const
{
	U64 len = wcslen(other);
	if (len != nLength) { return false; }

	return memcmp(str + start, other, nLength) == 0;
}

inline bool WString::CompareN(const WString& other, U32 nLength, U32 start) const
{
	if (other.size != nLength) { return false; }

	return memcmp(str + start, other.str, nLength) == 0;
}

inline const U64& WString::Size() const { return size; }

inline const U64& WString::Capacity() const { return capacity; }

inline U64 WString::Hash()
{
	if (hashed) { return hash; }

	hash = 0;
	const W16* ptr = str;
	while (*ptr) { hash = hash * 101 + *ptr++; }
	hashed = true;

	return hash;
}

inline W16* WString::Data() { return str; }

inline const W16* WString::Data() const { return str; }

inline bool WString::Blank() const
{
	if (size == 0) { return true; }
	W16* start = str;
	W16 c;

	while ((c = *start) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f');

	return start - str == size;
}

inline I32 WString::IndexOf(const W16& c, U64 start) const
{
	W16* it = str + start;

	while (*it != c && *it != '\0') { ++it; }

	if (*it == '\0') { return -1; }
	return (I32)(it - str);
}

inline I32 WString::LastIndexOf(const W16& c, U64 start) const
{
	W16* it = str + size - start - 1;

	U64 len = size;
	while (*it != c && len > 0) { --it; --len; }

	if (len) { return (I32)(it - str); }
	return -1;
}

inline WString& WString::Trim()
{
	hashed = false;
	W16* start = str;
	W16* end = str + size;
	W16 c;

	while ((c = *start) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f' || c == '\0') { ++start; }
	while ((c = *end) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f' || c == '\0') { --end; }

	size = end - start;
	memcpy(str, start, size);
	str[size] = '\0';

	return *this;
}

inline WString& WString::SubString(WString& newStr, U64 start, U64 nLength) const
{
	if (nLength < U64_MAX) { newStr.size = nLength; }
	else { newStr.size = size - start; }

	memcpy(newStr.str, str + start, newStr.size);
	newStr.str[newStr.size] = '\0';

	return newStr;
}

inline WString& WString::Append(const WString& append)
{
	hashed = false;
	memcpy(str + size, append.str, append.size);
	size += append.size;
	str[size] = '\0';

	return *this;
}

inline WString& WString::Prepend(const WString& prepend)
{
	hashed = false;
	memcpy(str + size, str, size);
	memcpy(str, prepend.str, prepend.size);
	size += prepend.size;
	str[size] = '\0';

	return *this;
}

inline WString& WString::Surround(const WString& prepend, const WString& append)
{
	hashed = false;
	memcpy(str + prepend.size, str, size);
	memcpy(str, prepend.str, prepend.size);
	size += prepend.size;

	memcpy(str + size, append.str, append.size);
	size += append.size;
	str[size] = '\0';

	return *this;
}

inline WString& WString::Insert(const WString& other, U32 i)
{
	hashed = false;
	memcpy(str + i + other.size, str + i, size - i);
	memcpy(str + i, other.str, other.size);
	size += other.size;
	str[size] = '\0';

	return *this;
}

inline WString& WString::ReplaceAll(const WString& find, const WString& replace, U64 start)
{
	hashed = false;
	W16* c = str + start;
	W16 ch = *c;
	while (ch != '\0')
	{
		while ((ch = *c) != '\0' && memcmp(c, find.str, find.size)) { ++c; }

		if (ch != '\0')
		{
			memcpy(c + replace.size, c + find.size, size - find.size - (c - str));
			memcpy(c, replace.str, replace.size);
			size = size - find.size + replace.size;
			str[size] = '\0';
		}
	}

	return *this;
}

inline WString& WString::ReplaceN(const WString& find, const WString& replace, U64 count, U64 start)
{
	hashed = false;
	W16* c = str + start;
	W16 ch = *c;
	while (ch != '\0' && count)
	{
		while ((ch = *c) != '\0' && memcmp(c, find.str, find.size)) { ++c; }

		if (ch != '\0')
		{
			--count;
			memcpy(c + replace.size, c + find.size, size - find.size - (c - str));
			memcpy(c, replace.str, replace.size);
			size = size - find.size + replace.size;
			str[size] = '\0';
		}
	}

	return *this;
}

inline WString& WString::ReplaceFirst(const WString& find, const WString& replace, U64 start)
{
	hashed = false;
	W16* c = str + start;
	while (*c != '\0' && memcmp(c, find.str, find.size)) { ++c; }

	if (*c != '\0')
	{
		memcpy(c + replace.size, c + find.size, size - find.size - (c - str));
		memcpy(c, replace.str, replace.size);
		size = size - find.size + replace.size;
		str[size] = '\0';
	}

	return *this;
}

inline void WString::Split(Vector<WString>& list, U8 delimiter, bool trimEntries) const
{

}

inline void WString::Format(U64& start, const WString& replace)
{
	hashed = false;
	W16* c = str + start;
	while (*c != '\0' && memcmp(c, "{}", 2)) { ++c; }

	if (*c != '\0')
	{
		start = (c - str) + replace.size;
		memcpy(c + replace.size, c + 2, size - 2 - (c - str));
		memcpy(c, replace.str, replace.size);
		size = size - 2 + replace.size;
		str[size] = '\0';
	}
}

//inline Vector<WString> WString::Split(U8 delimiter, bool trimEntries) const

inline W16* WString::begin() { return str; }

inline W16* WString::end() { return str + size; }

inline const W16* WString::begin() const { return str; }

inline const W16* WString::end() const { return str + size; }

inline W16* WString::rbegin() { return str + size - 1; }

inline W16* WString::rend() { return str - 1; }

inline const W16* WString::rbegin() const { return str + size - 1; }

inline const W16* WString::rend() const { return str - 1; }