#pragma once

#include "Defines.hpp"

#include "Platform/Memory.hpp"
#include "Core/Formatting.hpp"

template<class T>
NH_API inline constexpr U64 Length(const T* str) noexcept
{
	if (!str) { return 0; }

	const T* it = str;
	while (*it) { ++it; }

	return it - str;
}

template<Character C>
NH_API inline bool WhiteSpace(C c) noexcept
{
	if constexpr (IsSame<C, C8>) { return c == ' ' || c == '\t' || c == '\v' || c == '\r' || c == '\n' || c == '\f'; }
	if constexpr (IsSame<C, C16>) { return c == u' ' || c == u'\t' || c == u'\v' || c == u'\r' || c == u'\n' || c == u'\f'; }
	if constexpr (IsSame<C, C32>) { return c == U' ' || c == U'\t' || c == U'\v' || c == U'\r' || c == U'\n' || c == U'\f'; }
	if constexpr (IsSame<C, CW>) { return c == L' ' || c == L'\t' || c == L'\v' || c == L'\r' || c == L'\n' || c == L'\f'; }
}

template<Character C>
NH_API inline bool NotWhiteSpace(C c) noexcept
{
	if constexpr (IsSame<C, C8>) { return c != ' ' && c != '\t' && c != '\v' && c != '\r' && c != '\n' && c != '\f'; }
	if constexpr (IsSame<C, C16>) { return c != u' ' && c != u'\t' && c != u'\v' && c != u'\r' && c != u'\n' && c != u'\f'; }
	if constexpr (IsSame<C, C32>) { return c != U' ' && c != U'\t' && c != U'\v' && c != U'\r' && c != U'\n' && c != U'\f'; }
	if constexpr (IsSame<C, CW>) { return c != L' ' && c != L'\t' && c != L'\v' && c != L'\r' && c != L'\n' && c != L'\f'; }
}

template<Character C>
NH_API inline bool Numerical(C c) noexcept
{
	return c > 47 && c < 58;
}

struct NH_API StringView
{
	constexpr StringView() {}

	template<U64 Length>
	constexpr StringView(const C8(&str)[Length]) : string{ str }, length{ Length } {}
	constexpr StringView(const C8* str, U64 length) : string{ str }, length{ length } {}
	constexpr StringView(const StringView& other) : string{ other.string }, length{ other.length } {}
	constexpr StringView(StringView&& other) noexcept : string{ other.string }, length{ other.length } {}

	constexpr StringView& operator=(const StringView& other) { string = other.string; length = other.length; return *this; }
	constexpr StringView& operator=(StringView&& other) noexcept { string = other.string; length = other.length; return *this; }

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

		while (!(*it == 0 || memcmp(it, find, Length - 1))) { ++it; }

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
		while (!memcmp(it, find, Length - 1))
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
	constexpr U64 Size() const { return length - 1; }
	constexpr const C8* Data() const { return string; }

private:
	const C8* string = nullptr;
	U64 length = 0;
};

struct FormatTag{} static inline constexpr FORMAT;

template<class C>
struct StringBase
{
	using CharType = C;

	StringBase();
	StringBase(const C* other);
	StringBase(const C* other, U64 size);
	StringBase(const StringBase& other);
	StringBase(StringBase&& other) noexcept;
	template<U64 Count> StringBase(const C(&other)[Count]);
	template<typename... Args> StringBase(FormatTag, Args... args);

	StringBase& operator=(NullPointer);
	StringBase& operator=(const C* other);
	StringBase& operator=(const StringBase& other);
	StringBase& operator=(StringBase&& other) noexcept;

