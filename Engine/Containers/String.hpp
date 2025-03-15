#pragma once

#include "Defines.hpp"

#include "Platform\Memory.hpp"

template<class T>
NH_API constexpr U64 Length(const T* str) noexcept
{
	if (!str) { return 0; }

	const T* it = str;
	while (*it) { ++it; }

	return it - str;
}

struct NH_API StringView
{
	template<U64 Length>
	constexpr StringView(const C8(&str)[Length]) : string{ str }, length{ Length } {}

	constexpr StringView(const C8* str, U64 length) : string{ str }, length{ length } {}

	constexpr bool operator==(const StringView& other) const
	{
		if (other.length != length) { return false; }

		U64 size = length;
		const C8* p0 = string, * p1 = other.string;

		while (size--) { if (*p0++ != *p1++) { return false; } }

		return true;
	}

	constexpr bool operator!=(const StringView& other) const
	{
		if (other.length != length) { return true; }

		U64 size = length;
		const C8* p0 = string, * p1 = other.string;

		while (size--) { if (*p0++ != *p1++) { return true; } }

		return false;
	}

	constexpr StringView SubString(U64 offset = 0, U64 count = U64_MAX) const
	{
		offset = offset < length ? offset : length;
		count = count < (length - offset) ? count : length - offset;
		return { string + offset, count };
	}

	template<U64 Length>
	constexpr I64 IndexOf(const C8(&find)[Length], U64 start = 0) const
	{
		const C8* it = string + start;

		while (!(*it == 0 || CompareString(it, find, Length - 1))) { ++it; }

		if (*it == 0) { return -1; }
		return (I64)(it - string);
	}

	constexpr I64 IndexOf(C8 find, U64 start = 0) const
	{
		const C8* it = string + start;
		C8 c;

		while ((c = *it) != 0 && c != find) { ++it; }

		if (c == 0) { return -1; }
		return (I64)(it - string);
	}

	template<U64 Length>
	constexpr I64 LastIndexOf(const C8(&find)[Length], U64 start = 0) const
	{
		const C8* it = string + length - start - Length;

		U64 len = length - Length + 1;
		while (!CompareString(it, find, Length - 1))
		{
			if (--len) { --it; }
			else { return -1; }
		}

		if (len) { return (I64)(it - string); }
	}

	constexpr I64 LastIndexOf(C8 find, U64 start = 0) const
	{
		const C8* it = string + length - start - 1;

		U64 len = length;
		while (len && *it != find) { --it; --len; }

		if (len) { return (I64)(it - string); }
		return -1;
	}

	constexpr bool Empty() const { return length == 0; }
	constexpr U64 Size() const { return length; }
	constexpr const C8* Data() const { return string; }
	//constexpr U64 Hash() const { return Hash::StringHash(string, length); }
	//constexpr U64 HashCI() const { return Hash::StringHashCI(string, length); }

private:
	const C8* string;
	U64 length;
};

template<class C>
struct StringBase
{
	using CharType = C;

	StringBase();
	StringBase(const StringBase& other);
	StringBase(StringBase&& other) noexcept;

	StringBase& operator=(NullPointer);
	StringBase& operator=(const StringBase& other);
	StringBase& operator=(StringBase&& other) noexcept;

