#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Containers/String.hpp"

class NH_API Hash
{
public:
	/// <summary>
	/// Creates a hash for a string literal at compile-time
	/// </summary>
	/// <param name="str:">The string literal</param>
	/// <param name="length:">The length of the string</param>
	/// <returns>The hash</returns>
	static constexpr U64 String(const C8* str, U64 length)
	{
		U64 hash = 5381;
		U64 i = 0;

		for (i = 0; i < length; ++str, ++i)
		{
			hash = ((hash << 5) + hash) + (*str);
		}

		return hash;
	}

	/// <summary>
	/// Creates a hash for a string literal at compile-time, case insensitive
	/// </summary>
	/// <param name="str:">The string literal</param>
	/// <param name="length:">The length of the string</param>
	/// <returns>The hash</returns>
	static constexpr U64 StringCI(const C8* str, U64 length)
	{
		U64 hash = 5381;
		U64 i = 0;

		for (i = 0; i < length; ++str, ++i)
		{
			C8 c = *str;
			if (c > 64 && c < 91) { c += 32; }

			hash = ((hash << 5) + hash) + c;
		}

		return hash;
	}

	/// <summary>
	/// Creates a hash for data at compile-time
	/// </summary>
	/// <param name="str:">The string literal</param>
	/// <param name="length:">The length of the string</param>
	/// <returns>The hash</returns>
	static constexpr U64 Data(const U8* data, U64 length)
	{
		U64 hash = 5381;
		U64 i = 0;

		for (i = 0; i < length; ++data, ++i)
		{
			hash = ((hash << 5) + hash) + *data;
		}

		return hash;
	}

	/// <summary>
	/// Creates a hash for data at compile-time
	/// </summary>
	/// <param name="str:">The string literal</param>
	/// <param name="length:">The length of the string</param>
	/// <returns>The hash</returns>
	template <class Type>
	static constexpr U64 Any(const Type& t)
	{
		if constexpr (IsStringType<Type> || IsSame<Type, StringView>) { return String(t.Data(), t.Size()); }
		else { return Data((U8*)&t, sizeof(Type)); }
	}

private:

	STATIC_CLASS(Hash);
};

/// <summary>
/// Creates a hash for a string literal at compile-time
/// </summary>
/// <param name="str:">The string literal</param>
/// <param name="length:">The length of the string</param>
/// <returns>The hash</returns>
constexpr inline U64 operator""_Hash(const C8* str, U64 length) { return Hash::String(str, length); }

/// <summary>
/// Creates a hash for a string literal at compile-time, case insensitive
/// </summary>
/// <param name="str:">The string literal</param>
/// <param name="length:">The length of the string</param>
/// <returns>The hash</returns>
constexpr inline U64 operator""_HashCI(const C8* str, U64 length) { return Hash::StringCI(str, length); }