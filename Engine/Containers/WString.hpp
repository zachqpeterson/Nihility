#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"
#include "Vector.hpp"

#define IS_TRUE_W(c) c[0] == L't' && c[1] == L'r' && c[2] == L'u' && c[3] == L'e'
#define WHITE_SPACE_W(c, ptr) (c = *ptr) == L' ' || c == L'\t' || c == L'\r' || c == L'\n' || c == L'\v' || c == L'\f'
#define NOT_WHITE_SPACE(c, ptr) (c = *ptr) != L' ' && c != L'\t' && c != L'\r' && c != L'\n' && c != L'\v' && c != L'\f'

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
* TODO: Conversions from char to W16
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
	WString(W16* str);
	WString(const W16* str);
	WString(const WString& other);
	WString(WString&& other) noexcept;
	template<typename... Types> WString(const W16* fmt, const Types& ... args);
	template<typename... Types> WString(const Types& ... args);

	template<typename T> WString& operator=(T value);
	WString& operator=(W16* str);
	WString& operator=(const W16* str);
	WString& operator=(const WString& other);
	WString& operator=(WString&& other) noexcept;

	~WString();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);
	void Resize();

	template<typename T> std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>, T> ToType() const;
	template<typename T> std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>, T> ToType() const;
	template<typename T> std::enable_if_t<std::is_integral_v<T> && !std::_Is_nonbool_integral<T>, T> ToType() const;
	template<typename T> std::enable_if_t<std::is_floating_point_v<T>, T> ToType() const;

	template<typename T> WString& operator+=(T value);
	WString& operator+=(W16* other);
	WString& operator+=(const W16* other);
	WString& operator+=(const WString& other);

	template<typename T> explicit operator T() const;
	explicit operator W16* ();
	explicit operator const W16* () const;

	W16* operator*();
	const W16* operator*() const;
	W16& operator[](U32 i);
	const W16& operator[](U32 i) const;

	bool operator==(W16* other) const;
	bool operator==(const W16* other) const;
	bool operator==(const WString& other) const;
	bool operator!=(W16* other) const;
	bool operator!=(const W16* other) const;
	bool operator!=(const WString& other) const;

	bool Compare(W16* other) const;
	bool Compare(const W16* other) const;
	bool Compare(const WString& other) const;
	bool CompareN(W16* other, U32 nLength, U32 start = 0) const;
	bool CompareN(const W16* other, U32 nLength, U32 start = 0) const;
	bool CompareN(const WString& other, U32 nLength, U32 start = 0) const;

	const U64& Size() const;
	const U64& Capacity() const;
	U64 Hash();
	W16* Data();
	const W16* Data() const;
	bool Blank() const;
	I32 IndexOf(const W16& c, U64 start = 0) const;
	I32 LastIndexOf(const W16& c, U64 start = 0) const;
	WString& Trim();
	WString& SubString(WString& newStr, U64 start, U64 nLength = I64_MAX) const;
	WString& Append(const WString& append);
	WString& Prepend(const WString& prepend);
	WString& Surround(const WString& prepend, const WString& append);
	WString& Insert(const WString& WString, U32 i);
	WString& Overwrite(const WString& string, U32 i = 0);
	WString& ReplaceAll(const WString& find, const WString& replace, U64 start = 0);
	WString& ReplaceN(const WString& find, const WString& replace, U64 count, U64 start = 0);
	WString& ReplaceFirst(const WString& find, const WString& replace, U64 start = 0);
	void Split(Vector<WString>& list, U8 delimiter, bool trimEntries) const;

	W16* begin();
	W16* end();
	const W16* begin() const;
	const W16* end() const;

	W16* rbegin();
	W16* rend();
	const W16* rbegin() const;
	const W16* rend() const;

	static inline constexpr const W16 NULL_CHAR = L'\0';
	static inline constexpr const W16 NEGATIVE_CHAR = L'-';
	static inline constexpr const W16 DECIMAL_CHAR = L'.';
	static inline constexpr const W16 ZERO_CHAR = L'0';
	static inline constexpr const W16* TRUE_STR = L"true";
	static inline constexpr const W16* FALSE_STR = L"false";