	~StringBase();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);
	void Resize();

	C* operator*();
	const C* operator*() const;
	C& operator[](U64 i);
	const C& operator[](U64 i) const;

	bool operator==(C* other) const noexcept;
	bool operator==(const StringBase& other) const noexcept;
	template<U64 Count> bool operator==(const C(&other)[Count]) const noexcept;
	bool operator!=(C* other) const noexcept;
	bool operator!=(const StringBase& other) const noexcept;
	template<U64 Count> bool operator!=(const C(&other)[Count]) const noexcept;

	bool operator<(const StringBase& other) const noexcept;
	bool operator>(const StringBase& other) const noexcept;

	operator bool() const noexcept;

	bool Compare(C* other) const noexcept;
	bool Compare(const StringBase& other) const noexcept;
	template<U64 Count> bool Compare(const C(&other)[Count]) const noexcept;
	bool CompareN(C* other, U64 start = 0) const noexcept;
	bool CompareN(const StringBase& other, U64 start = 0) const noexcept;
	template<U64 Count> bool CompareN(const C(&other)[Count], U64 start = 0) const noexcept;
	bool StartsWith(C* other) const noexcept;
	bool StartsWith(const StringBase& other) const noexcept;
	template<U64 Count> bool StartsWith(const C(&other)[Count]) const noexcept;
	bool EndsWith(C* other) const noexcept;
	bool EndsWith(const StringBase& other) const noexcept;
	template<U64 Count> bool EndsWith(const C(&other)[Count]) const noexcept;

	bool Blank() const noexcept;
	I64 IndexOf(C* find, U64 start = 0) const noexcept;
	I64 IndexOf(const C& find, U64 start = 0) const noexcept;
	I64 IndexOf(const StringBase& find, U64 start = 0) const noexcept;
	template<U64 Count> I64 IndexOf(const C(&find)[Count], U64 start = 0) const noexcept;
	I64 LastIndexOf(C* find, U64 start = 0) const noexcept;
	I64 LastIndexOf(const C& find, U64 start = 0) const noexcept;
	I64 LastIndexOf(const StringBase& find, U64 start = 0) const noexcept;
	template<U64 Count> I64 LastIndexOf(const C(&find)[Count], U64 start = 0) const noexcept;
	I64 IndexOfNot(const C& find, U64 start = 0) const noexcept;

	StringBase& Trim() noexcept;
	StringBase SubString(U64 start, U64 nLength = U64_MAX) const noexcept;

	C* begin() noexcept;
	C* end() noexcept;
	const C* begin() const noexcept;
	const C* end() const noexcept;

	C* rbegin() noexcept;
	C* rend() noexcept;
	const C* rbegin() const noexcept;
	const C* rend() const noexcept;

private:
	U64 size = 0;
	U64 capacity = 0;
	C* string = nullptr;
};

using String = StringBase<C8>;
using String8 = StringBase<C8>;
using String16 = StringBase<C16>;
using String32 = StringBase<C32>;
using StringW = StringBase<CW>;

template<class C>
inline StringBase<C>::StringBase() {}

template<class C>
inline StringBase<C>::StringBase(const StringBase& other) : size(other.size)
{
	if (!string || capacity < other.size) { Memory::Reallocate(&string, size, capacity); }

	memcpy(string, other.string, size);
	string[size] = 0;
}

template<class C>
inline StringBase<C>::StringBase(StringBase&& other) noexcept : size(other.size), capacity(other.capacity), string(other.string)
{
	other.size = 0;
	other.capacity = 0;
	other.string = nullptr;
}

template<class C>
inline StringBase<C>& StringBase<C>::operator=(NullPointer)
{
	Destroy();
}

template<class C>
inline StringBase<C>& StringBase<C>::operator=(const StringBase<C>& other) 
{
	size = other.size;

	if (!string || capacity < other.size) { Memory::Reallocate(&string, size, capacity); }

	memcpy(string, other.string, size);
	string[size] = 0;

	return *this;
}

template<class C>
inline StringBase<C>& StringBase<C>::operator=(StringBase<C>&& other) noexcept
{
	if (string) { Memory::Free(&string); }

	size = other.size;
	capacity = other.capacity;
	string = other.string;

	other.size = 0;
	other.capacity = 0;
	other.string = nullptr;

	return *this;
}

template<class C>
inline StringBase<C>::~StringBase()
{
	if (string)
	{
		size = 0;
		capacity = 0;
		Memory::Free(&string);
	}
}

template<class C>
inline void StringBase<C>::Destroy()
{
	if (string)
	{
		size = 0;
		capacity = 0;
		Memory::Free(&string);
	}
}

template<class C>
inline void StringBase<C>::Clear()
{
	if (string)
	{
		string[0] = 0;
		size = 0;
	}
}

template<class C>
inline void StringBase<C>::Reserve(U64 size)
{
	if (size + 1 > capacity)
	{
		Memory::Reallocate(&string, size, capacity);
	}
}

template<class C>
inline void StringBase<C>::Resize(U64 size)
{
	if (size + 1 > this->capacity) { Reserve(size); }
	this->size = size;
	string[size] = 0;
}

template<class C>
inline void StringBase<C>::Resize()
{
	size = Length(string);
}
