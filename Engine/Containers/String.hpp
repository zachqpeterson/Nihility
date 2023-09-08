#pragma once

#include "ContainerDefines.hpp"

#include "Vector.hpp"
#include "Memory\Memory.hpp"
#include "Math\Hash.hpp"
#include "Math\Random.hpp"

#define UPPER_CHAR		0x01
#define LOWER_CHAR		0x02
#define DIGIT_CHAR		0x04
#define SPACE_CHAR		0x08
#define PUNCT_CHAR		0x10
#define CONTROL_CHAR	0x20
#define HEX_CHAR		0x40

#define ALPHA_CHAR		UPPER_CHAR | LOWER_CHAR
#define ALPHANUM_CHAR	ALPHA_CHAR | DIGIT_CHAR

static inline constexpr C8 TYPE_LOOKUP[]{
	0,							// -1 EOF
	CONTROL_CHAR,				// 00 (NUL)
	CONTROL_CHAR,				// 01 (SOH)
	CONTROL_CHAR,				// 02 (STX)
	CONTROL_CHAR,				// 03 (ETX)
	CONTROL_CHAR,				// 04 (EOT)
	CONTROL_CHAR,				// 05 (ENQ)
	CONTROL_CHAR,				// 06 (ACK)
	CONTROL_CHAR,				// 07 (BEL)
	CONTROL_CHAR,				// 08 (BS)
	SPACE_CHAR | CONTROL_CHAR,	// 09 (HT)
	SPACE_CHAR | CONTROL_CHAR,	// 0A (LF)
	SPACE_CHAR | CONTROL_CHAR,	// 0B (VT)
	SPACE_CHAR | CONTROL_CHAR,	// 0C (FF)
	SPACE_CHAR | CONTROL_CHAR,	// 0D (CR)
	CONTROL_CHAR,				// 0E (SI)
	CONTROL_CHAR,				// 0F (SO)
	CONTROL_CHAR,				// 10 (DLE)
	CONTROL_CHAR,				// 11 (DC1)
	CONTROL_CHAR,				// 12 (DC2)
	CONTROL_CHAR,				// 13 (DC3)
	CONTROL_CHAR,				// 14 (DC4)
	CONTROL_CHAR,				// 15 (NAK)
	CONTROL_CHAR,				// 16 (SYN)
	CONTROL_CHAR,				// 17 (ETB)
	CONTROL_CHAR,				// 18 (CAN)
	CONTROL_CHAR,				// 19 (EM)
	CONTROL_CHAR,				// 1A (SUB)
	CONTROL_CHAR,				// 1B (ESC)
	CONTROL_CHAR,				// 1C (FS)
	CONTROL_CHAR,				// 1D (GS)
	CONTROL_CHAR,				// 1E (RS)
	CONTROL_CHAR,				// 1F (US)
	SPACE_CHAR,					// 20 SPACE
	PUNCT_CHAR,					// 21 !
	PUNCT_CHAR,					// 22 "
	PUNCT_CHAR,					// 23 #
	PUNCT_CHAR,					// 24 $
	PUNCT_CHAR,					// 25 %
	PUNCT_CHAR,					// 26 &
	PUNCT_CHAR,					// 27 '
	PUNCT_CHAR,					// 28 (
	PUNCT_CHAR,					// 29 )
	PUNCT_CHAR,					// 2A *
	PUNCT_CHAR,					// 2B +
	PUNCT_CHAR,					// 2C ,
	PUNCT_CHAR,					// 2D -
	PUNCT_CHAR,					// 2E .
	PUNCT_CHAR,					// 2F /
	DIGIT_CHAR | HEX_CHAR,		// 30 0
	DIGIT_CHAR | HEX_CHAR,		// 31 1
	DIGIT_CHAR | HEX_CHAR,		// 32 2
	DIGIT_CHAR | HEX_CHAR,		// 33 3
	DIGIT_CHAR | HEX_CHAR,		// 34 4
	DIGIT_CHAR | HEX_CHAR,		// 35 5
	DIGIT_CHAR | HEX_CHAR,		// 36 6
	DIGIT_CHAR | HEX_CHAR,		// 37 7
	DIGIT_CHAR | HEX_CHAR,		// 38 8
	DIGIT_CHAR | HEX_CHAR,		// 39 9
	PUNCT_CHAR,					// 3A :
	PUNCT_CHAR,					// 3B ;
	PUNCT_CHAR,					// 3C <
	PUNCT_CHAR,					// 3D =
	PUNCT_CHAR,					// 3E >
	PUNCT_CHAR,					// 3F ?
	PUNCT_CHAR,					// 40 @
	UPPER_CHAR + HEX_CHAR,		// 41 A
	UPPER_CHAR + HEX_CHAR,		// 42 B
	UPPER_CHAR + HEX_CHAR,		// 43 C
	UPPER_CHAR + HEX_CHAR,		// 44 D
	UPPER_CHAR + HEX_CHAR,		// 45 E
	UPPER_CHAR + HEX_CHAR,		// 46 F
	UPPER_CHAR,					// 47 G
	UPPER_CHAR,					// 48 H
	UPPER_CHAR,					// 49 I
	UPPER_CHAR,					// 4A J
	UPPER_CHAR,					// 4B K
	UPPER_CHAR,					// 4C L
	UPPER_CHAR,					// 4D M
	UPPER_CHAR,					// 4E N
	UPPER_CHAR,					// 4F O
	UPPER_CHAR,					// 50 P
	UPPER_CHAR,					// 51 Q
	UPPER_CHAR,					// 52 R
	UPPER_CHAR,					// 53 S
	UPPER_CHAR,					// 54 T
	UPPER_CHAR,					// 55 U
	UPPER_CHAR,					// 56 V
	UPPER_CHAR,					// 57 W
	UPPER_CHAR,					// 58 X
	UPPER_CHAR,					// 59 Y
	UPPER_CHAR,					// 5A Z
	PUNCT_CHAR,					// 5B [
	PUNCT_CHAR,					// 5C \ 
	PUNCT_CHAR,					// 5D ]
	PUNCT_CHAR,					// 5E ^
	PUNCT_CHAR,					// 5F _
	PUNCT_CHAR,					// 60 `
	LOWER_CHAR + HEX_CHAR,		// 61 a
	LOWER_CHAR + HEX_CHAR,		// 62 b
	LOWER_CHAR + HEX_CHAR,		// 63 c
	LOWER_CHAR + HEX_CHAR,		// 64 d
	LOWER_CHAR + HEX_CHAR,		// 65 e
	LOWER_CHAR + HEX_CHAR,		// 66 f
	LOWER_CHAR,					// 67 g
	LOWER_CHAR,					// 68 h
	LOWER_CHAR,					// 69 i
	LOWER_CHAR,					// 6A j
	LOWER_CHAR,					// 6B k
	LOWER_CHAR,					// 6C l
	LOWER_CHAR,					// 6D m
	LOWER_CHAR,					// 6E n
	LOWER_CHAR,					// 6F o
	LOWER_CHAR,					// 70 p
	LOWER_CHAR,					// 71 q
	LOWER_CHAR,					// 72 r
	LOWER_CHAR,					// 73 s
	LOWER_CHAR,					// 74 t
	LOWER_CHAR,					// 75 u
	LOWER_CHAR,					// 76 v
	LOWER_CHAR,					// 77 w
	LOWER_CHAR,					// 78 x
	LOWER_CHAR,					// 79 y
	LOWER_CHAR,					// 7A z
	PUNCT_CHAR,					// 7B {
	PUNCT_CHAR,					// 7C |
	PUNCT_CHAR,					// 7D }
	PUNCT_CHAR,					// 7E ~
	CONTROL_CHAR,				// 7F (DEL)
};

