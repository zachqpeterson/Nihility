#include "String.hpp"

#include "Memory/Memory.hpp"
#include "Math/Math.hpp"
#include <Containers/Vector.hpp>

#include <string.h>
#include <stdio.h>

#ifndef _MSC_VER
#include <strings.h>
#endif

String::String(char* str)
{
	if (str == nullptr)
	{
		this->str = (char*)Memory::Allocate(1, MEMORY_TAG_STRING);
		length = 0;
	}
	else
	{
		length = strlen(str);
		this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(this->str, str, length);
	}
}

String::String(const char* str)
{
	if (str == nullptr)
	{
		this->str = (char*)Memory::Allocate(1, MEMORY_TAG_STRING);
		length = 0;
	}
	else
	{
		length = strlen(str);
		this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(this->str, str, length);
	}
}

String::String(const String& other)
{
	if (other.str == nullptr)
	{
		str = (char*)Memory::Allocate(1, MEMORY_TAG_STRING);
		length = 0;
	}
	else
	{
		length = strlen(other.str);
		str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(str, other.str, length);
	}
}

String::String(String&& other) noexcept
{
	str = other.str;
	other.str = nullptr;
	length = other.length;
	other.length = 0;
}

String::~String()
{
	if (str)
	{
		Memory::Free(str, length + 1, MEMORY_TAG_STRING);
		str = nullptr;
		length = 0;
	}
}

void String::Destroy()
{
	if (str)
	{
		Memory::Free(str, length + 1, MEMORY_TAG_STRING);
		str = nullptr;
		length = 0;
	}
}

void* String::operator new(U64 size)
{
	return Memory::Allocate(sizeof(String), MEMORY_TAG_STRING);
}

void String::operator delete(void* ptr)
{
	Memory::Free(ptr, sizeof(String), MEMORY_TAG_STRING);
}

String::String(I32 value)
{
	bool neg = value < 0;
	length = Math::Length(value) + neg;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	*str = '-' * neg;

	U32 val = Math::Abs(value);
	char* it = str + length - 1;

	if (val == 0) { *it = '0'; }
	else
	{
		while (val)
		{
			*it = val % 10 + '0';
			val /= 10;
			--it;
		}
	}
}

String::String(I64 value)
{
	bool neg = value < 0;
	length = Math::Length(value) + neg;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	*str = '-' * neg;

	U64 val = Math::Abs(value);
	char* it = str + length - 1;

	if (val == 0) { *it = '0'; }
	else
	{
		while (val)
		{
			*it = val % 10 + '0';
			val /= 10;
			--it;
		}
	}
}

String::String(U32 value)
{
	length = Math::Length(value);
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);

	char* it = str + length - 1;

	if (value == 0) { *it = '0'; }
	else
	{
		while (value)
		{
			*it = value % 10 + '0';
			value /= 10;
			--it;
		}
	}
}

String::String(U64 value)
{
	length = Math::Length(value);
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);

	char* it = str + length - 1;

	if (value == 0) { *it = '0'; }
	else
	{
		while (value)
		{
			*it = value % 10 + '0';
			value /= 10;
			--it;
		}
	}
}

String::String(F32 value)
{
	//TODO: NaN and infinity

	bool neg = value < 0;
	length = Math::Length(value) + neg + 6;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	*str = '-' * neg;

	U64 val = (U64)Math::Abs(value * 100000);
	char* it = str + length - 1;

	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = '.'; --it;

	if (val == 0) { *it = '0'; }
	else
	{
		while (val)
		{
			*it = val % 10 + '0';
			val /= 10;
			--it;
		}
	}
}

String::String(F64 value)
{
	//TODO: NaN and infinity

	bool neg = value < 0;
	length = Math::Length(value) + neg + 6;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	*str = '-' * neg;

	U64 val = (U64)Math::Abs(value * 100000);
	char* it = str + length - 1;

	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = '.'; --it;

	if (val == 0) { *it = '0'; }
	else
	{
		while (val)
		{
			*it = val % 10 + '0';
			val /= 10;
			--it;
		}
	}
}

