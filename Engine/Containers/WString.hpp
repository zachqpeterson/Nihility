#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"
#include "Vector.hpp"

#define IS_TRUE_W(c) c[0] == L't' && c[1] == L'r' && c[2] == L'u' && c[3] == L'e'
#define WHITE_SPACE_W(c, ptr) (c = *ptr) == L' ' || c == L'\t' || c == L'\r' || c == L'\n' || c == L'\v' || c == L'\f'
#define NOT_WHITE_SPACE_W(c, ptr) (c = *ptr) != L' ' && c != L'\t' && c != L'\r' && c != L'\n' && c != L'\v' && c != L'\f'

struct String;

/*
* TODO: Documentation
*
* TODO: Predicates / regex?
*
* TODO: Count of a character
*
* TODO: Make sure str isn't nullptr and capacity is large enough
*
* TODO: Conversions from char to C16
*
* TODO: Conversions from String to WString
*/
struct NH_API WString
{
public:
	WString();
	WString(NoInit flag);
	template<typename T> WString(T value);
	template<typename T> WString(T value, Hex flag);
	WString(C16* str);
	WString(const C16* str);
	WString(const WString& other);
	WString(WString&& other) noexcept;
	template<typename... Types> WString(const C16* fmt, const Types& ... args);
	template<typename... Types> WString(const Types& ... args);

	template<typename T> WString& operator=(T value);
	WString& operator=(C16* str);
	WString& operator=(const C16* str);
	WString& operator=(const WString& other);
	WString& operator=(WString&& other) noexcept;

	~WString();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);
	void Resize();

	template<typename T> EnableForSignedInt<T, T> ToType(U64 start = 0) const;
	template<typename T> EnableForUnsignedInt<T, T> ToType(U64 start = 0) const;
	template<typename T> EnableForBool<T, T> ToType(U64 start = 0) const;
	template<typename T> EnableForFloat<T, T> ToType(U64 start = 0) const;
	template<typename T> EnableForPointer<T, T> ToType(U64 start = 0) const;

	template<typename T> WString& operator+=(T value);
	WString& operator+=(C16* other);
	WString& operator+=(const C16* other);
	WString& operator+=(const WString& other);

	template<typename T> explicit operator T() const;
	explicit operator C16* ();
	explicit operator const C16* () const;

	C16* operator*();
	const C16* operator*() const;
	C16& operator[](U32 i);
	const C16& operator[](U32 i) const;

	bool operator==(C16* other) const;
	bool operator==(const C16* other) const;
	bool operator==(const WString& other) const;
	bool operator!=(C16* other) const;
	bool operator!=(const C16* other) const;
	bool operator!=(const WString& other) const;

	bool Compare(C16* other) const;
	bool Compare(const C16* other) const;
	bool Compare(const WString& other) const;
	bool CompareN(C16* other, U32 nLength, U32 start = 0) const;
	bool CompareN(const C16* other, U32 nLength, U32 start = 0) const;
	bool CompareN(const WString& other, U32 nLength, U32 start = 0) const;

	const U64& Size() const;
	const U64& Capacity() const;
	U64 Hash();
	C16* Data();
	const C16* Data() const;
	bool Blank() const;
	I32 IndexOf(const C16& c, U64 start = 0) const;
	I32 LastIndexOf(const C16& c, U64 start = 0) const;
	WString& Trim();
	WString& SubString(WString& newStr, U64 start, U64 nLength = I64_MAX) const;
	WString& Append(const WString& append);
	WString& Prepend(const WString& prepend);
	WString& Surround(const WString& prepend, const WString& append);
	WString& Insert(const WString& other, U32 i);
	WString& Overwrite(const WString& other, U32 i = 0);
	WString& ReplaceAll(const WString& find, const WString& replace, U64 start = 0);
	WString& ReplaceN(const WString& find, const WString& replace, U64 count, U64 start = 0);
	WString& ReplaceFirst(const WString& find, const WString& replace, U64 start = 0);
	void Split(Vector<WString>& list, U8 delimiter, bool trimEntries) const;

	C16* begin();
	C16* end();
	const C16* begin() const;
	const C16* end() const;

	C16* rbegin();
	C16* rend();
	const C16* rbegin() const;
	const C16* rend() const;

	static inline constexpr const C16 NULL_CHAR = L'\0';
	static inline constexpr const C16 NEGATIVE_CHAR = L'-';
	static inline constexpr const C16 DECIMAL_CHAR = L'.';
	static inline constexpr const C16 ZERO_CHAR = L'0';
	static inline constexpr const C16* TRUE_STR = L"true";
	static inline constexpr const C16* FALSE_STR = L"false";