static struct Hex {} HEX;

template<class C> struct StringBase;

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
constexpr inline U64 Length(const C* str)
{
	if (!str) { return 0; }

	const C* it = str;
	while (*it) { ++it; }

	return it - str;
}

constexpr inline U64 Length(NullPointer)
{
	return 0;
}

template<Character C>
static inline bool Compare(const C* a, const C* b, I64 length)
{
	const C* it0 = a;
	const C* it1 = b;
	C c0;
	C c1;

	while (length-- && (c0 = *it0++) == (c1 = *it1++));

	return (length + 1) == 0;
}

template<Character C>
static inline bool Compare(const C* a, const C* b)
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
template<class C>
struct NH_API StringBase
{
	using CharType = C;
	using StringBaseType = StringBase<C>;

	static StringBase RandomString(U32 length);

	StringBase();
	StringBase(NoInit);
	StringBase(const StringBase& other);
	StringBase(StringBase&& other) noexcept;
	template<typename Arg> StringBase(const Arg& value, Hex) noexcept;
	template<typename First, typename... Args> StringBase(const First& first, const Args& ... args) noexcept;
	template<typename First, typename... Args> StringBase(const C* fmt, const First& first, const Args& ... args) noexcept; //Take in any string literal type

	StringBase& operator=(const StringBase& other);
	StringBase& operator=(StringBase&& other) noexcept;
	template<typename Arg> StringBase& operator=(const Arg& value) noexcept;
	template<typename Arg> StringBase& operator+=(const Arg& value) noexcept;

