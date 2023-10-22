#pragma once

#include "ContainerDefines.hpp"

#include "Vector.hpp"
#include "Memory\Memory.hpp"
#include "Math\Hash.hpp"
#include "Math\Random.hpp"

template<Character C> struct StringBase;

using String = StringBase<C8>;
using String8 = StringBase<C8>;
using String16 = StringBase<C16>;
using String32 = StringBase<C32>;

template <class Type> inline constexpr bool IsStringType = AnyOf<RemovedQuals<Type>, StringBase<C8>, StringBase<C16>, StringBase<C32>>;
template <class Type> concept StringType = AnyOf<RemovedQuals<Type>, StringBase<C8>, StringBase<C16>, StringBase<C32>>;
template <class Type> inline constexpr bool IsNonStringPointer = IsPointer<Type> && !IsStringLiteral<Type>;
template <class Type> concept NonStringPointer = IsPointer<Type> && !IsStringLiteral<Type>;
template <class Type> inline constexpr bool IsNonStringClass = IsClass<Type> && !IsStringType<Type>;
template <class Type> concept NonStringClass = IsClass<Type> && !IsStringType<Type>;

template<Character C>
constexpr inline U64 Length(const C* str) noexcept
{
	if (!str) { return 0; }

	const C* it = str;
	while (*it) { ++it; }

	return it - str;
}

constexpr inline U64 Length(NullPointer) noexcept
{
	return 0;
}

template<Character C>
static inline bool Compare(const C* a, const C* b, I64 length) noexcept
{
	const C* it0 = a;
	const C* it1 = b;
	C c0;
	C c1;

	while (length-- && (c0 = *it0++) == (c1 = *it1++));

	return (length + 1) == 0;
}

template<Character C>
static inline bool Compare(const C* a, const C* b) noexcept
{
	const C* it0 = a;
	const C* it1 = b;
	C c0;
	C c1;

	while ((c0 = *it0++) == (c1 = *it1++) && c0 && c1);

	return !(c0 || c1);
}

/*
* TODO: Documentation
*
* TODO: Predicates / regex?
*
* TODO: Count of a character
*
* TODO: Conversions
*
* TODO: Option to add 0x prefix to {h}
*/
template<Character C>
struct StringBase
{
	using CharType = C;
	using StringBaseType = StringBase<C>;

	static StringBase RandomString(U32 length) noexcept;

	StringBase() noexcept;
	StringBase(const StringBase& other) noexcept;
	StringBase(StringBase&& other) noexcept;
	template<typename First, typename... Args> StringBase(const First& first, const Args& ... args) noexcept;
	template<typename... Args> StringBase& Format(const C* format, const Args& ... args) noexcept; //TODO: Take in any string literal type
	template<typename... Args> StringBase& Format(U64 start, const C* format, const Args& ... args) noexcept; //TODO: Take in any string literal type

	StringBase& operator=(NullPointer) noexcept;
	StringBase& operator=(const StringBase& other) noexcept;
	StringBase& operator=(StringBase&& other) noexcept;
	template<typename Arg> StringBase& operator=(const Arg& value) noexcept;
	template<typename Arg> StringBase& operator+=(const Arg& value) noexcept;

	template<typename Arg> StringBase operator+(const Arg& value) const noexcept;

	~StringBase() noexcept;
	void Destroy() noexcept;
	void Clear() noexcept;

	void Reserve(U64 size) noexcept;
	void Resize(U64 size) noexcept;
	void Resize() noexcept;

	template<Signed Arg> Arg ToType(U64 start = 0) const noexcept;
	template<Unsigned Arg> Arg ToType(U64 start = 0) const noexcept;
	template<Boolean Arg> Arg ToType(U64 start = 0) const noexcept;
	template<FloatingPoint Arg> Arg ToType(U64 start = 0) const noexcept;
	template<NonStringPointer Arg> Arg ToType(U64 start = 0) const noexcept;
	template<Character Arg> Arg ToType(U64 start = 0) const noexcept;
	template<StringLiteral Arg> Arg ToType(U64 start = 0) const noexcept;
	template<StringType Arg> Arg ToType(U64 start = 0) const noexcept;

	C* operator*() noexcept;
	const C* operator*() const noexcept;
	C& operator[](U64 i) noexcept;
	const C& operator[](U64 i) const noexcept;

	bool operator==(C* other) const noexcept;
	bool operator==(const StringBase& other) const noexcept;
	template<U64 Count> bool operator==(const C(&other)[Count]) const noexcept;
	bool operator!=(C* other) const noexcept;
	bool operator!=(const StringBase& other) const noexcept;
	template<U64 Count> bool operator!=(const C(&other)[Count]) const noexcept;

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

	StringBase& Trim() noexcept;
	template<typename Arg> StringBase& Append(const Arg& append) noexcept;
	template<typename Arg> StringBase& Prepend(const Arg& prepend) noexcept;
	template<typename PreArg, typename PostArg> StringBase& Surround(const PreArg& prepend, const PostArg& append) noexcept;
	template<typename Arg> StringBase& Insert(const Arg& value, U64 i) noexcept;
	template<typename Arg> StringBase& Overwrite(const Arg& value, U64 i = 0) noexcept;
	template<typename Arg> StringBase& ReplaceAll(const C* find, const Arg& replace, U64 start = 0) noexcept;
	template<typename Arg> StringBase& ReplaceN(const C* find, const Arg& replace, U64 count, U64 start = 0) noexcept;
	template<typename Arg> StringBase& Replace(const C* find, const Arg& replace, U64 start = 0) noexcept;

	StringBase GetFileName() const noexcept;
	StringBase SubString(U64 start, U64 nLength = U64_MAX) const noexcept;
	template<typename Arg> StringBase Appended(const Arg& append) const noexcept;
	template<typename Arg> StringBase Prepended(const Arg& prepend) const noexcept;
	template<typename PreArg, typename PostArg> StringBase Surrounded(const PreArg& prepend, const PostArg& append) const noexcept;
	Vector<StringBase> Split(C delimiter, bool trimEntries) const noexcept;