	~StringBase();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);
	void Resize();

	bool operator==(C* other) const;
	bool operator==(const StringBase& other) const;
	template<U64 Count> bool operator==(const C(&other)[Count]) const;
	bool operator!=(C* other) const;
	bool operator!=(const StringBase& other) const;
	template<U64 Count> bool operator!=(const C(&other)[Count]) const;

	bool operator<(C* other) const;
	bool operator<(const StringBase& other) const;
	template<U64 Count> bool operator<(const C(&other)[Count]) const;
	bool operator>(C* other) const;
	bool operator>(const StringBase& other) const;
	template<U64 Count> bool operator>(const C(&other)[Count]) const;

	bool Compare(C* other) const;
	bool Compare(const StringBase& other) const;
	template<U64 Count> bool Compare(const C(&other)[Count]) const;
	bool CompareN(C* other, U64 start = 0) const;
	bool CompareN(const StringBase& other, U64 start = 0) const;
	template<U64 Count> bool CompareN(const C(&other)[Count], U64 start = 0) const;
	bool StartsWith(C* other) const;
	bool StartsWith(const StringBase& other) const;
	template<U64 Count> bool StartsWith(const C(&other)[Count]) const;
	bool EndsWith(C* other) const;
	bool EndsWith(const StringBase& other) const;
	template<U64 Count> bool EndsWith(const C(&other)[Count]) const;

	I64 IndexOf(C* find, U64 start = 0) const;
	I64 IndexOf(const C& find, U64 start = 0) const;
	I64 IndexOf(const StringBase& find, U64 start = 0) const;
	template<U64 Count> I64 IndexOf(const C(&find)[Count], U64 start = 0) const;
	I64 LastIndexOf(C* find, U64 start = 0) const;
	I64 LastIndexOf(const C& find, U64 start = 0) const;
	I64 LastIndexOf(const StringBase& find, U64 start = 0) const;
	template<U64 Count> I64 LastIndexOf(const C(&find)[Count], U64 start = 0) const;
	I64 IndexOfNot(const C& find, U64 start = 0) const;

	StringBase& Trim();
	StringBase SubString(U64 start, U64 length = U64_MAX) const;

	StringBase& Append(const C* other);
	StringBase& Append(const StringBase& other);

	C* begin();
	C* end();
	const C* begin() const;
	const C* end() const;

	C* rbegin();
	C* rend();
	const C* rbegin() const;
	const C* rend() const;

	U64 Capacity() const;
	U64 Size() const;
	C* Data() const;

	operator C* ();
	operator C* () const;

	C* operator*();
	const C* operator*() const;
	C& operator[](U64 i);
	const C& operator[](U64 i) const;

	operator bool() const;
	bool Blank() const;
	bool Empty() const;

private:
	template<class Type> U64 FormatWrite(C* str, Type type);

	U64 size = 0;
	U64 capacity = 0;
	C* string = nullptr;
};

using String = StringBase<C8>;
using String8 = StringBase<C8>;
using String16 = StringBase<C16>;
using String32 = StringBase<C32>;
using StringW = StringBase<CW>;

template<class Type> static constexpr inline bool IsStringType = IsSpecializationOf<Type, StringBase>;
template<class Type> concept StringType = IsStringType<Type>;

template<class C>
inline StringBase<C>::StringBase() {}

template<class C>
inline StringBase<C>::StringBase(const C* other)
{
	U64 otherSize = Length(other);
	size = otherSize;

	capacity = Memory::Allocate(&string, size);

	memcpy(string, other, size * sizeof(C));
	string[size] = 0;
}

template<class C>
inline StringBase<C>::StringBase(const C* other, U64 size)
{
	this->size = size;

	capacity = Memory::Allocate(&string, size);

	memcpy(string, other, size * sizeof(C));
	string[size] = 0;
}