String::String(bool value)
{
	static const char* trueStr = "true";
	static const char* falseStr = "false";

	if (value)
	{
		length = 4;
		this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(this->str, trueStr, length);
	}
	else
	{
		length = 5;
		this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(this->str, falseStr, length);
	}
}

String::String(const Vector2& v)
{
	length = 5;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{}", length);
	Format(v.x, v.y);
}

String::String(const Vector3& v)
{
	length = 8;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{},{}", length);
	Format(v.x, v.y, v.z);
}

String::String(const Vector4& v)
{
	length = 11;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{},{},{}", length);
	Format(v.x, v.y, v.z, v.w);
}

String::String(const Vector2Int& v)
{
	length = 5;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{}", length);
	Format(v.x, v.y);
}

String::String(const Vector3Int& v)
{
	length = 8;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{},{}", length);
	Format(v.x, v.y, v.z);
}

String::String(const Vector4Int& v)
{
	length = 11;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{},{},{}", length);
	Format(v.x, v.y, v.z, v.w);
}

String& String::operator=(char* str)
{
	if (str == this->str) { return *this; }

	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	if (str == nullptr)
	{
		this->str = (char*)Memory::Allocate(1, MEMORY_TAG_STRING);
		length = 0;
	}
	else
	{
		length = strlen(str);
		this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(this->str, str, length);
	}

	return *this;
}

String& String::operator=(const char* str)
{
	if (str == this->str) { return *this; }

	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	if (str == nullptr)
	{
		this->str = (char*)Memory::Allocate(1, MEMORY_TAG_STRING);
		length = 0;
	}
	else
	{
		length = strlen(str);
		this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(this->str, str, length);
	}

	return *this;
}

String& String::operator=(const String& other)
{
	if (str == other.str) { return *this; }

	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	if (other.str == nullptr)
	{
		str = (char*)Memory::Allocate(1, MEMORY_TAG_STRING);
		length = 0;
	}
	else
	{
		length = strlen(other.str);
		str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(str, other.str, length);
	}

	return *this;
}

String& String::operator=(String&& other) noexcept
{
	if (str == other.str) { return *this; }

	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	str = other.str;
	other.str = nullptr;
	length = other.length;
	other.length = 0;

	return *this;
}

String& String::operator=(I32 value)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	bool neg = value < 0;
	length = Math::Length(value) + neg;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	*str = '-' * neg;

	U32 val = Math::Abs(value);
	char* it = str + length - 1;

	while (val)
	{
		*it = val % 10 + '0';
		val /= 10;
		--it;
	}

	return *this;
}

String& String::operator=(I64 value)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	bool neg = value < 0;
	length = Math::Length(value) + neg;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	*str = '-' * neg;

	U64 val = Math::Abs(value);
	char* it = str + length - 1;

	while (val)
	{
		*it = val % 10 + '0';
		val /= 10;
		--it;
	}

	return *this;
}

String& String::operator=(U32 value)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	length = Math::Length(value);
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);

	char* it = str + length - 1;

	while (value)
	{
		*it = value % 10 + '0';
		value /= 10;
		--it;
	}

	return *this;
}

String& String::operator=(U64 value)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	length = Math::Length(value);
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);

	char* it = str + length - 1;

	while (value)
	{
		*it = value % 10 + '0';
		value /= 10;
		--it;
	}

	return *this;
}

String& String::operator=(F32 value)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	//TODO: NaN and infinity

	bool neg = value < 0;
	length = Math::Length(value) + neg + 6;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	*str = '-' * neg;

	U64 val = (U64)Math::Abs(value * 100000);
	char* it = str + length - 1;

	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = '.'; --it;

	if (val == 0) { *it = '0'; }
	else
	{
		while (val)
		{
			*it = val % 10 + '0';
			val /= 10;
			--it;
		}
	}

	return *this;
}