	StringBase& ToUpper() noexcept;
	StringBase& ToLower() noexcept;

	U64 Size() const noexcept;
	U64 Capacity() const noexcept;
	U64 Hash() noexcept;
	U64 Hash() const noexcept;
	C* Data() noexcept;
	const C* Data() const noexcept;
	operator C* () noexcept;
	operator const C* () const noexcept;

	C Front() const noexcept;
	C Back() const noexcept;
	C PopBack() noexcept;

	C* begin() noexcept;
	C* end() noexcept;
	const C* begin() const noexcept;
	const C* end() const noexcept;

	C* rbegin() noexcept;
	C* rend() noexcept;
	const C* rbegin() const noexcept;
	const C* rend() const noexcept;

private:
	template<Signed Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value) noexcept;
	template<Unsigned Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value) noexcept;
	template<Boolean Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value) noexcept;
	template<FloatingPoint Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value, U64 decimalCount = 5) noexcept;
	template<NonStringPointer Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value) noexcept;
	template<Character Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value) noexcept;
	template<StringLiteral Arg, bool Hex, bool Insert, U64 Remove = 0, U64 Size = 0> U64 ToString(C* str, const Arg& value) noexcept;
	template<StringType Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value) noexcept;
	template<NonStringClass Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value) noexcept;

	template<typename Arg, bool Hex> static constexpr U64 RequiredCapacity() noexcept;

	template<typename Arg> void FindFormat(U64& start, const Arg& value) noexcept;

	static bool Compare(const C* a, const C* b, I64 length) noexcept;
	static bool WhiteSpace(C c) noexcept;
	static bool NotWhiteSpace(C c) noexcept;

	bool needHash{ true };
	U64 hash{ 0 };
	U64 size{ 0 };
	U64 capacity{ 0 };
	C* string{ nullptr };
};

template<Character C>
inline StringBase<C> StringBase<C>::RandomString(U32 length) noexcept
{
	String str{ };
	str.Resize(16);

	C* it = str.string;

	for (U32 i = 0; i < length; ++i)
	{
		*it++ = StringLookup<C>::ALPHANUM_LOOKUP[Random::RandomRange(0, Length(StringLookup<C>::ALPHANUM_LOOKUP))];
	}

	return str;
}

template<Character C>
inline StringBase<C>::StringBase() noexcept {}

template<Character C>
inline StringBase<C>::StringBase(const StringBase& other) noexcept : needHash{ other.needHash }, hash{ other.hash }, size{ other.size }
{
	if (!string || capacity < other.size) { Memory::Reallocate(&string, size, capacity); }

	Memory::Copy(string, other.string, size * sizeof(C));
	string[size] = StringLookup<C>::NULL_CHAR;
}

template<Character C>
inline StringBase<C>::StringBase(StringBase&& other) noexcept : needHash{ other.needHash }, hash{ other.hash }, size{ other.size }, capacity{ other.capacity }, string{ other.string }
{
	other.hash = 0;
	other.size = 0;
	other.capacity = 0;
	other.string = nullptr;
}

template<Character C>
template<typename First, typename... Args>
inline StringBase<C>::StringBase(const First& first, const Args& ... args) noexcept
{
	Memory::AllocateArray(&string, capacity, capacity);
	ToString<First, false, false>(string, first);
	(ToString<Args, false, false>(string + size, args), ...);
}

template<Character C>
template<typename... Args>
inline StringBase<C>& StringBase<C>::Format(const C* format, const Args& ... args) noexcept
{
	U64 length = Length(format) + 1;

	if (capacity < length) { Memory::Reallocate(&string, length, capacity); }
	size = length - 1;

	Memory::Copy(string, format, length * sizeof(C));
	U64 start = 0;
	(FindFormat(start, args), ...);

	return *this;
}

template<Character C>
template<typename... Args>
inline StringBase<C>& StringBase<C>::Format(U64 start, const C* format, const Args& ... args) noexcept
{
	U64 length = Length(format) + 1;

	if (capacity < start + length) { Memory::Reallocate(&string, start + length, capacity); }
	size = start + length - 1;

	Memory::Copy(string + start, format, length * sizeof(C));
	(FindFormat(start, args), ...);

	return *this;
}

template<Character C>
inline StringBase<C>& StringBase<C>::operator=(NullPointer) noexcept
{
	Destroy();
}

template<Character C>
inline StringBase<C>& StringBase<C>::operator=(const StringBase& other) noexcept
{
	hash = other.hash;
	size = other.size;

	if (!string || capacity < other.size) { Memory::Reallocate(&string, size, capacity); }

	Memory::Copy(string, other.string, size * sizeof(C));
	string[size] = StringLookup<C>::NULL_CHAR;

	return *this;
}