template<class C>
inline StringBase<C>::StringBase(const StringBase& other) : size(other.size)
{
	capacity = Memory::Allocate(&string, size);

	memcpy(string, other.string, size * sizeof(C));
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
template<U64 Count>
inline StringBase<C>::StringBase(const C(&other)[Count])
{
	size = Length(other);

	capacity = Memory::Allocate(&string, size);

	memcpy(string, other, size * sizeof(C));
	string[size] = 0;
}

template<class C>
template<typename... Args>
inline StringBase<C>::StringBase(FormatTag, Args... args)
{
	constexpr U64 length = (MaxFormatLength<Args>() + ...);

	capacity = Memory::Allocate(&string, length);

	((size += FormatWrite(string + size, args)), ...);
}

template<class C>
template<class Type>
inline U64 StringBase<C>::FormatWrite(C* str, Type type)
{
	if constexpr (IsStringType<Type>)
	{
		U64 length = type.Size();
		memcpy(str, type.Data(), length * sizeof(Type::CharType));

		return length;
	}
	else if constexpr (IsSame<Type, StringView>)
	{
		U64 length = type.Size();
		memcpy(str, type.Data(), length);

		return length;
	}
	else if constexpr (IsStringLiteral<Type>)
	{
		U64 length = Length(type);
		memcpy(str, type, length);

		return length;
	}
	else
	{
		return Format(str, type);
	}
}

template<class C>
inline StringBase<C>& StringBase<C>::operator=(NullPointer)
{
	Destroy();
}

template<class C>
inline StringBase<C>& StringBase<C>::operator=(const C* other)
{
	U64 otherSize = Length(other);
	size = otherSize;

	if (!string || capacity < otherSize) { capacity = Memory::Reallocate(&string, size); }

	memcpy(string, other, size * sizeof(C));
	string[size] = 0;

	return *this;
}

template<class C>
inline StringBase<C>& StringBase<C>::operator=(const StringBase<C>& other) 
{
	size = other.size;

	if (!string || capacity < other.size) { capacity = Memory::Reallocate(&string, size); }

	memcpy(string, other.string, size * sizeof(C));
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
		capacity = Memory::Reallocate(&string, size);
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

template<class C>
inline bool StringBase<C>::operator==(C* other) const
{
	U64 otherSize = Length(other);

	if (otherSize != size) { return false; }

	const C8* p0 = string, * p1 = other;

	U64 count = size;
	while (count--) { if (*p0++ != *p1++) { return false; } }

	return true;
}

template<class C>
inline bool StringBase<C>::operator==(const StringBase& other) const
{
	if (other.size != size) { return false; }

	const C8* p0 = string, * p1 = other.string;

	U64 count = size;
	while (count--) { if (*p0++ != *p1++) { return false; } }

	return true;
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::operator==(const C(&other)[Count]) const
{
	U64 otherSize = Length(other);

	if (otherSize != size) { return false; }

	const C8* p0 = string, * p1 = other;

	U64 count = size;
	while (count--) { if (*p0++ != *p1++) { return false; } }

	return true;
}

template<class C>
inline bool StringBase<C>::operator!=(C* other) const
{
	U64 otherSize = Length(other);

	if (otherSize != size) { return true; }

	const C8* p0 = string, * p1 = other;

	U64 count = size;
	while (count--) { if (*p0++ != *p1++) { return true; } }

	return false;
}

template<class C>
inline bool StringBase<C>::operator!=(const StringBase& other) const
{
	if (other.size != size) { return true; }

	const C8* p0 = string, * p1 = other.string;

	U64 count = size;
	while (count--) { if (*p0++ != *p1++) { return true; } }

	return false;
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::operator!=(const C(&other)[Count]) const
{
	U64 otherSize = Length(other);

	if (otherSize != size) { return true; }

	const C8* p0 = string, * p1 = other;

	U64 count = size;
	while (count--) { if (*p0++ != *p1++) { return true; } }

	return false;
}

template<class C>
inline bool StringBase<C>::operator<(C* other) const
{
	if constexpr (IsSame<C, C8>) { return strcmp(string, other) < 0; }
	if constexpr (IsSame<C, CW>) { return wcscmp(string, other) < 0; }
	else
	{
		U64 otherSize = Length(other);
		const C* it0 = string;
		const C* it1 = other;

		U64 length = size < otherSize ? size : otherSize;

		while (length-- && *it0 == *it1) { ++it0; ++it1; }

		if (length == U64_MAX) { return size < otherSize; }

		return *it0 < *it1;
	}
}

template<class C>
inline bool StringBase<C>::operator<(const StringBase& other) const
{
	if constexpr (IsSame<C, C8>) { return strcmp(string, other.string) < 0; }
	if constexpr (IsSame<C, CW>) { return wcscmp(string, other.string) < 0; }
	else
	{
		const C* it0 = string;
		const C* it1 = other.string;

		U64 length = size < other.size ? size : other.size;

		while (length-- && *it0 == *it1) { ++it0; ++it1; }

		if (length == U64_MAX) { return size < other.size; }

		return *it0 < *it1;
	}
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::operator<(const C(&other)[Count]) const
{
	U64 otherSize = Length(other);

	if constexpr (IsSame<C, C8>) { return strcmp(string, other) < 0; }
	if constexpr (IsSame<C, CW>) { return wcscmp(string, other) < 0; }
	else
	{
		const C* it0 = string;
		const C* it1 = other;

		U64 length = size < otherSize ? size : otherSize;

		while (length-- && *it0 == *it1) { ++it0; ++it1; }

		if (length == U64_MAX) { return size < otherSize; }

		return *it0 < *it1;
	}
}

template<class C>
inline bool StringBase<C>::operator>(C* other) const
{
	if constexpr (IsSame<C, C8>) { return strcmp(string, other) > 0; }
	if constexpr (IsSame<C, CW>) { return wcscmp(string, other) > 0; }
	else
	{
		U64 otherSize = Length(other);
		const C* it0 = string;
		const C* it1 = other;

		U64 length = size < otherSize ? size : otherSize;

		while (length-- && *it0 == *it1) { ++it0; ++it1; }

		if (length == U64_MAX) { return size > otherSize; }

		return *it0 > *it1;
	}
}

template<class C>
inline bool StringBase<C>::operator>(const StringBase& other) const
{
	if constexpr (IsSame<C, C8>) { return strcmp(string, other.string) > 0; }
	if constexpr (IsSame<C, CW>) { return wcscmp(string, other.string) > 0; }
	else
	{
		const C* it0 = string;
		const C* it1 = other.string;

		U64 length = size < other.size ? size : other.size;

		while (length-- && *it0 == *it1) { ++it0; ++it1; }

		if (length == U64_MAX) { return size > other.size; }

		return *it0 > *it1;
	}
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::operator>(const C(&other)[Count]) const
{
	U64 otherSize = Length(other);

	if constexpr (IsSame<C, C8>) { return strcmp(string, other) > 0; }
	if constexpr (IsSame<C, CW>) { return wcscmp(string, other) > 0; }
	else
	{
		const C* it0 = string;
		const C* it1 = other;

		U64 length = size < otherSize ? size : otherSize;

		while (length-- && *it0 == *it1) { ++it0; ++it1; }

		if (length == U64_MAX) { return size > otherSize; }

		return *it0 > *it1;
	}
}





template<class C>
inline I64 StringBase<C>::IndexOf(C* find, U64 start) const
{
	U64 findSize = Length(find);
	C* it = string + start;

	while (*it != 0 && !memcmp(it, find, findSize)) { ++it; }

	if (*it == 0) { return -1; }
	return (I64)(it - string);
}

template<class C>
inline I64 StringBase<C>::IndexOf(const C& find, U64 start) const
{
	C* it = string + start;
	C c;

	while ((c = *it) != 0 && c != find) { ++it; }

	if (c == 0) { return -1; }
	return (I64)(it - string);
}

template<class C>
inline I64 StringBase<C>::IndexOf(const StringBase& find, U64 start) const
{
	C* it = string + start;

	while (*it != 0 && !memcmp(it, find.string, find.size)) { ++it; }

	if (*it == 0) { return -1; }
	return (I64)(it - string);
}

template<class C>
template<U64 Count>
inline I64 StringBase<C>::IndexOf(const C(&find)[Count], U64 start) const
{
	C* it = string + start;

	while (*it != 0 && !memcmp(it, find, Count - 1)) { ++it; }

	if (*it == 0) { return -1; }
	return (I64)(it - string);
}

template<class C>
inline I64 StringBase<C>::LastIndexOf(C* find, U64 start) const
{
	U64 findSize = Length(find);
	C* it = string + (size - start - findSize);

	U64 len = size;
	while (len && !memcmp(it, find, findSize)) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<class C>
inline I64 StringBase<C>::LastIndexOf(const C& find, U64 start) const
{
	C* it = string + (size - start - 1);

	U64 len = size;
	while (len && *it != find) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<class C>
inline I64 StringBase<C>::LastIndexOf(const StringBase& find, U64 start) const
{
	C* it = string + (size - start - find.size);

	U64 len = size;
	while (len && !memcmp(it, find.string, find.size)) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<class C>
template<U64 Count>
inline I64 StringBase<C>::LastIndexOf(const C(&find)[Count], U64 start) const
{
	C* it = string + (size - start - Count + 1);

	U64 len = size;
	while (len && !memcmp(it, find, Count - 1)) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<class C>
inline I64 StringBase<C>::IndexOfNot(const C& find, U64 start) const
{
	C* it = string + start;
	C c;

	while ((c = *it) != 0 && c == find) { ++it; }

	if (c == 0) { return -1; }
	return (I64)(it - string);
}

template<class C>
inline StringBase<C>& StringBase<C>::Trim()
{
	C* start = string;
	C* end = string + size - 1;
	C c;

	//TODO: Verify this works
	while (WhiteSpace(c = *start)) { ++start; }
	while (WhiteSpace(c = *end)) { --end; }

	size = end - start + 1;
	memcpy(string, start, size * sizeof(C));
	string[size] = 0;

	return *this;
}

template<class C>
inline StringBase<C> StringBase<C>::SubString(U64 start, U64 length) const
{
	StringBase<C> str;

	if (length < U64_MAX) { str.Resize(length); }
	else { str.Resize(size - start); }

	memcpy(str.string, string + start, str.size * sizeof(C));
	str.string[str.size] = 0;

	return str;
}

template<class C>
inline StringBase<C>& StringBase<C>::Append(const C* other)
{
	U64 otherSize = Length(other);

	if (capacity < size + otherSize) { capacity = Memory::Reallocate(&string, size + otherSize); }

	memcpy(string + size, other, otherSize * sizeof(C));
	size += otherSize;

	return *this;
}

template<class C>
inline StringBase<C>& StringBase<C>::Append(const StringBase<C>& other)
{
	if (capacity < size + other.size) { capacity = Memory::Reallocate(&string, size + other.size); }

	memcpy(string + size, other.string, other.size * sizeof(C));
	size += other.size;

	return *this;
}

template<class C>
inline C* StringBase<C>::begin()
{
	return string;
}

template<class C>
inline C* StringBase<C>::end()
{
	return string + size;
}

template<class C>
inline const C* StringBase<C>::begin() const
{
	return string;
}

template<class C>
inline const C* StringBase<C>::end() const
{
	return string + size;
}

template<class C>
inline C* StringBase<C>::rbegin()
{
	return string + size - 1;
}

template<class C>
inline C* StringBase<C>::rend()
{
	return string - 1;
}

template<class C>
inline const C* StringBase<C>::rbegin() const
{
	return string + size - 1;
}

template<class C>
inline const C* StringBase<C>::rend() const
{
	return string - 1;
}

template<class C>
inline U64 StringBase<C>::Capacity() const
{
	return capacity;
}

template<class C>
inline U64 StringBase<C>::Size() const
{
	return size;
}

template<class C>
inline C* StringBase<C>::Data() const
{
	return string;
}

template<class C>
inline StringBase<C>::operator C* ()
{
	return string;
}

template<class C>
inline StringBase<C>::operator C* () const
{
	return string;
}

template<class C>
inline C* StringBase<C>::operator*()
{
	return string;
}

template<class C>
inline const C* StringBase<C>::operator*() const
{
	return string;
}

template<class C>
inline C& StringBase<C>::operator[](U64 i)
{
	return string[i];
}

template<class C>
inline const C& StringBase<C>::operator[](U64 i) const
{
	return string[i];
}

template<class C>
inline StringBase<C>::operator bool() const
{
	return size;
}

template<class C>
inline bool StringBase<C>::Blank() const
{
	if (size == 0) { return true; }
	C* it = string;
	C c;

	while (WhiteSpace(c = *it++));

	return c == 0;
}

template<class C>
inline bool StringBase<C>::Empty() const
{
	return size == 0;
}