	template<typename Arg> StringBase operator+(const Arg& value) const noexcept;

	~StringBase();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);
	void Resize();

	template<Signed Arg> Arg ToType(U64 start = 0) const;
	template<Unsigned Arg> Arg ToType(U64 start = 0) const;
	template<Boolean Arg> Arg ToType(U64 start = 0) const;
	template<FloatingPoint Arg> Arg ToType(U64 start = 0) const;
	template<NonStringPointer Arg> Arg ToType(U64 start = 0) const;
	template<Character Arg> Arg ToType(U64 start = 0) const;
	template<StringLiteral Arg> Arg ToType(U64 start = 0) const;
	template<StringType Arg> Arg ToType(U64 start = 0) const;

	C* operator*();
	const C* operator*() const;
	C& operator[](U64 i);
	const C& operator[](U64 i) const;

	bool operator==(C* other) const;
	bool operator==(const StringBase& other) const;
	template<U64 Count> bool operator==(const C(&other)[Count]) const;
	bool operator!=(C* other) const;
	bool operator!=(const StringBase& other) const;
	template<U64 Count> bool operator!=(const C(&other)[Count]) const;

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

	bool Blank() const;
	I64 IndexOf(const C& c, U64 start = 0) const;
	I64 LastIndexOf(const C& c, U64 start = 0) const;

	StringBase& Trim();
	template<typename Arg> StringBase& Append(const Arg& append);
	template<typename Arg> StringBase& Prepend(const Arg& prepend);
	template<typename PreArg, typename PostArg> StringBase& Surround(const PreArg& prepend, const PostArg& append);
	template<typename Arg> StringBase& Insert(const Arg& value, U64 i);
	template<typename Arg> StringBase& Overwrite(const Arg& value, U64 i = 0);
	template<typename Arg> StringBase& ReplaceAll(const C* find, const Arg& replace, U64 start = 0);
	template<typename Arg> StringBase& ReplaceN(const C* find, const Arg& replace, U64 count, U64 start = 0);
	template<typename Arg> StringBase& Replace(const C* find, const Arg& replace, U64 start = 0);

	void SubString(StringBase& newStr, U64 start, U64 nLength = U64_MAX) const;
	template<typename Arg> void Appended(StringBase& newStr, const Arg& append) const;
	template<typename Arg> void Prepended(StringBase& newStr, const Arg& prepend) const;
	template<typename PreArg, typename PostArg> void Surrounded(StringBase& newStr, const PreArg& prepend, const PostArg& append) const;
	void Split(Vector<StringBase>& list, C delimiter, bool trimEntries) const;

	StringBase& ToUpper();
	StringBase& ToLower();

	const U64& Size() const;
	const U64& Capacity() const;
	const U64& Hash() const;
	C* Data();
	const C* Data() const;
	operator C* ();
	operator const C* () const;

	C* begin();
	C* end();
	const C* begin() const;
	const C* end() const;

	C* rbegin();
	C* rend();
	const C* rbegin() const;
	const C* rend() const;

private:
	template<Signed Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value);
	template<Unsigned Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value);
	template<Boolean Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value);
	template<FloatingPoint Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value, U64 decimalCount = 5);
	template<NonStringPointer Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value);
	template<Character Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value);
	template<StringLiteral Arg, bool Hex, bool Insert, U64 Remove = 0, U64 Size = 0> U64 ToString(C* str, const Arg& value);
	template<StringType Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value);
	template<NonStringClass Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(C* str, const Arg& value);

	template<typename Arg, bool Hex> static constexpr U64 RequiredCapacity();

	template<typename Arg> void Format(U64& start, const Arg& value);

	static bool Compare(const C* a, const C* b, I64 length);
	static bool WhiteSpace(C c);
	static bool NotWhiteSpace(C c);

	U64 hash{ 0 };
	U64 size{ 0 };
	U64 capacity{ 0 };
	C* string{ nullptr };
};