template<Character C>
inline StringBase<C>& StringBase<C>::operator=(StringBase&& other) noexcept
{
	if(string) { Memory::Free(&string); }

	hash = other.hash;
	size = other.size;
	capacity = other.capacity;
	string = other.string;

	other.hash = 0;
	other.size = 0;
	other.capacity = 0;
	other.string = nullptr;

	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::operator=(const Arg& value) noexcept
{
	ToString<Arg, false, true, U64_MAX>(string, value);
	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::operator+=(const Arg& value) noexcept
{
	ToString<Arg, false, false>(string + size, value);
	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C> StringBase<C>::operator+(const Arg& value) const noexcept
{
	StringBase<C> copy = *this;
	copy += value;

	return Move(copy);
}

template<Character C>
inline StringBase<C>::~StringBase() noexcept
{
	hash = 0;
	if (string)
	{
		size = 0;
		capacity = 0;
		Memory::Free(&string);
	}
}

template<Character C>
inline void StringBase<C>::Destroy() noexcept
{
	hash = 0;
	if (string)
	{
		size = 0;
		capacity = 0;
		Memory::Free(&string);
	}
}

template<Character C>
inline void StringBase<C>::Clear() noexcept
{
	if (string)
	{
		string[0] = StringLookup<C>::NULL_CHAR;
		size = 0;
		hash = 0;
		needHash = false;
	}
}

template<Character C>
inline void StringBase<C>::Reserve(U64 size) noexcept
{
	if (size + 1 > capacity)
	{
		Memory::Reallocate(&string, size, capacity);
	}
}

template<Character C>
inline void StringBase<C>::Resize(U64 size) noexcept
{
	if (size + 1 > this->capacity) { Reserve(size); }
	this->size = size;
	string[size] = StringLookup<C>::NULL_CHAR;
	needHash = true;
}

template<Character C>
inline void StringBase<C>::Resize() noexcept
{
	size = Length(string);
	needHash = true;
}

template<Character C>
inline C* StringBase<C>::operator*() noexcept { return string; }

template<Character C>
inline const C* StringBase<C>::operator*() const noexcept { return string; }

template<Character C>
inline C& StringBase<C>::operator[](U64 i) noexcept { return string[i]; }

template<Character C>
inline const C& StringBase<C>::operator[](U64 i) const noexcept { return string[i]; }

template<Character C>
inline bool StringBase<C>::operator==(C* other) const noexcept
{
	U64 len = Length(other);
	if (len != size) { return false; }

	return Compare(string, other, size);
}

template<Character C>
inline bool StringBase<C>::operator==(const StringBase<C>& other) const noexcept
{
	if (other.size != size) { return false; }

	return Compare(string, other.string, size);
}

template<Character C>
template<U64 Count>
inline bool StringBase<C>::operator==(const C(&other)[Count]) const noexcept
{
	if (Count - 1 != size) { return false; }

	return Compare(string, other, Count - 1);
}

template<Character C>
inline bool StringBase<C>::operator!=(C* other) const noexcept
{
	U64 len = Length(other);
	if (len != size) { return true; }

	return !Compare(string, other, size);
}

template<Character C>
inline bool StringBase<C>::operator!=(const StringBase<C>& other) const noexcept
{
	if (other.size != size) { return true; }

	return !Compare(string, other.string, size);
}

template<Character C>
template<U64 Count>
inline bool StringBase<C>::operator!=(const C(&other)[Count]) const noexcept
{
	if (Count - 1 != size) { return true; }

	return !Compare(string, other, Count - 1);
}

template<Character C>
inline bool StringBase<C>::Compare(C* other) const noexcept
{
	U64 len = Length(other);
	if (len != size) { return false; }

	return Compare(string, other, size);
}

template<Character C>
inline bool StringBase<C>::Compare(const StringBase<C>& other) const noexcept
{
	if (other.size != size) { return false; }

	return Compare(string, other.string, size);
}

template<Character C>
template<U64 Count>
inline bool StringBase<C>::Compare(const C(&other)[Count]) const noexcept
{
	if (Count - 1 != size) { return false; }

	return Compare(string, other, Count - 1);
}

template<Character C>
inline bool StringBase<C>::CompareN(C* other, U64 start) const noexcept
{
	U64 len = Length(other);

	return Compare(string + start, other, len);
}

template<Character C>
inline bool StringBase<C>::CompareN(const StringBase<C>& other, U64 start) const noexcept
{
	return Compare(string + start, other.string);
}

template<Character C>
template<U64 Count>
inline bool StringBase<C>::CompareN(const C(&other)[Count], U64 start) const noexcept
{
	return Compare(string + start, other, Count - 1);
}

template<Character C>
inline bool StringBase<C>::StartsWith(C* other) const noexcept
{
	U64 otherSize = Length(other);

	return Compare(string, other, otherSize);
}

template<Character C>
inline bool StringBase<C>::StartsWith(const StringBase& other) const noexcept
{
	return Compare(string, other.string, other.size);
}

template<Character C>
template<U64 Count>
inline bool StringBase<C>::StartsWith(const C(&other)[Count]) const noexcept
{
	return Compare(string, other, Count - 1);
}

template<Character C>
inline bool StringBase<C>::EndsWith(C* other) const noexcept
{
	U64 otherSize = Length(other);

	return Compare(string + (size - otherSize), other, otherSize);
}

template<Character C>
inline bool StringBase<C>::EndsWith(const StringBase& other) const noexcept
{
	return Compare(string + (size - other.size), other.string, other.size);
}

template<Character C>
template<U64 Count>
inline bool StringBase<C>::EndsWith(const C(&other)[Count]) const noexcept
{
	return Compare(string + (size - Count - 1), other, Count - 1);
}

template<Character C>
inline U64 StringBase<C>::Size() const noexcept { return size; }

template<Character C>
inline U64 StringBase<C>::Capacity() const noexcept { return capacity; }

template<Character C>
inline U64 StringBase<C>::Hash() noexcept
{
	if (needHash)
	{
		needHash = false;
		hash = Hash::Calculate(string);
	}
	else
	{
		return hash;
	}
}

template<Character C>
inline U64 StringBase<C>::Hash() const noexcept
{
	if (needHash) { return Hash::Calculate(string, size); }
	else { return hash; }
}

template<Character C>
inline C* StringBase<C>::Data() noexcept { return string; }

template<Character C>
inline const C* StringBase<C>::Data() const noexcept { return string; }

template<Character C>
inline StringBase<C>::operator C* () noexcept { return string; }

template<Character C>
inline StringBase<C>::operator const C* () const noexcept { return string; }

template<Character C>
inline C StringBase<C>::Front() const noexcept { return *string; }

template<Character C>
inline C StringBase<C>::Back() const noexcept { return string[size - 1]; }

template<Character C>
inline C StringBase<C>::PopBack() noexcept { return string[size-- - 1]; }

template<Character C>
inline bool StringBase<C>::Blank() const noexcept
{
	if (size == 0) { return true; }
	C* it = string;
	C c;

	while (WhiteSpace(c = *it++));

	return c == StringLookup<C>::NULL_CHAR;
}

template<Character C>
inline I64 StringBase<C>::IndexOf(C* find, U64 start) const noexcept
{
	U64 findSize = Length(find);
	C* it = string + start;

	while (*it != StringLookup<C>::NULL_CHAR && Compare(it, find, findSize)) { ++it; }

	if (*it == StringLookup<C>::NULL_CHAR) { return -1; }
	return (I64)(it - string);
}

template<Character C>
inline I64 StringBase<C>::IndexOf(const C& find, U64 start) const noexcept
{
	C* it = string + start;
	C c;

	while ((c = *it) != StringLookup<C>::NULL_CHAR && c != find) { ++it; }

	if (c == StringLookup<C>::NULL_CHAR) { return -1; }
	return (I64)(it - string);
}

template<Character C>
inline I64 StringBase<C>::IndexOf(const StringBase& find, U64 start) const noexcept
{
	C* it = string + start;

	while (*it != StringLookup<C>::NULL_CHAR && Compare(it, find.string, find.size)) { ++it; }

	if (*it == StringLookup<C>::NULL_CHAR) { return -1; }
	return (I64)(it - string);
}

template<Character C>
template<U64 Count> 
inline I64 StringBase<C>::IndexOf(const C(&find)[Count], U64 start) const noexcept
{
	C* it = string + start;

	while (*it != StringLookup<C>::NULL_CHAR && Compare(it, find, Count)) { ++it; }

	if (*it == StringLookup<C>::NULL_CHAR) { return -1; }
	return (I64)(it - string);
}

template<Character C>
inline I64 StringBase<C>::LastIndexOf(C* find, U64 start) const noexcept
{
	U64 findSize = Length(find);
	C* it = string + size - start - findSize - 1;

	U64 len = size;
	while (len && Compare(it, find, findSize)) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<Character C>
inline I64 StringBase<C>::LastIndexOf(const C& find, U64 start) const noexcept
{
	C* it = string + size - start - 1;

	U64 len = size;
	while (len && *it != find) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<Character C>
inline I64 StringBase<C>::LastIndexOf(const StringBase& find, U64 start) const noexcept
{
	C* it = string + size - start - find.size - 1;

	U64 len = size;
	while (len && Compare(it, find.string, find.size)) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<Character C>
template<U64 Count> 
inline I64 StringBase<C>::LastIndexOf(const C(&find)[Count], U64 start) const noexcept
{
	C* it = string + size - start - Count - 1;

	U64 len = size;
	while (len && Compare(it, find, Count)) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<Character C>
inline StringBase<C>& StringBase<C>::Trim() noexcept
{
	C* start = string;
	C* end = string + size - 1;
	C c;

	//TODO: Verify this works
	while (WhiteSpace(c = *start)) { ++start; }
	while (WhiteSpace(c = *end)) { --end; }

	size = end - start + 1;
	Memory::Copy(string, start, size);
	string[size] = StringLookup<C>::NULL_CHAR;
	needHash = true;

	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Append(const Arg& append) noexcept
{
	ToString<Arg, false, false>(string + size, append);
	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Prepend(const Arg& prepend) noexcept
{
	ToString<Arg, false, true>(string, prepend);
	return *this;
}

template<Character C>
template<typename PreArg, typename PostArg>
inline StringBase<C>& StringBase<C>::Surround(const PreArg& prepend, const PostArg& append) noexcept
{
	ToString<PreArg, false, true>(string, prepend);
	ToString<PostArg, false, false>(string + size, append);
	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Insert(const Arg& value, U64 i) noexcept
{
	ToString<Arg, false, true>(string + i, value);
	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Overwrite(const Arg& value, U64 i) noexcept
{
	ToString<Arg, false, false>(string + i, value);
	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::ReplaceAll(const C* find, const Arg& replace, U64 start) noexcept
{
	U64 findSize = Length(find);
	C* it = string + start;
	C c = *it;

	while (c != StringLookup<C>::NULL_CHAR)
	{
		while ((c = *it) != StringLookup<C>::NULL_CHAR && Compare(it, find, findSize)) { ++it; }

		if (c != StringLookup<C>::NULL_CHAR) { ToString<Arg, false, true>(it, replace); }
	}

	string[size] = StringLookup<C>::NULL_CHAR;
	needHash = true;

	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::ReplaceN(const C* find, const Arg& replace, U64 count, U64 start) noexcept
{
	U64 findSize = Length(find);
	C* it = string + start;
	C c = *it;

	while (c != StringLookup<C>::NULL_CHAR && count)
	{
		while ((c = *it) != StringLookup<C>::NULL_CHAR && Compare(it, find, findSize)) { ++it; }

		if (c != StringLookup<C>::NULL_CHAR)
		{
			--count;
			ToString<Arg, false, true>(it, replace);
		}
	}

	string[size] = StringLookup<C>::NULL_CHAR;
	needHash = true;

	return *this;
}

template<Character C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Replace(const C* find, const Arg& replace, U64 start) noexcept
{
	U64 findSize = Length(find);
	C* it = string + start;
	C c;

	while ((c = *it) != StringLookup<C>::NULL_CHAR && Compare(it, find, findSize)) { ++it; }

	if (c != StringLookup<C>::NULL_CHAR) { ToString<Arg, false, true>(c, replace); }

	string[size] = StringLookup<C>::NULL_CHAR;
	needHash = true;

	return *this;
}

template<Character C>
inline StringBase<C> StringBase<C>::GetFileName() const noexcept
{
	I64 index;
	I64 extIndex = LastIndexOf(StringLookup<C>::DECIMAL_CHAR);

	if ((index = LastIndexOf(StringLookup<C>::FORWARD_SLASH) + 1) != -1) { return Move(SubString(index, extIndex - index)); }
	if ((index = LastIndexOf(StringLookup<C>::BACK_SLASH) + 1) != -1) { return Move(SubString(index, extIndex - index)); }

	return Move(SubString(0, extIndex));
}

template<Character C>
inline StringBase<C> StringBase<C>::SubString(U64 start, U64 nLength) const noexcept
{
	StringBase<C> str;

	if (nLength < U64_MAX) { str.Resize(nLength); }
	else { str.Resize(size - start); }

	Memory::Copy(str.string, string + start, str.size);
	str.string[str.size] = StringLookup<C>::NULL_CHAR;

	return Move(str);
}

template<Character C>
template<typename Arg>
inline StringBase<C> StringBase<C>::Appended(const Arg& append) const noexcept
{
	StringBase<C> str;

	str = *this;
	str.Append(append);

	return Move(str);
}

template<Character C>
template<typename Arg>
inline StringBase<C> StringBase<C>::Prepended(const Arg& prepend) const noexcept
{
	StringBase<C> str;

	str = *this;
	str.Prepend(prepend);

	return Move(str);
}

template<Character C>
template<typename PreArg, typename PostArg>
inline StringBase<C> StringBase<C>::Surrounded(const PreArg& prepend, const PostArg& append) const noexcept
{
	StringBase<C> str;

	str = *this;
	str.Surround(prepend, append);

	return Move(str);
}

template<Character C>
inline Vector<StringBase<C>> StringBase<C>::Split(C delimiter, bool trimEntries) const noexcept
{
	//TODO: this
}

template<Character C>
inline StringBase<C>& StringBase<C>::ToUpper() noexcept
{
	static_assert(IsSame<C, char>, "ToUpper is only supported for char strings currently");

	for (char& c : *this)
	{
		if (StringLookup<C>::TYPE_LOOKUP[c] & LOWER_CHAR) { c -= 32; }
	}

	needHash = true;

	return *this;
}

template<Character C>
inline StringBase<C>& StringBase<C>::ToLower() noexcept
{
	static_assert(IsSame<C, char>, "ToLower is only supported for char strings currently");

	for (char& c : *this)
	{
		if (StringLookup<C>::TYPE_LOOKUP[c] & UPPER_CHAR) { c += 32; }
	}

	needHash = true;

	return *this;
}

template<Character C>
inline C* StringBase<C>::begin() noexcept { return string; }

template<Character C>
inline C* StringBase<C>::end() noexcept { return string + size; }

template<Character C>
inline const C* StringBase<C>::begin() const noexcept { return string; }

template<Character C>
inline const C* StringBase<C>::end() const noexcept { return string + size; }

template<Character C>
inline C* StringBase<C>::rbegin() noexcept { return string + size - 1; }

template<Character C>
inline C* StringBase<C>::rend() noexcept { return string - 1; }

template<Character C>
inline const C* StringBase<C>::rbegin() const noexcept { return string + size - 1; }

template<Character C>
inline const C* StringBase<C>::rend() const noexcept { return string - 1; }

template<Character C>
template<Signed Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value) noexcept
{
	constexpr U64 typeSize = RequiredCapacity<Arg, Hex>();
	constexpr U64 moveSize = typeSize - Remove;
	const U64 strIndex = str - string;
	const U64 excessSize = size - strIndex;

	using UArg = Traits<UnsignedOf<Arg>>::Base;

	if (!string || capacity < size + moveSize) { Memory::Reallocate(&string, size + moveSize, capacity); str = string + strIndex; }
	if constexpr (Insert) { Memory::Copy(str + moveSize, str, excessSize * sizeof(C)); }

	C* c = str + typeSize;
	const C* digits;
	I64 addLength;
	UArg val;
	U64 neg;
	if (value < 0)
	{
		if constexpr (Hex) { val = Traits<UnsignedOf<Arg>>::MaxValue - ((U64)-value - 1); }
		else
		{
			str[0] = StringLookup<C>::NEGATIVE_CHAR;
			val = (UArg)-value;
			neg = 1;
		}
	}
	else { val = (UArg)value; neg = 0; }

	if constexpr (Hex)
	{
		constexpr U64 pairs = typeSize / 2;

		for (U8 i = 0; i < pairs; ++i)
		{
			digits = StringLookup<C>::HEX_LOOKUP + (val & 0xFF) * 2;

			*--c = digits[1];
			*--c = digits[0];

			val >>= 8;
		}

		addLength = typeSize;
	}
	else
	{
		while (val > 999)
		{
			UArg newVal = val / 1000;
			UArg remainder = val % 1000;
			digits = StringLookup<C>::DECIMAL_LOOKUP + (remainder * 3);
			*--c = digits[2];
			*--c = digits[1];
			*--c = digits[0];
			val = newVal;
		}

		digits = StringLookup<C>::DECIMAL_LOOKUP + (val * 3);
		*--c = digits[2];
		if (val > 9) { *--c = digits[1]; }
		if (val > 99) { *--c = digits[0]; }

		addLength = typeSize - (c - str) + neg;
	}

	size += addLength - Remove;

	if constexpr (Insert && !Hex) { Memory::Copy(str + neg, c, (addLength + excessSize - Remove) * sizeof(C)); }
	else { Memory::Copy(str, c, addLength * sizeof(C)); }

	string[size] = StringLookup<C>::NULL_CHAR;
	needHash = true;

	return strIndex + addLength;
}

template<Character C>
template<Unsigned Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value) noexcept
{
	constexpr U64 typeSize = RequiredCapacity<Arg, Hex>();
	constexpr U64 moveSize = typeSize - Remove;
	const U64 strIndex = str - string;
	const U64 excessSize = size - strIndex;

	if (!string || capacity < size + moveSize) { Memory::Reallocate(&string, size + moveSize, capacity); str = string + strIndex; }
	if constexpr (Insert) { Memory::Copy(str + moveSize, str, excessSize * sizeof(C)); }

	C* c = str + typeSize;
	const C* digits;
	U64 val = value;
	I64 addLength;

	if constexpr (Hex)
	{
		constexpr U64 pairs = typeSize / 2;

		for (U8 i = 0; i < pairs; ++i)
		{
			digits = StringLookup<C>::HEX_LOOKUP + (val & 0xFF) * 2;

			*--c = digits[1];
			*--c = digits[0];

			val >>= 8;
		}

		addLength = typeSize;
	}
	else
	{
		while (val > 999)
		{
			U64 newVal = val / 1000;
			U64 remainder = val % 1000;
			digits = StringLookup<C>::DECIMAL_LOOKUP + (remainder * 3);
			*--c = digits[2];
			*--c = digits[1];
			*--c = digits[0];
			val = newVal;
		}

		digits = StringLookup<C>::DECIMAL_LOOKUP + (val * 3);
		*--c = digits[2];
		if (val > 9) { *--c = digits[1]; }
		if (val > 99) { *--c = digits[0]; }

		addLength = typeSize - (c - str);
	}

	size += addLength - Remove;

	if constexpr (Insert && !Hex) { Memory::Copy(str, c, (addLength + excessSize - Remove) * sizeof(C)); }
	else { Memory::Copy(str, c, addLength * sizeof(C)); }

	string[size] = StringLookup<C>::NULL_CHAR;
	needHash = true;

	return strIndex + addLength;
}

template<Character C>
template<Boolean Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value) noexcept
{
	constexpr U64 trueSize = 5 - Remove;
	constexpr U64 falseSize = 6 - Remove;
	const U64 strIndex = str - string;

	if (value)
	{
		if (!string || capacity < size + trueSize) { Memory::Reallocate(&string, size + trueSize, capacity); str = string + strIndex; }

		if constexpr (Insert) { Copy(str + 4, str, size - strIndex); }

		Copy(str, StringLookup<C>::TRUE_STR, 4);
		size += 4;

		if constexpr (!Insert) { string[size] = StringLookup<C>::NULL_CHAR; }
		needHash = true;

		return strIndex + 4;
	}
	else
	{
		if (!string || capacity < size + falseSize) { Memory::Reallocate(&string, size + falseSize, capacity); str = string + strIndex; }

		if constexpr (Insert) { Copy(str + 5, str, size - strIndex); }

		Copy(str + size, StringLookup<C>::FALSE_STR, 5);
		size += 5;

		if constexpr (!Insert) { string[size] = StringLookup<C>::NULL_CHAR; }
		needHash = true;

		return strIndex + 5;
	}
}

template<Character C>
template<FloatingPoint Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value, U64 decimalCount) noexcept
{
	if constexpr (Hex) { return ToString<U64, Hex, Insert, Remove>(str, reinterpret_cast<const U64&>(value)); }
	else
	{
		const U64 typeSize = RequiredCapacity<Arg, Hex>() + decimalCount;
		const U64 moveSize = typeSize - Remove;
		const U64 strIndex = str - string;
		const U64 excessSize = size - strIndex;

		if (!string || capacity < size + moveSize) { Memory::Reallocate(&string, size + moveSize, capacity); str = string + strIndex; }
		if constexpr (Insert) { Memory::Copy(str + moveSize, str, excessSize * sizeof(C)); }

		C* c = str + typeSize;
		const C* digits;
		Arg val;
		U64 neg;

		if (value < 0)
		{
			str[0] = StringLookup<C>::NEGATIVE_CHAR;
			val = (Arg)-value;
			neg = 1;
		}
		else
		{
			val = (Arg)value;
			neg = 0;
		}

		if (decimalCount > 0)
		{
			U64 dec = (U64)((val - (F64)(U64)val) * 100000.0f);

			while (decimalCount > 2)
			{
				U64 newVal = dec / 1000;
				U64 remainder = dec % 1000;
				digits = StringLookup<C>::DECIMAL_LOOKUP + (remainder * 3);
				*--c = digits[2];
				*--c = digits[1];
				*--c = digits[0];
				dec = newVal;

				decimalCount -= 3;
			}

			digits = StringLookup<C>::DECIMAL_LOOKUP + (dec * 3);
			if (decimalCount > 0) { *--c = digits[2]; }
			if (decimalCount > 1) { *--c = digits[1]; }
			*--c = StringLookup<C>::DECIMAL_CHAR;
		}

		U64 whole = (U64)val;

		while (whole > 999)
		{
			U64 newVal = whole / 1000;
			U64 remainder = whole % 1000;
			digits = StringLookup<C>::DECIMAL_LOOKUP + (remainder * 3);
			*--c = digits[2];
			*--c = digits[1];
			*--c = digits[0];
			whole = newVal;
		}

		digits = StringLookup<C>::DECIMAL_LOOKUP + (whole * 3);
		*--c = digits[2];
		if (whole > 9) { *--c = digits[1]; }
		if (whole > 99) { *--c = digits[0]; }

		I64 addLength = typeSize + neg - (c - str);
		size += addLength - Remove;

		if constexpr (Insert) { Memory::Copy(str + neg, c, (addLength + excessSize - Remove) * sizeof(C)); }
		else { Memory::Copy(str + neg, c, addLength * sizeof(C)); }

		string[size] = StringLookup<C>::NULL_CHAR;
		needHash = true;

		return strIndex + addLength;
	}
}

template<Character C>
template<NonStringPointer Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value) noexcept
{
	return ToString<U64, Hex, Insert, Remove>(str, reinterpret_cast<const U64&>(value));
}

template<Character C>
template<Character Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value) noexcept
{
	using CharType = BaseType<Arg>;
	return ToString<CharType*, Hex, Insert, Remove, 1>(str, (CharType*)&value);
}

template<Character C>
template<StringLiteral Arg, bool Hex, bool Insert, U64 Remove, U64 Size>
inline U64 StringBase<C>::ToString(C* str, const Arg& value) noexcept
{
	using CharType = BaseType<Arg>;

	U64 strSize;
	if constexpr (Size == 0) { strSize = Length(value); }
	else { strSize = Size; }

	bool replace = false;
	U64 moveSize = strSize;
	if constexpr (Remove == U64_MAX) { replace = true; }
	else { moveSize -= Remove; }

	const U64 strIndex = str - string;
	const U64 excessSize = size - strIndex;

	if (!string || capacity < size + moveSize) { Memory::Reallocate(&string, size + moveSize, capacity); str = string + strIndex; }

	if constexpr (Insert) { Memory::Copy(str + moveSize, str, excessSize * sizeof(C)); }

	if constexpr (IsSame<CharType, C>) { Memory::Copy(str, value, strSize * sizeof(C)); }
	else if constexpr (IsSame<CharType, C8>)
	{
		if constexpr (IsSame<C, C16>)
		{
			//TODO
		}
		else //C32
		{
			//TODO
		}
	}
	else if constexpr (IsSame<CharType, C16>)
	{
		if constexpr (IsSame<C, C8>)
		{
			const CharType* it0 = value;
			CharType c;
			C* it1 = str;
			while ((c = *it0++) != '\0')
			{
				if (c <= 0x7F) { *it1++ = (C)c; }
				else { *it1++ = '?'; }
			}
		}
		else //C32
		{
			//TODO
		}
	}
	else if constexpr (IsSame<CharType, C32>)
	{
		if constexpr (IsSame<C, C8>)
		{
			//TODO
		}
		else //C16
		{
			//TODO
		}
	}
	else if constexpr (IsSame<CharType, char8_t>)
	{
		if constexpr (IsSame<C, C8>)
		{
			//TODO
		}
		else //C16
		{
			//TODO
		}
	}
	else if constexpr (IsSame<CharType, CW>)
	{
		if constexpr (IsSame<C, C8>)
		{
			const CharType* it0 = value;
			CharType c;
			C* it1 = str;
			while ((c = *it0++) != '\0')
			{
				if (c <= 0x7F) { *it1++ = (C)c; }
				else if (c <= 0x7FF) { *it1++ = (C)((c >> 6) | 0xC0); *it1++ = (C)((c & 0x3F) | 0x80); }
				else if (c <= 0xFFFF) { *it1++ = (C)((c >> 12) | 0xE0); *it1++ = (C)(((c >> 6) & 0x3F) | 0x80); *it1++ = (C)((c & 0x3F) | 0x80); }
				else { *it1++ = '?'; }
			}
		}
		else //C16
		{
			//TODO
		}
	}

	if (replace) { size = moveSize; }
	else { size += moveSize; }

	string[size] = StringLookup<C>::NULL_CHAR;
	needHash = true;

	return strIndex + strSize;
}

template<Character C>
template<StringType Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value) noexcept
{
	//TODO: Move semantics

	using StrBs = BaseType<Arg>;

	if constexpr (IsSame<StrBs, StringBase<C8>>) { using CharType = C8; }
	else if constexpr (IsSame<StrBs, StringBase<C16>>) { using CharType = C16; }
	else if constexpr (IsSame<StrBs, StringBase<C32>>) { using CharType = C32; }

	return ToString<CharType*, Hex, Insert, Remove>(str, (CharType*)value.Data());
}

template<Character C>
template<NonStringClass Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value) noexcept
{
	if constexpr (ConvertibleTo<Arg, StringBaseType>)
	{
		return ToString<StringBaseType, Hex, Insert, Remove>(str, value.operator StringBaseType());
	}
	else
	{
		return ToString<U64, Hex, Insert, Remove>(str, (U64)&value);
	}
}

template<Character C>
template<typename Arg, bool Hex>
inline constexpr U64 StringBase<C>::RequiredCapacity() noexcept
{
	if constexpr (Hex)
	{
		if constexpr (IsSame<Arg, U8>) { return 2; }
		if constexpr (IsSame<Arg, U16>) { return 4; }
		if constexpr (IsSame<Arg, U32>) { return 8; }
		if constexpr (IsSame<Arg, UL32>) { return 8; }
		if constexpr (IsSame<Arg, U64>) { return 16; }
		if constexpr (IsSame<Arg, I8>) { return 2; }
		if constexpr (IsSame<Arg, I16>) { return 4; }
		if constexpr (IsSame<Arg, I32>) { return 8; }
		if constexpr (IsSame<Arg, L32>) { return 8; }
		if constexpr (IsSame<Arg, I64>) { return 16; }
		if constexpr (IsSame<Arg, F32>) { return 16; }
		if constexpr (IsSame<Arg, F64>) { return 16; }
	}
	else
	{
		if constexpr (IsSame<Arg, U8>) { return 3; }
		if constexpr (IsSame<Arg, U16>) { return 5; }
		if constexpr (IsSame<Arg, U32>) { return 10; }
		if constexpr (IsSame<Arg, UL32>) { return 10; }
		if constexpr (IsSame<Arg, U64>) { return 20; }
		if constexpr (IsSame<Arg, I8>) { return 4; }
		if constexpr (IsSame<Arg, I16>) { return 6; }
		if constexpr (IsSame<Arg, I32>) { return 11; }
		if constexpr (IsSame<Arg, L32>) { return 11; }
		if constexpr (IsSame<Arg, I64>) { return 20; }
		if constexpr (IsSame<Arg, F32>) { return 22; }
		if constexpr (IsSame<Arg, F64>) { return 22; }
	}
}

template<Character C>
template<Signed Arg>
inline Arg StringBase<C>::ToType(U64 start) const noexcept
{
	C* it = string + start;
	C c;
	Arg value = 0;

	if (*it == StringLookup<C>::NEGATIVE_CHAR)
	{
		++it;
		while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR) { value *= 10; value -= c - StringLookup<C>::ZERO_CHAR; }
	}
	else
	{
		while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR) { value *= 10; value += c - StringLookup<C>::ZERO_CHAR; }
	}

	return value;
}

template<Character C>
template<Unsigned Arg>
inline Arg StringBase<C>::ToType(U64 start) const noexcept
{
	C* it = string + start;
	C c;
	Arg value = 0;

	while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR) { value *= 10; value += c - StringLookup<C>::ZERO_CHAR; }

	return value;
}

template<Character C>
template<Boolean Arg>
inline Arg StringBase<C>::ToType(U64 start) const noexcept
{
	return Compare(string + start, StringLookup<C>::TRUE_STR, 4);
}

template<Character C>
template<FloatingPoint Arg>
inline Arg StringBase<C>::ToType(U64 start) const noexcept
{
	//TODO: Handle NaN, +-INF

	C* it = string + start;
	C c;
	Arg value = 0.0f;
	F64 mul = 0.1f;

	if (*it == StringLookup<C>::NEGATIVE_CHAR)
	{
		++it;
		while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR && c != StringLookup<C>::DECIMAL_CHAR) { value *= 10; value -= c - StringLookup<C>::ZERO_CHAR; }
		while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR) { value -= (c - StringLookup<C>::ZERO_CHAR) * mul; mul *= 0.1f; }
	}
	else
	{
		if (*it == StringLookup<C>::POSITIVE_CHAR) { ++it; }

		while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR && c != StringLookup<C>::DECIMAL_CHAR) { value *= 10; value += c - StringLookup<C>::ZERO_CHAR; }
		while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR) { value += (c - StringLookup<C>::ZERO_CHAR) * mul; mul *= 0.1f; }
	}

	return value;
}