String& String::operator=(F64 value)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	//TODO: NaN and infinity

	bool neg = value < 0;
	length = Math::Length(value) + neg + 6;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	*str = '-' * neg;

	U64 val = (U64)Math::Abs(value * 100000);
	char* it = str + length - 1;

	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = val % 10 + '0'; val /= 10; --it;
	*it = '.'; --it;

	if (val == 0) { *it = '0'; }
	else
	{
		while (val)
		{
			*it = val % 10 + '0';
			val /= 10;
			--it;
		}
	}

	return *this;
}

String& String::operator=(bool value)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	static const char* trueStr = "true";
	static const char* falseStr = "false";

	if (value)
	{
		length = 4;
		this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(this->str, trueStr, length);
	}
	else
	{
		length = 5;
		this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(this->str, falseStr, length);
	}

	return *this;
}

String& String::operator=(const Vector2& v)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	length = 5;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{}", length);
	Format(v.x, v.y);

	return *this;
}

String& String::operator=(const Vector3& v)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	length = 8;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{},{}", length);
	Format(v.x, v.y, v.z);

	return *this;
}

String& String::operator=(const Vector4& v)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	length = 11;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{},{},{}", length);
	Format(v.x, v.y, v.z, v.w);

	return *this;
}

String& String::operator=(const Vector2Int& v)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	length = 5;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{}", length);
	Format(v.x, v.y);

	return *this;
}

String& String::operator=(const Vector3Int& v)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	length = 8;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{},{}", length);
	Format(v.x, v.y, v.z);

	return *this;
}

String& String::operator=(const Vector4Int& v)
{
	if (this->str)
	{
		Memory::Free(this->str, length + 1, MEMORY_TAG_STRING);
	}

	length = 11;
	str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(str, "{},{},{},{}", length);
	Format(v.x, v.y, v.z, v.w);

	return *this;
}

U64& String::Length()
{
	return length;
}

const U64& String::Length() const
{
	return length;
}

String String::Duplicate() const
{
	if (str)
	{
		char* copy = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(copy, str, length);
		return copy;
	}

	return {};
}

String String::NDuplicate(U64 length) const
{
	if (str && length < this->length)
	{
		char* copy = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
		Memory::Copy(copy, str, length);
		return copy;
	}

	return {};
}

bool String::Blank() const
{
	if (str && length > 0)
	{
		char* p = str;
		while (*p != '\0') { if (*p != ' ' && *p != '\n' && *p != '\t' && *p != '\r') { return false; } ++p; }
	}

	return true;
}

void String::Empty()
{
	if (str)
	{
		Memory::Free(str, length + 1, MEMORY_TAG_STRING);
		str = nullptr;
		length = 0;
	}
}

String& String::Trim()
{
	U64 start = 0;
	U64 length = this->length;

	char* p = str;

	while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') { ++p; ++start; --length; }
	if (length == 0)
	{
		Memory::Free(str, this->length + 1, MEMORY_TAG_STRING);
		str = nullptr;
		this->length = 0;
		return *this;
	}

	p += length - 1;
	while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') { --p; --length; }

	char* newStr = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(newStr, str + start, length);
	Memory::Free(str, this->length + 1, MEMORY_TAG_STRING);
	str = newStr;
	this->length = length;

	return *this;
}

String String::SubString(U64 start, U64 length) const
{
	if (start == this->length) { return String(""); }

	if ((start + length) > this->length || length == U64_MAX) { length = this->length - start; }

	char* newStr = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
	Memory::Copy(newStr, str + start, length);
	String s;
	s.str = newStr;
	s.length = length;

	return s;
}

I64 String::IndexOf(char c, U64 start) const
{
	if (str)
	{
		char* ch = str + start;
		for (U64 i = start; i < length; ++i, ++ch)
		{
			if (*ch == c) { return i; }
		}
	}

	return -1;
}

