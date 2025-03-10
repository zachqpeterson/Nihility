#pragma once

#include "Defines.hpp"

#include "Platform\Memory.hpp"

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