template<Character C>
template<NonStringPointer Arg>
inline Arg StringBase<C>::ToType(U64 start) const noexcept
{
	return Move(ToType<U64>(start));
}

template<Character C>
template<Character Arg>
inline Arg StringBase<C>::ToType(U64 start) const noexcept
{
	//TODO: conversions
	return string[start];
}

template<Character C>
template<StringLiteral Arg>
inline Arg StringBase<C>::ToType(U64 start) const noexcept
{
	using CharType = BaseType<Arg>;

	if constexpr (IsSame<CharType, C>) { return string + start; }
	else if constexpr (IsSame<CharType, C8>)
	{
		if constexpr (IsSame<C, C16>) {}
		else if constexpr (IsSame<C, C32>) {}
	}
	else if constexpr (IsSame<CharType, C16>)
	{
		if constexpr (IsSame<C, C8>) {}
		else if constexpr (IsSame<C, C32>) {}
	}
	else if constexpr (IsSame<CharType, C32>)
	{
		if constexpr (IsSame<C, C8>) {}
		else if constexpr (IsSame<C, C16>) {}
	}
	else if constexpr (IsSame<CharType, char8_t>)
	{
		if constexpr (IsSame<C, C8>) { return (C8*)(string + start); }
		else if constexpr (IsSame<C, C16>) {}
		else if constexpr (IsSame<C, C32>) {}
	}
	else if constexpr (IsSame<CharType, CW>)
	{
		if constexpr (IsSame<C, C8>) {}
		else if constexpr (IsSame<C, C16>) { return (CW*)(string + start); }
		else if constexpr (IsSame<C, C32>) {}
	}
}