private:
	template<typename T> std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>> ToWString(W16* str, T value);
	template<typename T> std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>> ToWString(W16* str, T value);
	template<typename T> std::enable_if_t<std::is_integral_v<T> && !std::_Is_nonbool_integral<T>> ToWString(W16* str, T value);
	template<typename T> std::enable_if_t<std::is_floating_point_v<T>> ToWString(W16* str, T value);
	template<typename T> std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>> HexToWString(W16* str, T value);
	template<typename T> std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>> HexToWString(W16* str, T value);
	template<typename T> std::enable_if_t<std::is_floating_point_v<T>> HexToWString(W16* str, T value);

	void Format(U64& start, const WString& replace);

	bool hashed{ false };
	U64 hash{ 0 };
	U64 size{ 0 };
	U64 capacity{ 0 };
	W16* str{ nullptr };

#pragma region LOOKUPS
	static inline constexpr const W16 DECIMAL_LOOKUP[] =
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

	static inline constexpr const W16 HEX_LOOKUP[] =
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

inline WString::WString() { }

template<typename T> inline WString::WString(T value) : str{ (W16*)Memory::Allocate1kb() } { ToWString(str, value); }

template<typename T> inline WString::WString(T value, Hex flag) : str{ (W16*)Memory::Allocate1kb() } { HexToWString(str, value); }

inline WString::WString(W16* str) : size{ wcslen(str) }, capacity{ size }, str{ (W16*)Memory::Allocate(capacity, capacity) }
{
	memcpy(this->str, str, (size + 1) * sizeof(W16));
}

inline WString::WString(const W16* str) : size{ wcslen(str) }, capacity{ size }, str{ (W16*)Memory::Allocate(capacity, capacity) }
{
	memcpy(this->str, str, (size + 1) * sizeof(W16));
}

inline WString::WString(const WString& other) : size{ other.size }, capacity{ other.capacity }, str{ (W16*)Memory::Allocate(capacity) }
{
	memcpy(str, other.str, (size + 1) * sizeof(W16));
}

inline WString::WString(WString&& other) noexcept : size{ other.size }, capacity{ other.capacity }, str{ other.str }
{
	other.size = 0;
	other.capacity = 0;
	other.str = nullptr;
}

template<typename... Types> inline WString::WString(const W16* fmt, const Types& ... args) : size{ wcslen(fmt) }, capacity{ size }, str{ (W16*)Memory::Allocate(capacity, capacity) }
{
	memcpy(str, fmt, (size + 1) * sizeof(W16));
	U64 start = 0;
	(Format(start, args), ...);
}

template<typename... Types> inline WString::WString(const Types& ... args) : str{ (W16*)Memory::Allocate1kb() }
{
	(Append(args), ...);
}

template<typename T> inline WString& WString::operator=(T value)
{
	ToWString(str, value);
	return *this;
}

inline WString& WString::operator=(W16* str)
{
	hashed = false;
	size = wcslen(str);
	if (capacity < size && this->str) { Memory::Free(this->str); }
	if (!this->str) { this->str = (W16*)Memory::Allocate(size * sizeof(W16), capacity); }

	memcpy(this->str, str, size + 1);

	return *this;
}

inline WString& WString::operator=(const W16* str)
{
	hashed = false;
	size = wcslen(str);
	if (capacity < size && this->str) { Memory::Free(this->str); }
	if (!this->str) { this->str = (W16*)Memory::Allocate(size * sizeof(W16), capacity); }

	memcpy(this->str, str, size + 1);

	return *this;
}

inline WString& WString::operator=(const WString& other)
{
	hashed = false;
	if (capacity < other.size && str) { Memory::Free(str); }
	if (!str) { str = (W16*)Memory::Allocate(other.capacity); }

	size = other.size;
	capacity = other.capacity;
	memcpy(this->str, other.str, size + 1);

	return *this;
}

inline WString& WString::operator=(WString&& other) noexcept
{
	hashed = false;
	if (str) { Memory::Free(str); }

	size = other.size;
	capacity = other.capacity;
	str = other.str;
	other.size = 0;
	other.capacity = 0;
	other.str = nullptr;

	return *this;
}

inline WString::~WString()
{
	Destroy();
}

inline void WString::Destroy()
{
	hashed = false;
	if (str)
	{
		size = 0;
		capacity = 0;
		Memory::Free(str);
		str = nullptr;
	}
}