template<class C>
inline StringBase<C> StringBase<C>::RandomString(U32 length)
{
	String str{ length };

	C* it = str.string;

	for (U32 i = 0; i < length; ++i)
	{
		*it++ = StringLookup<C>::ALPHANUM_LOOKUP[Random::RandomRange(0, Length(StringLookup<C>::ALPHANUM_LOOKUP))];
	}

	return str;
}

template<class C>
inline StringBase<C>::StringBase() { Memory::AllocateArray(&string, capacity, capacity); }

template<class C>
inline StringBase<C>::StringBase(NoInit flag) {}

template<class C>
inline StringBase<C>::StringBase(const StringBase& other) : hash{ other.hash }, size{ other.size }
{
	if (!string || capacity < other.size) { Memory::Reallocate(&string, size, capacity); }

	Memory::Copy(string, other.string, size * sizeof(C));
	string[size] = StringLookup<C>::NULL_CHAR;
}

template<class C>
inline StringBase<C>::StringBase(StringBase&& other) noexcept : hash{ other.hash }, size{ other.size }, capacity{ other.capacity }, string{ other.string }
{
	other.hash = 0;
	other.size = 0;
	other.capacity = 0;
	other.string = nullptr;
}

template<class C>
template<typename Arg>
inline StringBase<C>::StringBase(const Arg& value, Hex flag) noexcept { ToString<Arg, true, false>(string, value); hash = Hash::Calculate(string, size); }

template<class C>
template<typename First, typename... Args>
inline StringBase<C>::StringBase(const First& first, const Args& ... args) noexcept
{
	Memory::AllocateArray(&string, capacity, capacity);
	ToString<First, false, false>(string, first);
	(ToString<Args, false, false>(string + size, args), ...);
	hash = Hash::Calculate(string, size);
}

template<class C>
template<typename First, typename... Args>
inline StringBase<C>::StringBase(const C* fmt, const First& first, const Args& ... args) noexcept : size{ Length(fmt) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity, capacity);

	Memory::Copy(string, fmt, (size + 1) * sizeof(C));
	U64 start = 0;
	Format(start, first);
	(Format(start, args), ...);
	hash = Hash::Calculate(string, size);
}

template<class C>
inline StringBase<C>& StringBase<C>::operator=(const StringBase& other)
{
	hash = other.hash;
	size = other.size;

	if (!string || capacity < other.size) { Memory::Reallocate(&string, size, capacity); }

	Memory::Copy(string, other.string, size * sizeof(C));
	string[size] = StringLookup<C>::NULL_CHAR;

	return *this;
}

