#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"

struct String
{
public:
	String();
	String(I8 value);
	String(U8 value);
	String(I16 value);
	String(U16 value);
	String(I32 value);
	String(U32 value);
	String(I64 value);
	String(U64 value);
	String(F32 value);
	String(F64 value);
	String(bool value);
	String(U8* str);
	String(const U8* str);
	String(const String& other);
	String(String&& other) noexcept;
	template<typename... Types> String(const U8* fmt, const Types& ... args);

	String& operator=(I8 value);
	String& operator=(U8 value);
	String& operator=(I16 value);
	String& operator=(U16 value);
	String& operator=(I32 value);
	String& operator=(U32 value);
	String& operator=(I64 value);
	String& operator=(U64 value);
	String& operator=(F32 value);
	String& operator=(F64 value);
	String& operator=(bool value);
	String& operator=(U8* str);
	String& operator=(const U8* str);
	String& operator=(const String& other);
	String& operator=(String&& other) noexcept;

	~String();
	void Destroy();
	void Clear();

	I8  ToI8();
	U8  ToU8();
	I16 ToI16();
	U16 ToU16();
	I32 ToI32();
	U32 ToU32();
	I64 ToI64();
	U64 ToU64();
	F32 ToF32();
	F64 ToF64();
	bool ToBool();

	String operator+(I8  value) const;
	String operator+(I16 value) const;
	String operator+(I32 value) const;
	String operator+(I64 value) const;
	String operator+(U8  value) const;
	String operator+(U16 value) const;
	String operator+(U32 value) const;
	String operator+(U64 value) const;
	String operator+(F32 value) const;
	String operator+(F64 value) const;
	String operator+(bool value) const;
	String operator+(U8* other) const;
	String operator+(const U8* other) const;
	String operator+(const String& other) const;
	friend String operator+(U8* other0, const String& other1);
	friend String operator+(const U8* other0, const String& other1);

	explicit operator U8* ();
	explicit operator const U8* () const;
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

	U8* operator*();
	const U8* operator*() const;
	U8& operator[](U32 i);
	const U8& operator[](U32 i) const;

	bool operator==(U8* other) const;
	bool operator==(const U8* other) const;
	bool operator==(const String& other) const;
	bool operator!=(U8* other) const;
	bool operator!=(const U8* other) const;
	bool operator!=(const String& other) const;

	bool Compare(U8* other) const;
	bool Compare(const U8* other) const;
	bool Compare(const String& other) const;
	bool CompareN(U8* other, U32 lenth, U32 start = 0) const;
	bool CompareN(const U8* other, U32 lenth, U32 start = 0) const;
	bool CompareN(const String& other, U32 lenth, U32 start = 0) const;

	const U32& Length() const;
	String Duplicate() const;
	String NDuplicate(U64 length) const;
	bool Blank() const;
	String& Trim();
	String SubString(U64 start, U64 length = I64_MAX) const;
	I32 IndexOf(U8 c, U64 start = 0) const;
	String& Append(const String& append);
	String& Prepend(const String& prepend);
	String& Surround(const String& prepend, const String& append);
	String& Insert(const String& string, U32 i);
	String& ReplaceAll(const String& find, const String& replace, U64 start = 0);
	String& ReplaceN(const String& find, const String& replace, U64 start = 0);
	String& ReplaceFirst(const String& find, const String& replace, U64 start = 0);
	//Vector<String> Split(U8 delimiter, bool trimEntries) const;

	U8* begin();
	U8* end();
	const U8* begin() const;
	const U8* end() const;

	U8* rbegin();
	U8* rend();
	const U8* rbegin() const;
	const U8* rend() const;

private:
	U32 length;
	U8* str;
};

inline String::String() : length{ 0 }, str{ (U8*)Memory::Allocate1kb() } {}