inline void WString::Clear()
{
	hashed = false;
	str[0] = NULL_CHAR;
	size = 0;
}

inline void WString::Reserve(U64 size)
{
	if (size > capacity)
	{
		W16* temp = (W16*)Memory::Allocate(size * sizeof(W16), capacity);
		memcpy(temp, str, this->size * sizeof(W16));
		Memory::Free(str);
		str = temp;
	}
}

inline void WString::Resize(U64 size)
{
	if (size > this->capacity) { Reserve(size); }
	this->size = size;
	str[size] = NULL_CHAR;
}

inline void WString::Resize()
{
	this->size = wcslen(str);
}

template<typename T> inline WString& WString::operator+=(T value)
{
	ToWString(str + size, value);
	return *this;
}

inline WString& WString::operator+=(W16* other)
{
	hashed = false;
	U64 addLength = wcslen(other);
	if (capacity < size + addLength) { Memory::Reallocate(str, (size + addLength) * sizeof(W16), capacity); }
	memcpy(str + size, other, addLength * sizeof(W16));
	size += addLength;
	str[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::operator+=(const W16* other)
{
	hashed = false;
	U64 addLength = wcslen(other);
	if (capacity < size + addLength) { Memory::Reallocate(str, (size + addLength) * sizeof(W16), capacity); }
	memcpy(str + size, other, addLength * sizeof(W16));
	size += addLength;
	str[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::operator+=(const WString& other)
{
	hashed = false;
	if (capacity < size + other.size) { Memory::Reallocate(str, (size + other.size) * sizeof(W16), capacity); }
	memcpy(str + size, other.str, other.size * sizeof(W16));
	size += other.size;
	str[size] = NULL_CHAR;

	return *this;
}

template<typename T> inline WString::operator T() const { return ToType<T>(); }

inline WString::operator W16* () { return str; }

inline WString::operator const W16* () const { return str; }

inline W16* WString::operator*() { return str; }

inline const W16* WString::operator*() const { return str; }

inline W16& WString::operator[](U32 i) { return str[i]; }

inline const W16& WString::operator[](U32 i) const { return str[i]; }

inline bool WString::operator==(W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size * sizeof(W16)) == 0;
}

inline bool WString::operator==(const W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size * sizeof(W16)) == 0;
}

inline bool WString::operator==(const WString& other) const
{
	if (other.size != size) { return false; }

	return memcmp(str, other.str, size * sizeof(W16)) == 0;
}

inline bool WString::operator!=(W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return true; }

	return memcmp(str, other, size * sizeof(W16));
}

inline bool WString::operator!=(const W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return true; }

	return memcmp(str, other, size * sizeof(W16));
}

inline bool WString::operator!=(const WString& other) const
{
	if (other.size != size) { return true; }

	return memcmp(str, other.str, size * sizeof(W16));
}

inline bool WString::Compare(W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size * sizeof(W16)) == 0;
}

inline bool WString::Compare(const W16* other) const
{
	U64 len = wcslen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size * sizeof(W16)) == 0;
}

inline bool WString::Compare(const WString& other) const
{
	if (other.size != size) { return false; }

	return memcmp(str, other.str, size * sizeof(W16)) == 0;
}

inline bool WString::CompareN(W16* other, U32 nLength, U32 start) const
{
	U64 len = wcslen(other);
	if (len != nLength) { return false; }

	return memcmp(str + start, other, nLength * sizeof(W16)) == 0;
}

inline bool WString::CompareN(const W16* other, U32 nLength, U32 start) const
{
	U64 len = wcslen(other);
	if (len != nLength) { return false; }

	return memcmp(str + start, other, nLength * sizeof(W16)) == 0;
}

inline bool WString::CompareN(const WString& other, U32 nLength, U32 start) const
{
	if (other.size != nLength) { return false; }

	return memcmp(str + start, other.str, nLength * sizeof(W16)) == 0;
}

inline const U64& WString::Size() const { return size; }

inline const U64& WString::Capacity() const { return capacity; }

inline U64 WString::Hash()
{
	if (hashed) { return hash; }

	hash = 0;
	const char* ptr = (char*)str;
	while (*ptr) { hash = hash * 101 + *ptr++; }
	hashed = true;

	return hash;
}

inline W16* WString::Data() { return str; }

inline const W16* WString::Data() const { return str; }