private:
	template<typename T> EnableForSignedInt<T> ToString(C16* str, T value);
	template<typename T> EnableForUnsignedInt<T> ToString(C16* str, T value);
	template<typename T> EnableForBool<T> ToString(C16* str, T value);
	template<typename T> EnableForFloat<T> ToString(C16* str, T value);
	template<typename T> EnableForPointer<T> ToString(C16* str, T value);

	template<typename T> EnableForSignedInt<T> HexToString(C16* str, T value);
	template<typename T> EnableForUnsignedInt<T> HexToString(C16* str, T value);
	template<typename T> EnableForFloat<T> HexToString(C16* str, T value);
	template<typename T> EnableForPointer<T> HexToString(C16* str, T value);

	void Format(U64& start, const WString& replace);

	bool hashed{ false };
	U64 hash{ 0 };
	U64 size{ 0 };
	U64 capacity{ 0 };
	C16* string{ nullptr };

#pragma region LOOKUPS
	static inline constexpr const C16 DECIMAL_LOOKUP[] =
		L"000001002003004005006007008009010011012013014015016017018019"
		L"020021022023024025026027028029030031032033034035036037038039"
		L"040041042043044045046047048049050051052053054055056057058059"
		L"060061062063064065066067068069070071072073074075076077078079"
		L"080081082083084085086087088089090091092093094095096097098099"
		L"100101102103104105106107108109110111112113114115116117118119"
		L"120121122123124125126127128129130131132133134135136137138139"
		L"140141142143144145146147148149150151152153154155156157158159"
		L"160161162163164165166167168169170171172173174175176177178179"
		L"180181182183184185186187188189190191192193194195196197198199"
		L"200201202203204205206207208209210211212213214215216217218219"
		L"220221222223224225226227228229230231232233234235236237238239"
		L"240241242243244245246247248249250251252253254255256257258259"
		L"260261262263264265266267268269270271272273274275276277278279"
		L"280281282283284285286287288289290291292293294295296297298299"
		L"300301302303304305306307038309310311312313314315316317318319"
		L"320321322323324325326327238329330331332333334335336337338339"
		L"340341342343344345346347438349350351352353354355356357358359"
		L"360361362363364365366367638369370371372373374375376377378379"
		L"380381382383384385386387838389390391392393394395396397398399"
		L"400401402403404405406407408409410411412413414415416417418419"
		L"420421422423424425426427428429430431432433434435436437438439"
		L"440441442443444445446447448449450451452453454455456457458459"
		L"460461462463464465466467468469470471472473474475476477478479"
		L"480481482483484485486487488489490491492493494495496497498499"
		L"500501502503504505506507508509510511512513514515516517518519"
		L"520521522523524525526527528529530531532533534535536537538539"
		L"540541542543544545546547548549550551552553554555556557558559"
		L"560561562563564565566567568569570571572573574575576577578579"
		L"580581582583584585586587588589590591592593594595596597598599"
		L"600601602603604605606607608609610611612613614615616617618619"
		L"620621622623624625626627628629630631632633634635636637638639"
		L"640641642643644645646647648649650651652653654655656657658659"
		L"660661662663664665666667668669670671672673674675676677678679"
		L"680681682683684685686687688689690691692693694695696697698699"
		L"707701702703704705706707708709710711712713714715716717718719"
		L"727721722723724725726727728729730731732733734735736737738739"
		L"747741742743744745746747748749750751752753754755756757758759"
		L"767761762763764765766767768769770771772773774775776777778779"
		L"787781782783784785786787788789790791792793794795796797798799"
		L"800801802803804805806807808809810811812813814815816817818819"
		L"820821822823824825826827828829830831832833834835836837838839"
		L"840841842843844845846847848849850851852853854855856857858859"
		L"860861862863864865866867868869870871872873874875876877878879"
		L"880881882883884885886887888889890891892893894895896897898899"
		L"900901902903904905906907908909910911912913914915916917918919"
		L"920921922923924925926927928929930931932933934935936937938939"
		L"940941942943944945946947948949950951952953954955956957958959"
		L"960961962963964965966967968969970971972973974975976977978979"
		L"980981982983984985986987988989990991992993994995996997998999";

	static inline constexpr const C16 HEX_LOOKUP[] =
		L"000102030405060708090A0B0C0D0E0F"
		L"101112131415161718191A1B1C1D1E1F"
		L"202122232425262728292A2B2C2D2E2F"
		L"303132333435363738393A3B3C3D3E3F"
		L"404142434445464748494A4B4C4D4E4F"
		L"505152535455565758595A5B5C5D5E5F"
		L"606162636465666768696A6B6C6D6E6F"
		L"707172737475767778797A7B7C7D7E7F"
		L"808182838485868788898A8B8C8D8E8F"
		L"909192939495969798999A9B9C9D9E9F"
		L"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
		L"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
		L"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
		L"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
		L"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
		L"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