Vector<String> String::Split(char delimiter, bool trimEntries) const
{
	Vector<String> strings;
	if (Blank()) { return strings; }

	U64 start = 0;
	I64 end = IndexOf(delimiter, start);

	if (trimEntries)
	{
		while (end != -1)
		{
			strings.Push(SubString(start, end - start));
			strings.Back().Trim();
			start = end + 1;
			end = IndexOf(delimiter, start); //TODO: Returning wrong index
		}

		strings.Push(SubString(start));
		strings.Back().Trim();

		return strings;
	}

	while (end != -1)
	{
		strings.Push(Move(SubString(start, end - start)));
		start = end + 1;
		end = IndexOf(delimiter, start);
	}

	strings.Push(Move(SubString(start)));

	return strings;
}

String& String::Append(const String& append)
{
	if (str && append.str)
	{
		U64 newLength = length + append.length;

		char* newStr = (char*)Memory::Allocate(newLength + 1, MEMORY_TAG_STRING);

		Memory::Copy(newStr, str, length);
		Memory::Copy(newStr + length, append.str, append.length);

		Memory::Free(str, length + 1, MEMORY_TAG_STRING);
		str = newStr;
		length = newLength;
	}

	return *this;
}

String& String::Prepend(const String& prepend)
{
	if (str && prepend.str)
	{
		U64 newLength = length + prepend.length;

		char* newStr = (char*)Memory::Allocate(newLength + 1, MEMORY_TAG_STRING);

		Memory::Copy(newStr, prepend.str, prepend.length);
		Memory::Copy(newStr + prepend.length, str, length);

		Memory::Free(str, length + 1, MEMORY_TAG_STRING);
		str = newStr;
		length = newLength;
	}

	return *this;
}

String& String::Surround(const String& prepend, const String& append)
{
	if (str && prepend.str && append.str)
	{
		U64 newLength = length + prepend.length + append.length;

		char* newStr = (char*)Memory::Allocate(newLength + 1, MEMORY_TAG_STRING);

		Memory::Copy(newStr, prepend.str, prepend.length);
		Memory::Copy(newStr + prepend.length, str, length);
		Memory::Copy(newStr + prepend.length + length, append.str, append.length);

		Memory::Free(str, length + 1, MEMORY_TAG_STRING);
		str = newStr;
		length = newLength;
	}

	return *this;
}

String& String::Insert(const String& string, U64 i)
{
	if (str && string.str)
	{
		U64 newLength = length + string.length;

		char* newStr = (char*)Memory::Allocate(newLength + 1, MEMORY_TAG_STRING);

		Memory::Copy(newStr, str, i);
		Memory::Copy(newStr + i, string.str, string.length);
		Memory::Copy(newStr + i + string.length, str, length - i);

		Memory::Free(str, length + 1, MEMORY_TAG_STRING);
		str = newStr;
		length = newLength;
	}

	return *this;
}

String& String::Replace(const String& find, const String& replace, U64 start)
{
	return *this;
}

String& String::ReplaceFirst(const String& find, const String& replace, U64 start)
{
	if (str && find.str && replace.str)
	{
		char* c = str + start;
		char* f = find.str;

		char* s = nullptr;

		U64 count = 0;
		U64 index = start;

		while (*c != '\0')
		{
			s = c;

			while (*s == *f && count < find.length) { ++s; ++f; ++count; }
			f = find.str;

			if (count == find.length)
			{
				U64 newLength = length - find.length + replace.length;

				char* newStr = (char*)Memory::Allocate(newLength + 1, MEMORY_TAG_STRING);

				Memory::Copy(newStr, str, index);
				Memory::Copy(newStr + index, replace.str, replace.length);
				Memory::Copy(newStr + index + replace.length, str + index + find.length, length - index - find.length);

				Memory::Free(str, length + 1, MEMORY_TAG_STRING);
				str = newStr;
				length = newLength;

				return *this;
			}

			count = 0;

			++c;
			++index;
		}
	}

	return *this;
}