template<Character C>
template<StringType Arg>
inline Arg StringBase<C>::ToType(U64 start) const noexcept
{
	if constexpr (IsSame<Arg, StringBase<C>>) { return Move(String(string + start)); }
	else if constexpr (IsSame<Arg, StringBase<C8>>)
	{
		if constexpr (IsSame<StringBase<C>, StringBase<C16>>)
		{
			//TODO
		}
		else //C32
		{
			//TODO
		}
	}
	else if constexpr (IsSame<Arg, StringBase<C16>>)
	{
		if constexpr (IsSame<StringBase<C>, StringBase<C8>>)
		{
			//TODO
		}
		else //C32
		{
			//TODO
		}
	}
	else if constexpr (IsSame<Arg, StringBase<C32>>)
	{
		if constexpr (IsSame<StringBase<C>, StringBase<C8>>)
		{
			//TODO
		}
		else //C16
		{
			//TODO
		}
	}
}

template<Character C>
inline bool StringBase<C>::Compare(const C* a, const C* b, I64 length) noexcept
{
	const C* it0 = a;
	const C* it1 = b;

	C c0;
	C c1;

	while (length-- && (c0 = *it0++) == (c1 = *it1++)) {}

	return (length + 1) == 0;
}

template<Character C>
inline bool StringBase<C>::WhiteSpace(C c) noexcept
{
	return c == StringLookup<C>::SPACE || c == StringLookup<C>::HTAB || c == StringLookup<C>::VTAB ||
		c == StringLookup<C>::NEW_LINE || c == StringLookup<C>::RETURN || c == StringLookup<C>::FEED;
}