#pragma endregion
};

inline WString::WString() { Memory::AllocateArray(&string, capacity); }

inline WString::WString(NoInit flag) {}

template<typename T> inline WString::WString(T value) { ToWString(string, value); }

template<typename T> inline WString::WString(T value, Hex flag) { HexToWString(string, value); }

inline WString::WString(C16* str) : size{ wcslen(str) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);
	memcpy(string, str, (size + 1) * sizeof(C16));
}

inline WString::WString(const C16* str) : size{ wcslen(str) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);
	memcpy(string, str, (size + 1) * sizeof(C16));
}

inline WString::WString(const WString& other) : size{ other.size }, capacity{ other.capacity }
{
	Memory::AllocateArray(&string, capacity);
	memcpy(string, other.string, (size + 1) * sizeof(C16));
}

inline WString::WString(WString&& other) noexcept : size{ other.size }, capacity{ other.capacity }, string{ other.string }
{
	other.size = 0;
	other.capacity = 0;
	other.string = nullptr;
}

template<typename... Types> inline WString::WString(const C16* fmt, const Types& ... args) : size{ wcslen(fmt) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);

	memcpy(string, fmt, (size + 1) * sizeof(C16));
	U64 start = 0;
	(Format(start, args), ...);
}

template<typename... Types> inline WString::WString(const Types& ... args)
{
	Memory::AllocateArray(&string, capacity);
	(Append(args), ...);
}

template<typename T> inline WString& WString::operator=(T value)
{
	ToWString(string, value);
	return *this;
}

inline WString& WString::operator=(C16* str)
{
	hashed = false;
	size = wcslen(str);

	if (capacity < size || !string) { Memory::Reallocate(&string, capacity = size); }

	memcpy(string, str, size + 1);

	return *this;
}

inline WString& WString::operator=(const C16* str)
{
	hashed = false;
	size = wcslen(str);

	if (capacity < size || !string) { Memory::Reallocate(&string, capacity = size); }

	memcpy(string, str, size + 1);

	return *this;
}

inline WString& WString::operator=(const WString& other)
{
	hashed = false;
	if (!other.string) { Destroy(); return *this; }

	size = other.size;

	if (capacity < other.size) { Memory::Reallocate(&string, capacity = other.size); }

	memcpy(string, other.string, size + 1);

	return *this;
}

inline WString& WString::operator=(WString&& other) noexcept
{
	hashed = false;
	if (!other.string) { Destroy(); return *this; }

	if (string) { Memory::FreeArray(&string); }

	size = other.size;
	capacity = other.capacity;
	string = other.string;
	other.size = 0;
	other.capacity = 0;
	other.string = nullptr;

	return *this;
}

inline WString::~WString()
{
	Destroy();
}

