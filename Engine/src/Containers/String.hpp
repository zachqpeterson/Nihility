#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"

//TODO: Temp
#include <string.h>

template<typename> struct Vector;

struct NH_API String
{
	struct Iterator
	{
		Iterator(char* ptr) : ptr{ ptr } {}

		char& operator* () const { return *ptr; }
		char* operator-> () { return ptr; }

		Iterator& operator++ () { ++ptr; return *this; }
		Iterator operator++ (int)
		{
			Iterator temp = *this;
			++ptr;
			return temp;
		}

		Iterator& operator-- () { --ptr; return *this; }
		Iterator operator-- (int)
		{
			Iterator temp = *this;
			--ptr;
			return temp;
		}

		Iterator operator+(int i)
		{
			Iterator temp = *this;
			temp += i;
			return temp;
		}

		Iterator operator-(int i)
		{
			Iterator temp = *this;
			temp -= i;
			return temp;
		}

		Iterator& operator+=(int i)
		{
			ptr += i;
			return *this;
		}

		Iterator& operator-=(int i)
		{
			ptr -= i;
			return *this;
		}

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; }
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; }
		friend bool operator< (const Iterator& a, const Iterator& b) { return a.ptr > b.ptr; }
		friend bool operator> (const Iterator& a, const Iterator& b) { return a.ptr < b.ptr; }
		friend bool operator<= (const Iterator& a, const Iterator& b) { return a.ptr >= b.ptr; }
		friend bool operator>= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr; }

		operator bool() { return ptr; }

	private:
		char* ptr;
	};

	String() : str{ nullptr }, length{ 0 } {}
	String(char* str);
	String(const char* str);
	String(const String& other);
	String(String&& other) noexcept;

	~String();
	void Destroy();

	void* operator new(U64 size);
	void operator delete(void* ptr);

	String(I32 value);
	String(I64 value);
	String(U32 value);
	String(U64 value);
	String(F32 value);
	String(F64 value);
	String(bool value);

	String& operator=(char* str);
	String& operator=(const char* str);
	String& operator=(const String& other);
	String& operator=(String&& other) noexcept;

	String& operator=(I32 value);
	String& operator=(I64 value);
	String& operator=(U32 value);
	String& operator=(U64 value);
	String& operator=(F32 value);
	String& operator=(F64 value);
	String& operator=(bool value);

	U64& Length();
	const U64& Length() const;
	String Duplicate() const;
	String NDuplicate(U64 length) const;
	bool Blank() const;
	void Empty();
	String& Trim();
	String SubString(U64 start, U64 length = I64_MAX) const;
	I64 IndexOf(char c, U64 start = 0) const;
	Vector<String> Split(char delimiter, bool trimEntries) const;
	String& Append(const String& append);
	String& Prepend(const String& prepend);
	String& Surround(const String& prepend, const String& append);
	String& Insert(const String& string, U64 i);
	String& Replace(const String& find, const String& replace, U64 start = 0);
	String& ReplaceFirst(const String& find, const String& replace, U64 start = 0);

	I8  ToI8();
	I16 ToI16();
	I32 ToI32();
	I64 ToI64();
	U8  ToU8();
	U16 ToU16();
	U32 ToU32();
	U64 ToU64();
	F32 ToF32();
	F64 ToF64();
	bool ToBool();

	String operator+(char* other) { String s = *this; s.Append(other); return s; }
	String operator+(const char* other) { String s = *this; s.Append(other); return s; }
	String operator+(String& other) { String s = *this; s.Append(other); return s; }
	friend String operator+(char* other0, String& other1) { String newStr = other1; newStr.Prepend(other0); return newStr; }
	friend String operator+(const char* other0, String& other1) { String newStr = other1; newStr.Prepend(other0); return newStr; }

	Iterator begin() { return Iterator{ str }; }
	Iterator end() { return Iterator{ &str[length] }; }
	Iterator begin() const { return Iterator{ str }; }
	Iterator end() const { return Iterator{ &str[length] }; }

	operator const char* () const { return str; }
	operator char* () { return str; }
	const char* operator*() const { return str; }
	char* operator*() { return str; }
	char& operator[](U64 i) { return str[i]; }
	const char& operator[](U64 i) const { return str[i]; }

	bool operator==(const String& other) const;
	bool operator==(const char* other) const;
	bool operator==(char* other) const;
	bool operator!=(const String& other) const;
	bool operator!=(const char* other) const;
	bool operator!=(char* other) const;

	template<typename... Types>
	String(const char* fmt, const Types& ... args)
	{
		if (fmt == nullptr)
		{
			str = (char*)Memory::Allocate(1, MEMORY_TAG_STRING);
			length = 0;
		}
		else
		{
			length = strlen(fmt);
			str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_STRING);
			Memory::Copy(str, fmt, length);
		}

		Format(args...);
	}

	template<typename... Types>
	void Format(const Types& ... args)
	{
		(ReplaceFirst("{}", args), ...);
	}

private:
	char* str;
	U64 length;
};