inline bool WString::Blank() const
{
	if (size == 0) { return true; }
	W16* start = str;
	W16 c;

	while (WHITE_SPACE_W(c, start));

	return start - str == size;
}

inline I32 WString::IndexOf(const W16& c, U64 start) const
{
	W16* it = str + start;

	while (*it != c && *it != NULL_CHAR) { ++it; }

	if (*it == NULL_CHAR) { return -1; }
	return (I32)(it - str);
}

inline I32 WString::LastIndexOf(const W16& c, U64 start) const
{
	W16* it = str + size - start - 1;

	U64 len = size;
	while (*it != c && len > 0) { --it; --len; }

	if (len) { return (I32)(it - str); }
	return -1;
}

inline WString& WString::Trim()
{
	hashed = false;
	W16* start = str;
	W16* end = str + size;
	W16 c;

	while (WHITE_SPACE_W(c, start)) { ++start; }
	while (WHITE_SPACE_W(c, end)) { --end; }

	size = end - start;
	memcpy(str, start, size * sizeof(W16));
	str[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::SubString(WString& newStr, U64 start, U64 nLength) const
{
	if (nLength < U64_MAX) { newStr.size = nLength; }
	else { newStr.size = size - start; }

	memcpy(newStr.str, str + start, newStr.size * sizeof(W16));
	newStr.str[newStr.size] = NULL_CHAR;

	return newStr;
}

inline WString& WString::Append(const WString& append)
{
	if (capacity < size + append.size) { Memory::Reallocate(str, size + append.size, capacity); }
	hashed = false;
	memcpy(str + size, append.str, append.size * sizeof(W16));
	size += append.size;
	str[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::Prepend(const WString& prepend)
{
	if (capacity < size + prepend.size) { Memory::Reallocate(str, (size + prepend.size) * sizeof(W16), capacity); }
	hashed = false;
	memcpy(str + size, str, size * sizeof(W16));
	memcpy(str, prepend.str, prepend.size * sizeof(W16));
	size += prepend.size;
	str[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::Surround(const WString& prepend, const WString& append)
{
	if (capacity < size + append.size + prepend.size) { Memory::Reallocate(str, (size + append.size + prepend.size) * sizeof(W16), capacity); }
	hashed = false;
	memcpy(str + prepend.size, str, size * sizeof(W16));
	memcpy(str, prepend.str, prepend.size * sizeof(W16));
	size += prepend.size;

	memcpy(str + size, append.str, append.size * sizeof(W16));
	size += append.size;
	str[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::Insert(const WString& other, U32 i)
{
	if (capacity < size + other.size) { Memory::Reallocate(str, (size + other.size) * sizeof(W16), capacity); }
	hashed = false;
	memcpy(str + i + other.size, str + i, size - i);
	memcpy(str + i, other.str, other.size);
	size += other.size;
	str[size] = NULL_CHAR;

	return *this;
}

inline WString& WString::Overwrite(const WString& string, U32 i)
{
	W16* c = str + i;
	memcpy(c, string.str, (string.size + 1) * sizeof(W16));

	return *this;
}

inline WString& WString::ReplaceAll(const WString& find, const WString& replace, U64 start)
{
	//TODO: Capacity Check
	hashed = false;
	W16* c = str + start;
	W16 ch = *c;
	while (ch != NULL_CHAR)
	{
		while ((ch = *c) != NULL_CHAR && memcmp(c, find.str, find.size * sizeof(W16))) { ++c; }

		if (ch != NULL_CHAR)
		{
			memcpy(c + replace.size, c + find.size, (size - find.size - (c - str)) * sizeof(W16));
			memcpy(c, replace.str, replace.size * sizeof(W16));
			size = size - find.size + replace.size;
			str[size] = NULL_CHAR;
		}
	}

	return *this;
}

inline WString& WString::ReplaceN(const WString& find, const WString& replace, U64 count, U64 start)
{
	//TODO: Capacity Check
	hashed = false;
	W16* c = str + start;
	W16 ch = *c;
	while (ch != NULL_CHAR && count)
	{
		while ((ch = *c) != NULL_CHAR && memcmp(c, find.str, find.size * sizeof(W16))) { ++c; }

		if (ch != NULL_CHAR)
		{
			--count;
			memcpy(c + replace.size, c + find.size, (size - find.size - (c - str)) * sizeof(W16));
			memcpy(c, replace.str, replace.size * sizeof(W16));
			size = size - find.size + replace.size;
			str[size] = NULL_CHAR;
		}
	}

	return *this;
}

inline WString& WString::ReplaceFirst(const WString& find, const WString& replace, U64 start)
{
	if (capacity < size + replace.size - find.size) { Memory::Reallocate(str, (size + replace.size - find.size) * sizeof(W16), capacity); }
	hashed = false;
	W16* c = str + start;
	while (*c != NULL_CHAR && memcmp(c, find.str, find.size * sizeof(W16))) { ++c; }

	if (*c != NULL_CHAR)
	{
		memcpy(c + replace.size, c + find.size, (size - find.size - (c - str)) * sizeof(W16));
		memcpy(c, replace.str, replace.size * sizeof(W16));
		size = size - find.size + replace.size;
		str[size] = NULL_CHAR;
	}

	return *this;
}

inline void WString::Split(Vector<WString>& list, U8 delimiter, bool trimEntries) const
{

}

inline void WString::Format(U64& start, const WString& replace)
{
	//TODO: Capacity Check
	hashed = false;
	W16* c = str + start;
	while (*c != NULL_CHAR && memcmp(c, L"{}", 4)) { ++c; }

	if (*c != NULL_CHAR)
	{
		start = (c - str) + replace.size;
		memcpy(c + replace.size, c + 2, (size - 2 - (c - str)) * sizeof(W16));
		memcpy(c, replace.str, replace.size * sizeof(W16));
		size = size - 2 + replace.size;
		str[size] = NULL_CHAR;
	}
}

inline W16* WString::begin() { return str; }

inline W16* WString::end() { return str + size; }

inline const W16* WString::begin() const { return str; }

inline const W16* WString::end() const { return str + size; }

inline W16* WString::rbegin() { return str + size - 1; }

inline W16* WString::rend() { return str - 1; }

inline const W16* WString::rbegin() const { return str + size - 1; }

inline const W16* WString::rend() const { return str - 1; }

template<typename T> inline std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>> WString::ToWString(W16* str, T value)
{
	hashed = false;
	if (capacity < size + 20 && this->str) { Memory::Free(str); }
	if (!str) { str = (W16*)Memory::Allocate((size + 20) * sizeof(W16), capacity); }

	W16* c = str + 20;
	const W16* threeDigits;
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

	memcpy(str + neg, c, addLength * sizeof(W16));
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>> WString::ToWString(W16* str, T value)
{
	hashed = false;
	if (capacity < size + 20 && this->str) { Memory::Free(str); }
	if (!str) { str = (W16*)Memory::Allocate((size + 20) * sizeof(W16), capacity); }

	W16* c = str + 20;
	const W16* threeDigits;
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

	memcpy(str, c, addLength * sizeof(W16));
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_integral_v<T> && !std::_Is_nonbool_integral<T>> WString::ToWString(W16* str, T value)
{
	hashed = false;
	if (value)
	{
		if (capacity < size + 4 && this->str) { Memory::Free(str); }
		if (!str) { str = (W16*)Memory::Allocate((size + 4) * sizeof(W16), capacity); }
		memcpy(str + size, TRUE_STR, 4 * sizeof(W16));
		size += 4;
		str[size] = NULL_CHAR;
	}
	else
	{
		if (capacity < size + 5 && this->str) { Memory::Free(str); }
		if (!str) { str = (W16*)Memory::Allocate((size + 5) * sizeof(W16), capacity); }
		memcpy(str + size, FALSE_STR, 5 * sizeof(W16));
		size += 5;
		str[size] = NULL_CHAR;
	}
}

template<typename T> inline std::enable_if_t<std::is_floating_point_v<T>> WString::ToWString(W16* str, T value)
{
	hashed = false;
	if (capacity < size + 27 && this->str) { Memory::Free(str); }
	if (!str) { str = (W16*)Memory::Allocate((size + 27) * sizeof(W16), capacity); }

	W16* c = str + 27;
	const W16* threeDigits;
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

	memcpy(str + neg, c, addLength * sizeof(W16));
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>> WString::HexToWString(W16* str, T value)
{
	hashed = false;
	if (capacity < size + 16 && this->str) { Memory::Free(str); }
	if (!str) { str = (W16*)Memory::Allocate((size + 16) * sizeof(W16), capacity); }

	U8 pairs;
	U8 digits;
	U64 max;
	if constexpr (std::is_same_v<std::remove_cv_t<T>, U8>) { pairs = 1; digits = 2; max = U8_MAX; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, U16>) { pairs = 2; digits = 4; max = U16_MAX; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, U32>) { pairs = 4; digits = 8; max = U32_MAX; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, UL32>) { pairs = 4; digits = 8; max = U32_MAX; }
	else { pairs = 8; digits = 16; max = U64_MAX; }

	W16* c = str + digits;
	const W16* twoDigits;

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

	memcpy(str, c, digits * sizeof(W16));
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>> WString::HexToWString(W16* str, T value)
{
	hashed = false;
	if (capacity < size + 16 && this->str) { Memory::Free(str); }
	if (!str) { str = (W16*)Memory::Allocate((size + 16) * sizeof(W16), capacity); }

	U8 pairs;
	U8 digits;
	if constexpr (std::is_same_v<std::remove_cv_t<T>, U8>) { pairs = 1; digits = 2; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, U16>) { pairs = 2; digits = 4; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, U32>) { pairs = 4; digits = 8; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, UL32>) { pairs = 4; digits = 8; }
	else { pairs = 8; digits = 16; }

	W16* c = str + digits;
	const W16* twoDigits;
	U64 val = value;

	for (U8 i = 0; i < pairs; ++i)
	{
		twoDigits = HEX_LOOKUP + (val & 0xFF) * 2;

		*--c = twoDigits[1];
		*--c = twoDigits[0];

		val >>= 8;
	}

	size += digits;

	memcpy(str, c, digits * sizeof(W16));
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_floating_point_v<T>> WString::HexToWString(W16* str, T value)
{
	hashed = false;
	if (capacity < size + 16 && this->str) { Memory::Free(str); }
	if (!str) { str = (W16*)Memory::Allocate((size + 16) * sizeof(W16), capacity); }

	U8 pairs = 8;
	U8 digits = 16;

	W16* c = str + digits;
	const W16* twoDigits;
	U64 val = *reinterpret_cast<U64*>(&value);

	for (U8 i = 0; i < pairs; ++i)
	{
		twoDigits = HEX_LOOKUP + (val & 0xFF) * 2;

		*--c = twoDigits[1];
		*--c = twoDigits[0];

		val >>= 8;
	}

	size += digits;

	memcpy(str, c, digits * sizeof(W16));
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>, T> WString::ToType() const
{
	W16* it = str;
	W16 c;
	I64 value = 0;

	if (*str == NEGATIVE_CHAR)
	{
		++it;
		while ((c = *it++) != NULL_CHAR) { value *= 10; value -= c - ZERO_CHAR; }
	}
	else
	{
		while ((c = *it++) != NULL_CHAR) { value *= 10; value += c - ZERO_CHAR; }
	}

	return value;
}

template<typename T> inline std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>, T> WString::ToType() const
{
	W16* it = str;
	W16 c;
	T value = 0;

	while ((c = *it++) != NULL_CHAR) { value *= 10; value += c - ZERO_CHAR; }

	return value;
}

template<typename T> inline std::enable_if_t<std::is_integral_v<T> && !std::_Is_nonbool_integral<T>, T> WString::ToType() const
{
	return IS_TRUE_W(str);
}

template<typename T> inline std::enable_if_t<std::is_floating_point_v<T>, T> WString::ToType() const
{
	W16* it = str;
	W16 c;
	F64 value = 0.0f;
	F64 mul = 0.1f;

	if (*str == NEGATIVE_CHAR)
	{
		++it;
		while ((c = *it++) != NULL_CHAR && c != DECIMAL_CHAR) { value *= 10; value -= c - ZERO_CHAR; }
		while ((c = *it++) != NULL_CHAR) { value -= (c - ZERO_CHAR) * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != NULL_CHAR && c != DECIMAL_CHAR) { value *= 10; value += c - ZERO_CHAR; }
		while ((c = *it++) != NULL_CHAR) { value += (c - ZERO_CHAR) * mul; mul *= 0.1f; }
	}

	return value;
}