inline void WString::Destroy()
{
	hashed = false;
	hash = 0;
	if (string)
	{
		size = 0;
		capacity = 0;
		Memory::FreeArray(&string);
	}
}

inline void WString::Clear()
{
	hashed = false;
	string[0] = NULL_CHAR;
	size = 0;
}

inline void WString::Reserve(U64 size)
{
	if (size > capacity)
	{
		Memory::Reallocate(&string, capacity = size);
	}
}

inline void WString::Resize(U64 size)
{
	if (size > this->capacity) { Reserve(size); }
	this->size = size;
	string[size] = NULL_CHAR;
}

inline void WString::Resize()
{
	this->size = wcslen(string);
}

template<typename T> inline WString& WString::operator+=(T value)
{
	ToWString(string + size, value);
	return *this;
}

inline WString& WString::operator+=(C16* other)
{
	hashed = false;
	U64 addLength = wcslen(other);
	if (capacity < size + addLength) { Memory::Reallocate(&string, capacity = size + addLength); }
	memcpy(string + size, other, addLength * sizeof(C16));
	size += addLength;
	string[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::operator+=(const C16* other)
{
	hashed = false;
	U64 addLength = wcslen(other);
	if (capacity < size + addLength) { Memory::Reallocate(&string, capacity = size + addLength); }
	memcpy(string + size, other, addLength * sizeof(C16));
	size += addLength;
	string[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::operator+=(const WString& other)
{
	hashed = false;
	if (capacity < size + other.size) { Memory::Reallocate(&string, capacity = size + other.size); }
	memcpy(string + size, other.string, other.size * sizeof(C16));
	size += other.size;
	string[size] = NULL_CHAR;

	return *this;
}

template<typename T> inline WString::operator T() const { return ToType<T>(); }

inline WString::operator C16* () { return string; }

inline WString::operator const C16* () const { return string; }

inline C16* WString::operator*() { return string; }

inline const C16* WString::operator*() const { return string; }

inline C16& WString::operator[](U32 i) { return string[i]; }

inline const C16& WString::operator[](U32 i) const { return string[i]; }

inline bool WString::operator==(C16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(string, other, size * sizeof(C16)) == 0;
}

inline bool WString::operator==(const C16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(string, other, size * sizeof(C16)) == 0;
}

inline bool WString::operator==(const WString& other) const
{
	if (other.size != size) { return false; }

	return memcmp(string, other.string, size * sizeof(C16)) == 0;
}

inline bool WString::operator!=(C16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return true; }

	return memcmp(string, other, size * sizeof(C16));
}

inline bool WString::operator!=(const C16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return true; }

	return memcmp(string, other, size * sizeof(C16));
}

inline bool WString::operator!=(const WString& other) const
{
	if (other.size != size) { return true; }

	return memcmp(string, other.string, size * sizeof(C16));
}

inline bool WString::Compare(C16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(string, other, size * sizeof(C16)) == 0;
}

inline bool WString::Compare(const C16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(string, other, size * sizeof(C16)) == 0;
}

inline bool WString::Compare(const WString& other) const
{
	if (other.size != size) { return false; }

	return memcmp(string, other.string, size * sizeof(C16)) == 0;
}

inline bool WString::CompareN(C16* other, U32 nLength, U32 start) const
{
	U64 len = wcslen(other);
	if (len != nLength) { return false; }

	return memcmp(string + start, other, nLength * sizeof(C16)) == 0;
}

inline bool WString::CompareN(const C16* other, U32 nLength, U32 start) const
{
	U64 len = wcslen(other);
	if (len != nLength) { return false; }

	return memcmp(string + start, other, nLength * sizeof(C16)) == 0;
}

inline bool WString::CompareN(const WString& other, U32 nLength, U32 start) const
{
	if (other.size != nLength) { return false; }

	return memcmp(string + start, other.string, nLength * sizeof(C16)) == 0;
}

inline const U64& WString::Size() const { return size; }

inline const U64& WString::Capacity() const { return capacity; }

inline U64 WString::Hash()
{
	if (hashed) { return hash; }

	hash = 0;
	const C16* ptr = (C16*)string;
	while (*ptr) { hash = hash * 101 + *ptr++; }
	hashed = true;

	return hash;
}

inline C16* WString::Data() { return string; }

inline const C16* WString::Data() const { return string; }

inline bool WString::Blank() const
{
	if (size == 0) { return true; }
	C16* start = string;
	C16 c;

	while (WHITE_SPACE_W(c, start));

	return start - string == size;
}

inline I32 WString::IndexOf(const C16& c, U64 start) const
{
	C16* it = string + start;

	while (*it != c && *it != NULL_CHAR) { ++it; }

	if (*it == NULL_CHAR) { return -1; }
	return (I32)(it - string);
}

inline I32 WString::LastIndexOf(const C16& c, U64 start) const
{
	C16* it = string + size - start - 1;

	U64 len = size;
	while (*it != c && len > 0) { --it; --len; }

	if (len) { return (I32)(it - string); }
	return -1;
}

inline WString& WString::Trim()
{
	hashed = false;
	C16* start = string;
	C16* end = string + size;
	C16 c;

	while (WHITE_SPACE_W(c, start)) { ++start; }
	while (WHITE_SPACE_W(c, end)) { --end; }

	size = end - start;
	memcpy(string, start, size * sizeof(C16));
	string[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::SubString(WString& newStr, U64 start, U64 nLength) const
{
	if (nLength < U64_MAX) { newStr.size = nLength; }
	else { newStr.size = size - start; }

	memcpy(newStr.string, string + start, newStr.size * sizeof(C16));
	newStr.string[newStr.size] = NULL_CHAR;

	return newStr;
}

inline WString& WString::Append(const WString& append)
{
	if (capacity < size + append.size) { Memory::Reallocate(&string, capacity = size + append.size); }
	hashed = false;
	memcpy(string + size, append.string, append.size * sizeof(C16));
	size += append.size;
	string[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::Prepend(const WString& prepend)
{
	if (capacity < size + prepend.size) { Memory::Reallocate(&string, capacity = size + prepend.size); }
	hashed = false;
	memcpy(string + size, string, size * sizeof(C16));
	memcpy(string, prepend.string, prepend.size * sizeof(C16));
	size += prepend.size;
	string[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::Surround(const WString& prepend, const WString& append)
{
	if (capacity < size + append.size + prepend.size) { Memory::Reallocate(&string, capacity = size + append.size + prepend.size); }
	hashed = false;
	memcpy(string + prepend.size, string, size * sizeof(C16));
	memcpy(string, prepend.string, prepend.size * sizeof(C16));
	size += prepend.size;

	memcpy(string + size, append.string, append.size * sizeof(C16));
	size += append.size;
	string[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::Insert(const WString& other, U32 i)
{
	if (capacity < size + other.size) { Memory::Reallocate(&string, capacity = size + other.size); }
	hashed = false;
	memcpy(string + i + other.size, string + i, size - i);
	memcpy(string + i, other.string, other.size);
	size += other.size;
	string[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::Overwrite(const WString& other, U32 i)
{
	C16* c = string + i;
	memcpy(c, other.string, (other.size + 1) * sizeof(C16));

	return *this;
}

inline WString& WString::ReplaceAll(const WString& find, const WString& replace, U64 start)
{
	//TODO: Capacity Check
	hashed = false;
	C16* c = string + start;
	C16 ch = *c;
	while (ch != NULL_CHAR)
	{
		while ((ch = *c) != NULL_CHAR && memcmp(c, find.string, find.size * sizeof(C16))) { ++c; }

		if (ch != NULL_CHAR)
		{
			memcpy(c + replace.size, c + find.size, (size - find.size - (c - string)) * sizeof(C16));
			memcpy(c, replace.string, replace.size * sizeof(C16));
			size = size - find.size + replace.size;
			string[size] = NULL_CHAR;
		}
	}

	return *this;
}

inline WString& WString::ReplaceN(const WString& find, const WString& replace, U64 count, U64 start)
{
	if (capacity < size + replace.size * count - find.size * count) { Memory::Reallocate(&string, capacity = size + replace.size * count - find.size * count); }

	hashed = false;
	C16* c = string + start;
	C16 ch = *c;
	while (ch != NULL_CHAR && count)
	{
		while ((ch = *c) != NULL_CHAR && memcmp(c, find.string, find.size * sizeof(C16))) { ++c; }

		if (ch != NULL_CHAR)
		{
			--count;
			memcpy(c + replace.size, c + find.size, (size - find.size - (c - string)) * sizeof(C16));
			memcpy(c, replace.string, replace.size * sizeof(C16));
			size = size - find.size + replace.size;
			string[size] = NULL_CHAR;
		}
	}

	return *this;
}

inline WString& WString::ReplaceFirst(const WString& find, const WString& replace, U64 start)
{
	if (capacity < size + replace.size - find.size) { Memory::Reallocate(&string, capacity = size + replace.size - find.size); }

	hashed = false;
	C16* c = string + start;
	while (*c != NULL_CHAR && memcmp(c, find.string, find.size * sizeof(C16))) { ++c; }

	if (*c != NULL_CHAR)
	{
		memcpy(c + replace.size, c + find.size, (size - find.size - (c - string)) * sizeof(C16));
		memcpy(c, replace.string, replace.size * sizeof(C16));
		size = size - find.size + replace.size;
		string[size] = NULL_CHAR;
	}

	return *this;
}

inline void WString::Split(Vector<WString>& list, U8 delimiter, bool trimEntries) const
{

}

inline void WString::Format(U64& start, const WString& replace)
{
	if (capacity < size - 2 + replace.size) { Memory::Reallocate(&string, capacity = size - 2 + replace.size); }

	hashed = false;
	C16* c = string + start;
	while (*c != NULL_CHAR && memcmp(c, L"{}", 4)) { ++c; }

	if (*c != NULL_CHAR)
	{
		start = (c - string) + replace.size;
		memcpy(c + replace.size, c + 2, (size - 2 - (c - string)) * sizeof(C16));
		memcpy(c, replace.string, replace.size * sizeof(C16));
		size = size - 2 + replace.size;
		string[size] = NULL_CHAR;
	}
}

inline C16* WString::begin() { return string; }

inline C16* WString::end() { return string + size; }

inline const C16* WString::begin() const { return string; }

inline const C16* WString::end() const { return string + size; }

inline C16* WString::rbegin() { return string + size - 1; }

inline C16* WString::rend() { return string - 1; }

inline const C16* WString::rbegin() const { return string + size - 1; }

inline const C16* WString::rend() const { return string - 1; }

template<typename T> inline EnableForSignedInt<T> WString::ToString(C16* str, T value)
{
	hashed = false;
	if (!string || capacity < size + 20) { Memory::Reallocate(&string, capacity = size + 20); }

	C16* c = str + 20;
	const C16* threeDigits;
	U8 neg = 0;

	U64 abs = (U64)value;

	if (value < 0)
	{
		str[0] = NEGATIVE_CHAR;
		abs = (U64)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U64 newVal = abs / 1000;
		U64 remainder = abs % 1000;
		threeDigits = DECIMAL_LOOKUP + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = DECIMAL_LOOKUP + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	U64 addLength = 20 + neg - (c - str);
	size += addLength;

	memcpy(str + neg, c, addLength * sizeof(C16));
	str[size] = NULL_CHAR;
}

template<typename T> inline EnableForUnsignedInt<T> WString::ToString(C16* str, T value)
{
	hashed = false;
	if (!string || capacity < size + 20) { Memory::Reallocate(&string, capacity = size + 20); }

	C16* c = str + 20;
	const C16* threeDigits;
	U64 val = value;

	while (val > 999)
	{
		U64 newVal = val / 1000;
		U64 remainder = val % 1000;
		threeDigits = DECIMAL_LOOKUP + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		val = newVal;
	}

	threeDigits = DECIMAL_LOOKUP + (val * 3);
	*--c = threeDigits[2];
	if (val > 9) { *--c = threeDigits[1]; }
	if (val > 99) { *--c = threeDigits[0]; }

	U64 addLength = 20 - (c - str);
	size += addLength;

	memcpy(str, c, addLength * sizeof(C16));
	str[size] = NULL_CHAR;
}

template<typename T> inline EnableForBool<T> WString::ToString(C16* str, T value)
{
	hashed = false;
	if (value)
	{
		if (!string || capacity < size + 4) { Memory::Reallocate(&string, capacity = size + 4); }
		memcpy(str + size, TRUE_STR, 4 * sizeof(C16));
		size += 4;
		str[size] = NULL_CHAR;
	}
	else
	{
		if (!string || capacity < size + 5) { Memory::Reallocate(&string, capacity = size + 5); }
		memcpy(str + size, FALSE_STR, 5 * sizeof(C16));
		size += 5;
		str[size] = NULL_CHAR;
	}
}

template<typename T> inline EnableForFloat<T> WString::ToString(C16* str, T value)
{
	hashed = false;
	if (!string || capacity < size + 27) { Memory::Reallocate(&string, capacity = size + 27); }

	C16* c = str + 27;
	const C16* threeDigits;
	U8 neg = 0;

	F64 abs = value;

	if (value < 0)
	{
		str[0] = NEGATIVE_CHAR;
		abs = -value;
		neg = 1;
	}

	U64 dec = (U64)((abs - (F64)(U64)abs) * 100000.0f);

	U64 newVal = dec / 1000;
	U64 remainder = dec % 1000;
	threeDigits = DECIMAL_LOOKUP + (remainder * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = threeDigits[0];
	dec = newVal;

	threeDigits = DECIMAL_LOOKUP + (dec * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = DECIMAL_CHAR;

	U64 whole = (U64)abs;

	while (whole > 999)
	{
		U64 newVal = whole / 1000;
		U64 remainder = whole % 1000;
		threeDigits = DECIMAL_LOOKUP + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		whole = newVal;
	}

	threeDigits = DECIMAL_LOOKUP + (whole * 3);
	*--c = threeDigits[2];
	if (whole > 9) { *--c = threeDigits[1]; }
	if (whole > 99) { *--c = threeDigits[0]; }

	U64 addLength = 27 + neg - (c - str);
	size += addLength;

	memcpy(str + neg, c, addLength * sizeof(C16));
	str[size] = NULL_CHAR;
}

template<typename T> inline EnableForPointer<T> WString::ToString(C16* str, T value)
{
	ToString(str, (U64)value);
}

template<typename T> inline EnableForSignedInt<T> WString::HexToString(C16* str, T value)
{
	hashed = false;
	if (!string || capacity < size + 16) { Memory::Reallocate(&string, capacity = size + 16); }

	U8 pairs;
	U8 digits;
	U64 max;

	if constexpr (IsSame<T, U8>) { pairs = 1; digits = 2; max = U8_MAX; }
	else if constexpr (IsSame<T, U16>) { pairs = 2; digits = 4; max = U16_MAX; }
	else if constexpr (IsSame<T, U32>) { pairs = 4; digits = 8; max = U32_MAX; }
	else if constexpr (IsSame<T, UL32>) { pairs = 4; digits = 8; max = U32_MAX; }
	else { pairs = 8; digits = 16; max = U64_MAX; }

	C16* c = str + digits;
	const C16* twoDigits;

	U64 abs;
	if (value < 0)
	{
		abs = max - ((U64)-value - 1);

		for (U8 i = 0; i < pairs; ++i)
		{
			U64 j = abs & 0xFF;
			twoDigits = HEX_LOOKUP + (abs & 0xFF) * 2;

			*--c = twoDigits[1];
			*--c = twoDigits[0];

			abs >>= 8;
		}
	}
	else
	{
		abs = (U64)value;

		for (U8 i = 0; i < pairs; ++i)
		{
			twoDigits = HEX_LOOKUP + (abs & 0xFF) * 2;

			*--c = twoDigits[1];
			*--c = twoDigits[0];

			abs >>= 8;
		}
	}

	size += digits;

	memcpy(str, c, digits * sizeof(C16));
	str[size] = NULL_CHAR;
}

template<typename T> inline EnableForUnsignedInt<T> WString::HexToString(C16* str, T value)
{
	hashed = false;
	if (!string || capacity < size + 16) { Memory::Reallocate(&string, capacity = size + 16); }

	U8 pairs;
	U8 digits;
	if constexpr (IsSame<T, U8>) { pairs = 1; digits = 2; }
	else if constexpr (IsSame<T, U16>) { pairs = 2; digits = 4; }
	else if constexpr (IsSame<T, U32>) { pairs = 4; digits = 8; }
	else if constexpr (IsSame<T, UL32>) { pairs = 4; digits = 8; }
	else { pairs = 8; digits = 16; }

	C16* c = str + digits;
	const C16* twoDigits;
	U64 val = value;

	for (U8 i = 0; i < pairs; ++i)
	{
		twoDigits = HEX_LOOKUP + (val & 0xFF) * 2;

		*--c = twoDigits[1];
		*--c = twoDigits[0];

		val >>= 8;
	}

	size += digits;

	memcpy(str, c, digits * sizeof(C16));
	str[size] = NULL_CHAR;
}

template<typename T> inline EnableForFloat<T> WString::HexToString(C16* str, T value)
{
	hashed = false;
	if (!string || capacity < size + 16) { Memory::Reallocate(&string, capacity = size + 16); }

	U8 pairs = 8;
	U8 digits = 16;

	C16* c = str + digits;
	const C16* twoDigits;
	U64 val = *reinterpret_cast<U64*>(&value);

	for (U8 i = 0; i < pairs; ++i)
	{
		twoDigits = HEX_LOOKUP + (val & 0xFF) * 2;

		*--c = twoDigits[1];
		*--c = twoDigits[0];

		val >>= 8;
	}

	size += digits;

	memcpy(str, c, digits * sizeof(C16));
	str[size] = NULL_CHAR;
}

template<typename T> inline EnableForSignedInt<T, T> WString::ToType(U64 start) const
{
	C16* it = string + start;
	C16 c;
	I64 value = 0;

	if (*string == NEGATIVE_CHAR)
	{
		++it;
		while (NOT_WHITE_SPACE_W(c, it) && c != NULL_CHAR) { value *= 10; value -= c - ZERO_CHAR; ++it; }
	}
	else
	{
		while (NOT_WHITE_SPACE_W(c, it)) { value *= 10; value += c - ZERO_CHAR; ++it; }
	}

	return value;
}

template<typename T> inline EnableForUnsignedInt<T, T> WString::ToType(U64 start) const
{
	C16* it = string + start;
	C16 c;
	T value = 0;

	while (NOT_WHITE_SPACE_W(c, it) && c != NULL_CHAR) { value *= 10; value += c - ZERO_CHAR; ++it; }

	return value;
}

template<typename T> inline EnableForBool<T, T> WString::ToType(U64 start) const
{
	C16* c = string + start;
	return IS_TRUE_W(c);
}

template<typename T> inline EnableForFloat<T, T> WString::ToType(U64 start) const
{
	C16* it = string + start;
	C16 c;
	F64 value = 0.0f;
	F64 mul = 0.1f;

	if (*string == NEGATIVE_CHAR)
	{
		++it;
		while (NOT_WHITE_SPACE_W(c, it) && c != NULL_CHAR && c != DECIMAL_CHAR) { value *= 10; value -= c - ZERO_CHAR; ++it; }
		while (NOT_WHITE_SPACE_W(c, it) && c != NULL_CHAR) { value -= (c - ZERO_CHAR) * mul; mul *= 0.1f; ++it; }
	}
	else
	{
		while (NOT_WHITE_SPACE_W(c, it) && c != NULL_CHAR && c != DECIMAL_CHAR) { value *= 10; value += c - ZERO_CHAR; ++it; }
		while (NOT_WHITE_SPACE_W(c, it) && c != NULL_CHAR) { value += (c - ZERO_CHAR) * mul; mul *= 0.1f; ++it; }
	}

	return value;
}

template<typename T> inline EnableForPointer<T, T> WString::ToType(U64 start) const
{
	return (T)ToType<U64>(start);
}