I8  String::ToI8()
{
	I8 i = 0;
	sscanf_s(str, "%hhd", &i);
	return i;
}

I16 String::ToI16()
{
	I16 i = 0;
	sscanf_s(str, "%hd", &i);
	return i;
}

I32 String::ToI32()
{
	I32 i = 0;
	sscanf_s(str, "%d", &i);
	return i;
}

I64 String::ToI64()
{
	I64 i = 0;
	sscanf_s(str, "%lld", &i);
	return i;
}

U8  String::ToU8()
{
	U8 i = 0;
	sscanf_s(str, "%hhu", &i);
	return i;
}

U16 String::ToU16()
{
	U16 i = 0;
	sscanf_s(str, "%hu", &i);
	return i;
}

U32 String::ToU32()
{
	U32 i = 0;
	sscanf_s(str, "%u", &i);
	return i;
}

U64 String::ToU64()
{
	U64 i = 0;
	sscanf_s(str, "%llu", &i);
	return i;
}

F32 String::ToF32()
{
	F32 f = 0.0f;
	sscanf_s(str, "%f", &f);
	return f;
}

F64 String::ToF64()
{
	F64 f = 0.0f;
	sscanf_s(str, "%lf", &f);
	return f;
}

bool String::ToBool()
{
	return !strcmp(str, "true") || !strcmp(str, "1");
}

Vector2 String::ToVector2()
{
	Vector2 v;
	sscanf_s(str, "%f,%f", &v.x, &v.y);
	return v;
}

Vector3 String::ToVector3()
{
	Vector3 v;
	sscanf_s(str, "%f,%f,%f", &v.x, &v.y, &v.z);
	return v;
}

Vector4 String::ToVector4()
{
	Vector4 v;
	sscanf_s(str, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w);
	return v;
}

Vector2Int String::ToVector2Int()
{
	Vector2Int v;
	sscanf_s(str, "%d,%d", &v.x, &v.y);
	return v;
}

Vector3Int String::ToVector3Int()
{
	Vector3Int v;
	sscanf_s(str, "%d,%d,%d", &v.x, &v.y, &v.z);
	return v;
}

Vector4Int String::ToVector4Int()
{
	Vector4Int v;
	sscanf_s(str, "%d,%d,%d,%d", &v.x, &v.y, &v.z, &v.w);
	return v;
}

bool String::operator==(const String& other) const
{
	if (other && length == other.length)
	{
		for (U64 i = 0; i < length; ++i)
		{
			if (str[i] != other.str[i])
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

bool String::operator==(const char* other) const
{
	if (other && length == strlen(other))
	{
		for (U64 i = 0; i < length; ++i)
		{
			if (str[i] != other[i])
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

bool String::operator==(char* other) const
{
	if (other && length == strlen(other))
	{
		for (U64 i = 0; i < length; ++i)
		{
			if (str[i] != other[i])
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

bool String::operator!=(const String& other) const
{
	if (length == other.length)
	{
		for (U64 i = 0; i < length; ++i)
		{
			if (str[i] != other.str[i])
			{
				return true;
			}
		}

		return length == 0;
	}

	return true;
}

bool String::operator!=(const char* other) const
{
	if (other)
	{
		if (length == strlen(other))
		{
			for (U64 i = 0; i < length; ++i)
			{
				if (str[i] != other[i])
				{
					return true;
				}
			}

			return length == 0;
		}

		return true;
	}

	return str == nullptr;
}

bool String::operator!=(char* other) const
{
	if (other)
	{
		if (length == strlen(other))
		{
			for (U64 i = 0; i < length; ++i)
			{
				if (str[i] != other[i])
				{
					return true;
				}
			}

			return length == 0;
		}

		return true;
	}

	return str == nullptr;
}