template<class C>
inline StringBase<C>& StringBase<C>::operator=(StringBase&& other) noexcept
{
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

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::operator=(const Arg& value) noexcept
{
	ToString<Arg, false, true, U64_MAX>(string, value);
	hash = Hash::Calculate(string, size);
	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::operator+=(const Arg& value) noexcept
{
	ToString<Arg, false, false>(string + size, value);
	hash = Hash::Calculate(string, size);
	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C> StringBase<C>::operator+(const Arg& value) const noexcept
{
	StringBase<C> copy = *this;
	copy += value;

	return Move(copy);
}

template<class C>
inline StringBase<C>::~StringBase()
{
	hash = 0;
	if (string)
	{
		size = 0;
		capacity = 0;
		Memory::FreeArray(&string);
	}
}

template<class C>
inline void StringBase<C>::Destroy()
{
	hash = 0;
	if (string)
	{
		size = 0;
		capacity = 0;
		Memory::FreeArray(&string);
	}
}

template<class C>
inline void StringBase<C>::Clear()
{
	if (string)
	{
		string[0] = StringLookup<C>::NULL_CHAR;
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
	string[size] = StringLookup<C>::NULL_CHAR;
}

template<class C>
inline void StringBase<C>::Resize()
{
	size = Length(string);

	hash = Hash::Calculate(string, size);
}

template<class C>
inline C* StringBase<C>::operator*() { return string; }

template<class C>
inline const C* StringBase<C>::operator*() const { return string; }

template<class C>
inline C& StringBase<C>::operator[](U64 i) { return string[i]; }

template<class C>
inline const C& StringBase<C>::operator[](U64 i) const { return string[i]; }

template<class C>
inline bool StringBase<C>::operator==(C* other) const
{
	U64 len = Length(other);
	if (len != size) { return false; }

	return Compare(string, other, size);
}

template<class C>
inline bool StringBase<C>::operator==(const StringBase<C>& other) const
{
	if (other.size != size) { return false; }

	return Compare(string, other.string, size);
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::operator==(const C(&other)[Count]) const
{
	if (Count - 1 != size) { return false; }

	return Compare(string, other, Count - 1);
}

template<class C>
inline bool StringBase<C>::operator!=(C* other) const
{
	U64 len = Length(other);
	if (len != size) { return true; }

	return !Compare(string, other, size);
}

template<class C>
inline bool StringBase<C>::operator!=(const StringBase<C>& other) const
{
	if (other.size != size) { return true; }

	return !Compare(string, other.string, size);
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::operator!=(const C(&other)[Count]) const
{
	if (Count - 1 != size) { return true; }

	return !Compare(string, other, Count - 1);
}

template<class C>
inline bool StringBase<C>::Compare(C* other) const
{
	U64 len = Length(other);
	if (len != size) { return false; }

	return Compare(string, other, size);
}

template<class C>
inline bool StringBase<C>::Compare(const StringBase<C>& other) const
{
	if (other.size != size) { return false; }

	return Compare(string, other.string, size);
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::Compare(const C(&other)[Count]) const
{
	if (Count - 1 != size) { return false; }

	return Compare(string, other, Count - 1);
}

template<class C>
inline bool StringBase<C>::CompareN(C* other, U64 start) const
{
	U64 len = Length(other);

	return Compare(string + start, other, len);
}

template<class C>
inline bool StringBase<C>::CompareN(const StringBase<C>& other, U64 start) const
{
	return Compare(string + start, other.string);
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::CompareN(const C(&other)[Count], U64 start) const
{
	return Compare(string + start, other, Count - 1);
}

template<class C>
inline bool StringBase<C>::StartsWith(C* other) const
{
	U64 otherSize = Length(other);

	return Compare(string, other, otherSize);
}

template<class C>
inline bool StringBase<C>::StartsWith(const StringBase& other) const
{
	return Compare(string, other.string, other.size);
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::StartsWith(const C(&other)[Count]) const
{
	return Compare(string, other, Count - 1);
}

template<class C>
inline bool StringBase<C>::EndsWith(C* other) const
{
	U64 otherSize = Length(other);

	return Compare(string + (size - otherSize), other, otherSize);
}

template<class C>
inline bool StringBase<C>::EndsWith(const StringBase& other) const
{
	return Compare(string + (size - other.size), other.string, other.size);
}

template<class C>
template<U64 Count>
inline bool StringBase<C>::EndsWith(const C(&other)[Count]) const
{
	return Compare(string + (size - Count - 1), other, Count - 1);
}

template<class C>
inline const U64& StringBase<C>::Size() const { return size; }

template<class C>
inline const U64& StringBase<C>::Capacity() const { return capacity; }

template<class C>
inline const U64& StringBase<C>::Hash() const { return hash; }

template<class C>
inline C* StringBase<C>::Data() { return string; }

template<class C>
inline const C* StringBase<C>::Data() const { return string; }

template<class C>
inline StringBase<C>::operator C* () { return string; }

template<class C>
inline StringBase<C>::operator const C* () const { return string; }

template<class C>
inline bool StringBase<C>::Blank() const
{
	if (size == 0) { return true; }
	C* it = string;
	C c;

	while (WhiteSpace(c = *it++));

	return c == StringLookup<C>::NULL_CHAR;
}

template<class C>
inline I64 StringBase<C>::IndexOf(const C& ch, U64 start) const
{
	C* it = string + start;
	C c;

	while ((c = *it) != ch && c != StringLookup<C>::NULL_CHAR) { ++it; }

	if (*it == StringLookup<C>::NULL_CHAR) { return -1; }
	return (I64)(it - string);
}

template<class C>
inline I64 StringBase<C>::LastIndexOf(const C& c, U64 start) const
{
	C* it = string + size - start - 1;

	U64 len = size;
	while (*it != c && len > 0) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
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
	Memory::Copy(string, start, size);
	string[size] = StringLookup<C>::NULL_CHAR;

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Append(const Arg& append)
{
	ToString<Arg, false, false>(string + size, append);

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Prepend(const Arg& prepend)
{
	ToString<Arg, false, true>(string, prepend);

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
template<typename PreArg, typename PostArg>
inline StringBase<C>& StringBase<C>::Surround(const PreArg& prepend, const PostArg& append)
{
	ToString<PreArg, false, true>(string, prepend);
	ToString<PostArg, false, false>(string + size, append);

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Insert(const Arg& value, U64 i)
{
	ToString<Arg, false, true>(string + i, value);

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Overwrite(const Arg& value, U64 i)
{
	ToString<Arg, false, false>(string + i, value);

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::ReplaceAll(const C* find, const Arg& replace, U64 start)
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

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::ReplaceN(const C* find, const Arg& replace, U64 count, U64 start)
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

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
template<typename Arg>
inline StringBase<C>& StringBase<C>::Replace(const C* find, const Arg& replace, U64 start)
{
	U64 findSize = Length(find);
	C* it = string + start;
	C c;

	while ((c = *it) != StringLookup<C>::NULL_CHAR && Compare(it, find, findSize)) { ++c; }

	if (c != StringLookup<C>::NULL_CHAR) { ToString<Arg, false, true>(c, replace); }

	string[size] = StringLookup<C>::NULL_CHAR;

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
inline void StringBase<C>::SubString(StringBase<C>& newStr, U64 start, U64 nLength) const
{
	if (nLength < U64_MAX) { newStr.Resize(nLength); }
	else { newStr.Resize(size - start); }

	Memory::Copy(newStr.string, string + start, newStr.size);
	newStr.string[newStr.size] = StringLookup<C>::NULL_CHAR;
}

template<class C>
template<typename Arg>
inline void StringBase<C>::Appended(StringBase& newStr, const Arg& append) const
{
	newStr = *this;
	newStr.Append(append);
}

template<class C>
template<typename Arg>
inline void StringBase<C>::Prepended(StringBase& newStr, const Arg& prepend) const
{
	newStr = *this; //TODO: This doesn't make a copy
	newStr.Prepend(prepend);
}

template<class C>
template<typename PreArg, typename PostArg>
inline void StringBase<C>::Surrounded(StringBase& newStr, const PreArg& prepend, const PostArg& append) const
{
	newStr = *this;
	newStr.Surround(prepend, append);
}

template<class C>
inline void StringBase<C>::Split(Vector<StringBase<C>>& list, C delimiter, bool trimEntries) const
{
	//TODO
}

template<class C>
inline StringBase<C>& StringBase<C>::ToUpper()
{
	static_assert(IsSame<C, char>, "ToUpper is only supported for char strings currently");

	for (char& c : *this)
	{
		if (StringLookup<C>::TYPE_LOOKUP[c] & LOWER_CHAR) { c -= 32; }
	}

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
inline StringBase<C>& StringBase<C>::ToLower()
{
	static_assert(IsSame<C, char>, "ToLower is only supported for char strings currently");

	for (char& c : *this)
	{
		if (StringLookup<C>::TYPE_LOOKUP[c] & UPPER_CHAR) { c += 32; }
	}

	hash = Hash::Calculate(string, size);

	return *this;
}

template<class C>
inline C* StringBase<C>::begin() { return string; }

template<class C>
inline C* StringBase<C>::end() { return string + size; }

template<class C>
inline const C* StringBase<C>::begin() const { return string; }

template<class C>
inline const C* StringBase<C>::end() const { return string + size; }

template<class C>
inline C* StringBase<C>::rbegin() { return string + size - 1; }

template<class C>
inline C* StringBase<C>::rend() { return string - 1; }

template<class C>
inline const C* StringBase<C>::rbegin() const { return string + size - 1; }

template<class C>
inline const C* StringBase<C>::rend() const { return string - 1; }

template<class C>
template<Signed Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value)
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

	return strIndex + addLength;
}

template<class C>
template<Unsigned Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value)
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

	return strIndex + addLength;
}

template<class C>
template<Boolean Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value)
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

		return strIndex + 4;
	}
	else
	{
		if (!string || capacity < size + falseSize) { Memory::Reallocate(&string, size + falseSize, capacity); str = string + strIndex; }

		if constexpr (Insert) { Copy(str + 5, str, size - strIndex); }

		Copy(str + size, StringLookup<C>::FALSE_STR, 5);
		size += 5;

		if constexpr (!Insert) { string[size] = StringLookup<C>::NULL_CHAR; }

		return strIndex + 5;
	}
}

template<class C>
template<FloatingPoint Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value, U64 decimalCount)
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
		else { Memory::Copy(str, c, addLength * sizeof(C)); }

		string[size] = StringLookup<C>::NULL_CHAR;

		return strIndex + addLength;
	}
}

template<class C>
template<NonStringPointer Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value)
{
	return ToString<U64, Hex, Insert, Remove>(str, reinterpret_cast<const U64&>(value));
}

template<class C>
template<Character Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value)
{
	using CharType = BaseType<Arg>;
	return ToString<CharType*, Hex, Insert, Remove, 1>(str, (CharType*)&value);
}

template<class C>
template<StringLiteral Arg, bool Hex, bool Insert, U64 Remove, U64 Size>
inline U64 StringBase<C>::ToString(C* str, const Arg& value)
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

	return strIndex + strSize;
}

template<class C>
template<StringType Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value)
{
	//TODO: Move semantics

	using StrBs = BaseType<Arg>;

	if constexpr (IsSame<StrBs, StringBase<C8>>) { using CharType = C8; }
	else if constexpr (IsSame<StrBs, StringBase<C16>>) { using CharType = C16; }
	else if constexpr (IsSame<StrBs, StringBase<C32>>) { using CharType = C32; }

	return ToString<CharType*, Hex, Insert, Remove>(str, (CharType*)value.Data());
}

template<class C>
template<NonStringClass Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<C>::ToString(C* str, const Arg& value)
{
	static_assert(ConvertibleTo<Arg, StringBaseType>, "Arg passed into a string formatter must be convertable to a String type!");

	return ToString<StringBaseType, Hex, Insert, Remove>(str, value.operator StringBaseType());
}

template<class C>
template<typename Arg, bool Hex>
inline constexpr U64 StringBase<C>::RequiredCapacity()
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

template<class C>
template<Signed Arg>
inline Arg StringBase<C>::ToType(U64 start) const
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

	return Move(value);
}

template<class C>
template<Unsigned Arg>
inline Arg StringBase<C>::ToType(U64 start) const
{
	C* it = string + start;
	C c;
	Arg value = 0;

	while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR) { value *= 10; value += c - StringLookup<C>::ZERO_CHAR; }

	return Move(value);
}

template<class C>
template<Boolean Arg>
inline Arg StringBase<C>::ToType(U64 start) const
{
	return Compare(string + start, StringLookup<C>::TRUE_STR, 4);
}

template<class C>
template<FloatingPoint Arg>
inline Arg StringBase<C>::ToType(U64 start) const
{
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
		while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR && c != StringLookup<C>::DECIMAL_CHAR) { value *= 10; value += c - StringLookup<C>::ZERO_CHAR; }
		while (NotWhiteSpace(c = *it++) && c != StringLookup<C>::NULL_CHAR) { value += (c - StringLookup<C>::ZERO_CHAR) * mul; mul *= 0.1f; }
	}

	return Move(value);
}