inline String::String(I8 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(U8 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(I16 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(U16 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(I32 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(U32 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(I64 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(U64 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(F32 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(F64 value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(bool value) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{
	if (value)
	{
		length = 4;
	}
	else
	{
		length = 5;
	}
}

inline String::String(U8* str) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(const U8* str) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(const String& other) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String::String(String&& other) noexcept : length{ other.length }, str{ other.str }
{
	other.length = 0;
	other.str = nullptr;
}

template<typename... Types> inline String::String(const U8* fmt, const Types& ... args) : length{ 0 }, str{ (U8*)Memory::Allocate1kb() }
{

}

inline String& String::operator=(I8 value)
{

}

inline String& String::operator=(U8 value)
{

}

inline String& String::operator=(I16 value)
{

}

inline String& String::operator=(U16 value)
{

}

inline String& String::operator=(I32 value)
{

}

inline String& String::operator=(U32 value)
{

}

inline String& String::operator=(I64 value)
{

}

inline String& String::operator=(U64 value)
{

}

inline String& String::operator=(F32 value)
{

}

inline String& String::operator=(F64 value)
{

}

inline String& String::operator=(bool value)
{

}

inline String& String::operator=(U8* str)
{

}

inline String& String::operator=(const U8* str)
{

}

inline String& String::operator=(const String& other)
{

}

inline String& String::operator=(String&& other) noexcept
{

}

inline String::~String()
{
	Destroy();
}

inline void String::Destroy()
{
	if (str)
	{
		length = 0;
		Memory::Free1kb(str);
	}
}

inline void String::Clear()
{
	if (str)
	{
		str[0] = '\0';
		length = 0;
	}
}

inline I8  String::ToI8()
{

}

inline U8  String::ToU8()
{

}

inline I16 String::ToI16()
{

}

inline U16 String::ToU16()
{

}

inline I32 String::ToI32()
{

}

inline U32 String::ToU32()
{

}

inline I64 String::ToI64()
{

}

inline U64 String::ToU64()
{

}

inline F32 String::ToF32()
{

}

inline F64 String::ToF64()
{

}


inline bool String::ToBool()
{

}

inline String String::operator+(I8  value) const
{

}

inline String String::operator+(I16 value) const
{

}

inline String String::operator+(I32 value) const
{

}

inline String String::operator+(I64 value) const
{

}

inline String String::operator+(U8  value) const
{

}

inline String String::operator+(U16 value) const
{

}

inline String String::operator+(U32 value) const
{

}

inline String String::operator+(U64 value) const
{

}

inline String String::operator+(F32 value) const
{

}

inline String String::operator+(F64 value) const
{

}

inline String String::operator+(bool value) const
{

}

inline String String::operator+(U8* other) const
{

}

inline String String::operator+(const U8* other) const
{

}

inline String String::operator+(const String& other) const
{

}

inline String operator+(U8* other0, const String& other1)
{

}

inline String operator+(const U8* other0, const String& other1)
{

}

inline String::operator U8* () { return str; }

inline String::operator const U8* () const { return str; }

inline String::operator I8() const
{

}

inline String::operator U8() const
{

}

inline String::operator I16() const
{

}

inline String::operator U16() const
{

}

inline String::operator I32() const
{

}

inline String::operator U32() const
{

}

inline String::operator I64() const
{

}

inline String::operator U64() const
{

}

inline String::operator F32() const
{

}

inline String::operator F64() const
{

}

inline String::operator bool() const
{

}

inline U8* String::operator*() { return str; }

inline const U8* String::operator*() const { return str; }

inline U8& String::operator[](U32 i) { return str[i]; }

inline const U8& String::operator[](U32 i) const { return str[i]; }

inline bool String::operator==(U8* other) const
{

}

inline bool String::operator==(const U8* other) const
{

}

inline bool String::operator==(const String& other) const
{

}

inline bool String::operator!=(U8* other) const
{

}

inline bool String::operator!=(const U8* other) const
{

}

inline bool String::operator!=(const String& other) const
{

}

inline bool String::Compare(U8* other) const
{

}

inline bool String::Compare(const U8* other) const
{

}

inline bool String::Compare(const String& other) const
{

}

inline bool String::CompareN(U8* other, U32 lenth, U32 start = 0) const
{

}

inline bool String::CompareN(const U8* other, U32 lenth, U32 start = 0) const
{

}

inline bool String::CompareN(const String& other, U32 lenth, U32 start = 0) const
{

}

inline const U32& String::Length() const { return length; }

inline String String::Duplicate() const
{

}

inline String String::NDuplicate(U64 length) const
{

}

inline bool String::Blank() const
{

}

inline String& String::Trim()
{

}

inline String String::SubString(U64 start, U64 length = I64_MAX) const
{

}

inline I32 String::IndexOf(U8 c, U64 start = 0) const
{

}

inline String& String::Append(const String& append)
{

}

inline String& String::Prepend(const String& prepend)
{

}

inline String& String::Surround(const String& prepend, const String& append)
{

}

inline String& String::Insert(const String& string, U32 i)
{

}

inline String& String::ReplaceAll(const String& find, const String& replace, U64 start = 0)
{

}

inline String& String::ReplaceN(const String& find, const String& replace, U64 start = 0)
{

}

inline String& String::ReplaceFirst(const String& find, const String& replace, U64 start = 0)
{

}

//inline Vector<String> String::Split(U8 delimiter, bool trimEntries) const

inline U8* String::begin() { return str; }

inline U8* String::end() { return str + length; }

inline const U8* String::begin() const { return str; }

inline const U8* String::end() const { return str + length; }

inline U8* String::rbegin() { return str + length - 1; }

inline U8* String::rend() { return str - 1; }

inline const U8* String::rbegin() const { return str + length - 1; }

inline const U8* String::rend() const { return str - 1; }