template<Character C>
inline bool StringBase<C>::NotWhiteSpace(C c) noexcept
{
	return c != StringLookup<C>::SPACE && c != StringLookup<C>::HTAB && c != StringLookup<C>::VTAB &&
		c != StringLookup<C>::NEW_LINE && c != StringLookup<C>::RETURN && c != StringLookup<C>::FEED;
}

template<Character C>
template<typename Arg>
inline void StringBase<C>::FindFormat(U64& start, const Arg& value) noexcept
{
	C* it = string + start;
	C c = *it;

	//TODO: escape characters ``
	while ((c = *it++) != StringLookup<C>::NULL_CHAR)
	{
		if (c == StringLookup<C>::OPEN_BRACE)
		{
			c = *it++;
			switch (c)
			{
			case StringLookup<C>::CLOSE_BRACE: { start = ToString<Arg, false, true, 2>(it - 2, value); return; } break;
			case StringLookup<C>::FMT_HEX: { if (*it == StringLookup<C>::CLOSE_BRACE) { start = ToString<Arg, true, true, 3>(it - 2, value); return; } } break;
			case StringLookup<C>::FMT_DEC: {
				if constexpr (IsFloatingPoint<Arg>)
				{
					if (*it == StringLookup<C>::CLOSE_BRACE) { start = ToString<Arg, false, true, 3>(it - 2, value, 5); return; }
					else if (it[1] == StringLookup<C>::CLOSE_BRACE) { start = ToString<Arg, false, true, 4>(it - 2, value, *it - '0'); return; }
				}
			} break;
			}
		}
	}
}