template<class C>
template<NonStringPointer Arg>
inline Arg StringBase<C>::ToType(U64 start) const
{
	U64 value = ToType(start);
	return Move(value);
}

template<class C>
template<Character Arg>
inline Arg StringBase<C>::ToType(U64 start) const
{
	//TODO: conversions
	return string[start];
}

template<class C>
template<StringLiteral Arg>
inline Arg StringBase<C>::ToType(U64 start) const
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

template<class C>
template<StringType Arg>
inline Arg StringBase<C>::ToType(U64 start) const
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

template<class C>
inline bool StringBase<C>::Compare(const C* a, const C* b, I64 length)
{
	const C* it0 = a;
	const C* it1 = b;

	C c0;
	C c1;

	while (length-- && (c0 = *it0++) == (c1 = *it1++)) {}

	return (length + 1) == 0;
}

template<class C>
inline bool StringBase<C>::WhiteSpace(C c)
{
	return c == StringLookup<C>::SPACE || c == StringLookup<C>::HTAB || c == StringLookup<C>::VTAB || 
		c == StringLookup<C>::NEW_LINE || c == StringLookup<C>::RETURN || c == StringLookup<C>::FEED;
}

template<class C>
inline bool StringBase<C>::NotWhiteSpace(C c)
{
	return c != StringLookup<C>::SPACE && c != StringLookup<C>::HTAB && c != StringLookup<C>::VTAB && 
		c != StringLookup<C>::NEW_LINE && c != StringLookup<C>::RETURN && c != StringLookup<C>::FEED;
}

template<class C>
template<typename Arg>
inline void StringBase<C>::Format(U64& start, const Arg& value)
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