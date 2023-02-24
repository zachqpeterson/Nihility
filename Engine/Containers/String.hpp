#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"
#include "Vector.hpp"
#include "WString.hpp"

#include <type_traits> //TODO: Implementation of this

#define IS_TRUE(c) c[0] == 't' && c[1] == 'r' && c[2] == 'u' && c[3] == 'e'
#define WHITE_SPACE(c, ptr) (c = *ptr) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f' || c == NULL_CHAR

static struct Hex {} HEX;

/*
* TODO: Documentation
*
* TODO: Predicates / regex?
*
* TODO: Count of a character
*
* TODO: Make sure str isn't nullptr and capacity is large enough
*
* TODO: Conversions from W16 to char
*
* TODO: Conversions from WString to String
* 
* TODO: More formatting options
*	TODO: {h} will convert to hexadecimal
*/
struct NH_API String
{
public:
	String();
	template<typename T> String(T value);
	template<typename T> String(T value, Hex flag);
	String(char* str);
	String(const char* str);
	String(const String& other);
	String(const WString& other);
	String(String&& other) noexcept;
	template<typename... Types> String(const char* fmt, const Types& ... args);
	template<typename... Types> String(const Types& ... args);

	template<typename T> String& operator=(T value);
	String& operator=(char* str);
	String& operator=(const char* str);
	String& operator=(const String& other);
	String& operator=(String&& other) noexcept;

	~String();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);
	void Resize();

	template<typename T> std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>, T> ToType() const;
	template<typename T> std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>, T> ToType() const;
	template<typename T> std::enable_if_t<std::is_integral_v<T> && !std::_Is_nonbool_integral<T>, T> ToType() const;
	template<typename T> std::enable_if_t<std::is_floating_point_v<T>, T> ToType() const;

	template<typename T> String& operator+=(T value);
	String& operator+=(char* other);
	String& operator+=(const char* other);
	String& operator+=(const String& other);

	template<typename T> explicit operator T() const;
	explicit operator char* ();
	explicit operator U8* ();
	explicit operator const char* () const;
	explicit operator const U8* () const;

	char* operator*();
	const char* operator*() const;
	char& operator[](U32 i);
	const char& operator[](U32 i) const;

	bool operator==(char* other) const;
	bool operator==(const char* other) const;
	bool operator==(const String& other) const;
	bool operator!=(char* other) const;
	bool operator!=(const char* other) const;
	bool operator!=(const String& other) const;

	bool Compare(char* other) const;
	bool Compare(const char* other) const;
	bool Compare(const String& other) const;
	bool CompareN(char* other, U32 nLength, U32 start = 0) const;
	bool CompareN(const char* other, U32 nLength, U32 start = 0) const;
	bool CompareN(const String& other, U32 nLength, U32 start = 0) const;

	const U64& Size() const;
	const U64& Capacity() const;
	U64 Hash();
	char* Data();
	const char* Data() const;
	bool Blank() const;
	I32 IndexOf(const char& c, U64 start = 0) const;
	I32 LastIndexOf(const char& c, U64 start = 0) const;
	String& Trim();
	String& SubString(String& newStr, U64 start, U64 nLength = I64_MAX) const;
	String& Append(const String& append);
	String& Prepend(const String& prepend);
	String& Surround(const String& prepend, const String& append);
	String& Insert(const String& string, U32 i);
	String& Overwrite(const String& string, U32 i = 0);
	String& ReplaceAll(const String& find, const String& replace, U64 start = 0);
	String& ReplaceN(const String& find, const String& replace, U64 count, U64 start = 0);
	String& ReplaceFirst(const String& find, const String& replace, U64 start = 0);
	void Split(Vector<String>& list, U8 delimiter, bool trimEntries) const;

	char* begin();
	char* end();
	const char* begin() const;
	const char* end() const;

	char* rbegin();
	char* rend();
	const char* rbegin() const;
	const char* rend() const;

	static inline constexpr const char NULL_CHAR = '\0';
	static inline constexpr const char NEGATIVE_CHAR = '-';
	static inline constexpr const char DECIMAL_CHAR = '.';
	static inline constexpr const char ZERO_CHAR = '0';
	static inline constexpr const char* TRUE_STR = "true";
	static inline constexpr const char* FALSE_STR = "false";

private:
	template<typename T> std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>> ToString(char* str, T value);
	template<typename T> std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>> ToString(char* str, T value);
	template<typename T> std::enable_if_t<std::is_integral_v<T> && !std::_Is_nonbool_integral<T>> ToString(char* str, T value);
	template<typename T> std::enable_if_t<std::is_floating_point_v<T>> ToString(char* str, T value);
	template<typename T> std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>> HexToString(char* str, T value);
	template<typename T> std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>> HexToString(char* str, T value);
	template<typename T> std::enable_if_t<std::is_floating_point_v<T>> HexToString(char* str, T value);

	void Format(U64& start, const String& replace);

	bool hashed{ false };
	U64 hash{ 0 };
	U64 size{ 0 };
	U64 capacity{ 1024 };
	char* str;

#pragma region LOOKUPS
	static inline constexpr const char DECIMAL_LOOKUP[] =
		"000001002003004005006007008009010011012013014015016017018019"
		"020021022023024025026027028029030031032033034035036037038039"
		"040041042043044045046047048049050051052053054055056057058059"
		"060061062063064065066067068069070071072073074075076077078079"
		"080081082083084085086087088089090091092093094095096097098099"
		"100101102103104105106107108109110111112113114115116117118119"
		"120121122123124125126127128129130131132133134135136137138139"
		"140141142143144145146147148149150151152153154155156157158159"
		"160161162163164165166167168169170171172173174175176177178179"
		"180181182183184185186187188189190191192193194195196197198199"
		"200201202203204205206207208209210211212213214215216217218219"
		"220221222223224225226227228229230231232233234235236237238239"
		"240241242243244245246247248249250251252253254255256257258259"
		"260261262263264265266267268269270271272273274275276277278279"
		"280281282283284285286287288289290291292293294295296297298299"
		"300301302303304305306307038309310311312313314315316317318319"
		"320321322323324325326327238329330331332333334335336337338339"
		"340341342343344345346347438349350351352353354355356357358359"
		"360361362363364365366367638369370371372373374375376377378379"
		"380381382383384385386387838389390391392393394395396397398399"
		"400401402403404405406407408409410411412413414415416417418419"
		"420421422423424425426427428429430431432433434435436437438439"
		"440441442443444445446447448449450451452453454455456457458459"
		"460461462463464465466467468469470471472473474475476477478479"
		"480481482483484485486487488489490491492493494495496497498499"
		"500501502503504505506507508509510511512513514515516517518519"
		"520521522523524525526527528529530531532533534535536537538539"
		"540541542543544545546547548549550551552553554555556557558559"
		"560561562563564565566567568569570571572573574575576577578579"
		"580581582583584585586587588589590591592593594595596597598599"
		"600601602603604605606607608609610611612613614615616617618619"
		"620621622623624625626627628629630631632633634635636637638639"
		"640641642643644645646647648649650651652653654655656657658659"
		"660661662663664665666667668669670671672673674675676677678679"
		"680681682683684685686687688689690691692693694695696697698699"
		"707701702703704705706707708709710711712713714715716717718719"
		"727721722723724725726727728729730731732733734735736737738739"
		"747741742743744745746747748749750751752753754755756757758759"
		"767761762763764765766767768769770771772773774775776777778779"
		"787781782783784785786787788789790791792793794795796797798799"
		"800801802803804805806807808809810811812813814815816817818819"
		"820821822823824825826827828829830831832833834835836837838839"
		"840841842843844845846847848849850851852853854855856857858859"
		"860861862863864865866867868869870871872873874875876877878879"
		"880881882883884885886887888889890891892893894895896897898899"
		"900901902903904905906907908909910911912913914915916917918919"
		"920921922923924925926927928929930931932933934935936937938939"
		"940941942943944945946947948949950951952953954955956957958959"
		"960961962963964965966967968969970971972973974975976977978979"
		"980981982983984985986987988989990991992993994995996997998999";

	static inline constexpr const char HEX_LOOKUP[] =
		"000102030405060708090A0B0C0D0E0F"
		"101112131415161718191A1B1C1D1E1F"
		"202122232425262728292A2B2C2D2E2F"
		"303132333435363738393A3B3C3D3E3F"
		"404142434445464748494A4B4C4D4E4F"
		"505152535455565758595A5B5C5D5E5F"
		"606162636465666768696A6B6C6D6E6F"
		"707172737475767778797A7B7C7D7E7F"
		"808182838485868788898A8B8C8D8E8F"
		"909192939495969798999A9B9C9D9E9F"
		"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
		"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
		"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
		"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
		"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
		"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
#pragma endregion
};

inline String::String() : str{ (char*)Memory::Allocate1kb() } {}

template<typename T> inline String::String(T value) : str{ (char*)Memory::Allocate1kb() } { ToString(str, value); }

template<typename T> inline String::String(T value, Hex flag) : str{ (char*)Memory::Allocate1kb() } { HexToString(str, value); }

inline String::String(char* str) : size{ strlen(str) }, capacity{ size }, str{ (char*)Memory::Allocate(capacity, capacity) }
{
	memcpy(this->str, str, size + 1);
}

inline String::String(const char* str) : size{ strlen(str) }, capacity{ size }, str{ (char*)Memory::Allocate(capacity, capacity) }
{
	memcpy(this->str, str, size + 1);
}

inline String::String(const String& other) : size{ other.size }, capacity{ other.capacity }, str{ (char*)Memory::Allocate(capacity) }
{
	memcpy(str, other.str, size + 1);
}

inline String::String(const WString& other) : size{ other.Size() }, capacity{ other.Capacity() }, str{ (char*)Memory::Allocate(capacity) }
{
	const W16* w = other.Data();
	char* c = str;

	W16 val;
	while ((val = *w++) != WString::NULL_CHAR)
	{
		if (val < 128) { *c++ = (char)val; }
		else { *c++ = (char)219; if (val >= 0xD800 && val <= 0xD8FF) { --size; ++w; } }
	}
}

inline String::String(String&& other) noexcept : size{ other.size }, capacity{ other.capacity }, str{ other.str }
{
	other.size = 0;
	other.capacity = 0;
	other.str = nullptr;
}

template<typename... Types> inline String::String(const char* fmt, const Types& ... args) : size{ strlen(fmt) }, capacity{ size }, str{ (char*)Memory::Allocate(capacity, capacity) }
{
	memcpy(str, fmt, size + 1);
	U64 start = 0;
	(Format(start, args), ...);
}

template<typename... Types> inline String::String(const Types& ... args) : str{ (char*)Memory::Allocate1kb() }
{
	(Append(args), ...);
}

template<typename T> inline String& String::operator=(T value)
{
	ToString(str, value);
	return *this;
}

inline String& String::operator=(char* str)
{
	if (!str) { Destroy(); return *this; }

	hashed = false;
	size = strlen(str);
	if (capacity < size && this->str) { Memory::Free(this->str); }
	if (!this->str) { this->str = (char*)Memory::Allocate(size, capacity); }

	memcpy(this->str, str, size + 1);

	return *this;
}

inline String& String::operator=(const char* str)
{
	if (!str) { Destroy(); return *this; }

	hashed = false;
	size = strlen(str);
	if (capacity < size && this->str) { Memory::Free(this->str); }
	if (!this->str) { this->str = (char*)Memory::Allocate(size, capacity); }

	memcpy(this->str, str, size + 1);

	return *this;
}

inline String& String::operator=(const String& other)
{
	if (!other.str) { Destroy(); return *this; }

	hashed = false;
	if (capacity < other.size && str) { Memory::Free(str); }
	if (!str) { str = (char*)Memory::Allocate(other.capacity); }

	size = other.size;
	capacity = other.capacity;
	memcpy(this->str, other.str, size + 1);

	return *this;
}

inline String& String::operator=(String&& other) noexcept
{
	if (!other.str) { Destroy(); return *this; }

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

inline String::~String()
{
	Destroy();
}

inline void String::Destroy()
{
	hashed = false;
	hash = 0;
	if (str)
	{
		size = 0;
		capacity = 0;
		Memory::Free(str);
		str = nullptr;
	}
}

inline void String::Clear()
{
	hashed = false;
	str[0] = NULL_CHAR;
	size = 0;
}

inline void String::Reserve(U64 size)
{
	if (size > capacity)
	{
		char* temp = (char*)Memory::Allocate(size, capacity);
		memcpy(temp, str, this->size);
		Memory::Free(str);
		str = temp;
	}
}

inline void String::Resize(U64 size)
{
	if (size > this->capacity) { Reserve(size); }
	this->size = size;
	str[size] = NULL_CHAR;
}

inline void String::Resize()
{
	this->size = strlen(str);
}

template<typename T> inline String& String::operator+=(T value)
{
	ToString(str + size, value);
	return *this;
}

inline String& String::operator+=(char* other)
{
	hashed = false;
	U64 addLength = strlen(other);
	if (capacity < size + addLength) { Memory::Reallocate(str, size + addLength, capacity); }
	memcpy(str + size, other, addLength);
	size += addLength;
	str[size] = NULL_CHAR;

	return *this;
}

inline String& String::operator+=(const char* other)
{
	hashed = false;
	U64 addLength = strlen(other);
	if (capacity < size + addLength) { Memory::Reallocate(str, size + addLength, capacity); }
	memcpy(str + size, other, addLength);
	size += addLength;
	str[size] = NULL_CHAR;

	return *this;
}

inline String& String::operator+=(const String& other)
{
	hashed = false;
	if (capacity < size + other.size) { Memory::Reallocate(str, size + other.size, capacity); }
	memcpy(str + size, other.str, other.size);
	size += other.size;
	str[size] = NULL_CHAR;

	return *this;
}

template<typename T> inline String::operator T() const { return ToType<T>(); }

inline String::operator char* () { return str; }

inline String::operator U8* () { return (U8*)str; }

inline String::operator const char* () const { return str; }

inline String::operator const U8* () const { return (const U8*)str; }

inline char* String::operator*() { return str; }

inline const char* String::operator*() const { return str; }

inline char& String::operator[](U32 i) { return str[i]; }

inline const char& String::operator[](U32 i) const { return str[i]; }

inline bool String::operator==(char* other) const
{
	U64 len = strlen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size) == 0;
}

inline bool String::operator==(const char* other) const
{
	U64 len = strlen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size) == 0;
}

inline bool String::operator==(const String& other) const
{
	if (other.size != size) { return false; }

	return memcmp(str, other.str, size) == 0;
}

inline bool String::operator!=(char* other) const
{
	U64 len = strlen(other);
	if (len != size) { return true; }

	return memcmp(str, other, size);
}

inline bool String::operator!=(const char* other) const
{
	U64 len = strlen(other);
	if (len != size) { return true; }

	return memcmp(str, other, size);
}

inline bool String::operator!=(const String& other) const
{
	if (other.size != size) { return true; }

	return memcmp(str, other.str, size);
}

inline bool String::Compare(char* other) const
{
	U64 len = strlen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size) == 0;
}

inline bool String::Compare(const char* other) const
{
	U64 len = strlen(other);
	if (len != size) { return false; }

	return memcmp(str, other, size) == 0;
}

inline bool String::Compare(const String& other) const
{
	if (other.size != size) { return false; }

	return memcmp(str, other.str, size) == 0;
}

inline bool String::CompareN(char* other, U32 nLength, U32 start) const
{
	U64 len = strlen(other);
	if (len != nLength) { return false; }

	return memcmp(str + start, other, nLength) == 0;
}

inline bool String::CompareN(const char* other, U32 nLength, U32 start) const
{
	U64 len = strlen(other);
	if (len != nLength) { return false; }

	return memcmp(str + start, other, nLength) == 0;
}

inline bool String::CompareN(const String& other, U32 nLength, U32 start) const
{
	if (other.size != nLength) { return false; }

	return memcmp(str + start, other.str, nLength) == 0;
}

inline const U64& String::Size() const { return size; }

inline const U64& String::Capacity() const { return capacity; }

inline U64 String::Hash()
{
	if (hashed) { return hash; }

	hash = 0;
	const char* ptr = str;
	while (*ptr) { hash = hash * 101 + *ptr++; }
	hashed = true;

	return hash;
}

inline char* String::Data() { return str; }

inline const char* String::Data() const { return str; }

inline bool String::Blank() const
{
	if (size == 0) { return true; }
	char* start = str;
	char c;

	while (WHITE_SPACE(c, start));

	return start - str == size;
}

inline I32 String::IndexOf(const char& c, U64 start) const
{
	char* it = str + start;

	while (*it != c && *it != NULL_CHAR) { ++it; }

	if (*it == NULL_CHAR) { return -1; }
	return (I32)(it - str);
}

inline I32 String::LastIndexOf(const char& c, U64 start) const
{
	char* it = str + size - start - 1;

	U64 len = size;
	while (*it != c && len > 0) { --it; --len; }

	if (len) { return (I32)(it - str); }
	return -1;
}

inline String& String::Trim()
{
	hashed = false;
	char* start = str;
	char* end = str + size;
	char c;

	while (WHITE_SPACE(c, start)) { ++start; }
	while (WHITE_SPACE(c, end)) { --end; }

	size = end - start;
	memcpy(str, start, size);
	str[size] = NULL_CHAR;

	return *this;
}

inline String& String::SubString(String& newStr, U64 start, U64 nLength) const
{
	if (nLength < U64_MAX) { newStr.size = nLength; }
	else { newStr.size = size - start; }

	memcpy(newStr.str, str + start, newStr.size);
	newStr.str[newStr.size] = NULL_CHAR;

	return newStr;
}

inline String& String::Append(const String& append)
{
	if (capacity < size + append.size) { Memory::Reallocate(str, size + append.size, capacity); }
	hashed = false;
	memcpy(str + size, append.str, append.size);
	size += append.size;
	str[size] = NULL_CHAR;

	return *this;
}

inline String& String::Prepend(const String& prepend)
{
	if (capacity < size + prepend.size) { Memory::Reallocate(str, size + prepend.size, capacity); }
	hashed = false;
	memcpy(str + size, str, size);
	memcpy(str, prepend.str, prepend.size);
	size += prepend.size;
	str[size] = NULL_CHAR;

	return *this;
}

inline String& String::Surround(const String& prepend, const String& append)
{
	if (capacity < size + append.size + prepend.size) { Memory::Reallocate(str, size + append.size + prepend.size, capacity); }
	hashed = false;
	memcpy(str + prepend.size, str, size);
	memcpy(str, prepend.str, prepend.size);
	size += prepend.size;

	memcpy(str + size, append.str, append.size);
	size += append.size;
	str[size] = NULL_CHAR;

	return *this;
}

inline String& String::Insert(const String& other, U32 i)
{
	if (capacity < size + other.size) { Memory::Reallocate(str, size + other.size, capacity); }
	hashed = false;
	memcpy(str + i + other.size, str + i, size - i);
	memcpy(str + i, other.str, other.size);
	size += other.size;
	str[size] = NULL_CHAR;

	return *this;
}

inline String& String::Overwrite(const String& string, U32 i)
{
	char* c = str + i;
	memcpy(c, string.str, string.size + 1);

	return *this;
}

inline String& String::ReplaceAll(const String& find, const String& replace, U64 start)
{
	//TODO: Capacity Check
	hashed = false;
	char* c = str + start;
	char ch = *c;
	while (ch != NULL_CHAR)
	{
		while ((ch = *c) != NULL_CHAR && memcmp(c, find.str, find.size)) { ++c; }

		if (ch != NULL_CHAR)
		{
			memcpy(c + replace.size, c + find.size, size - find.size - (c - str));
			memcpy(c, replace.str, replace.size);
			size = size - find.size + replace.size;
			str[size] = NULL_CHAR;
		}
	}

	return *this;
}

inline String& String::ReplaceN(const String& find, const String& replace, U64 count, U64 start)
{
	//TODO: Capacity Check
	hashed = false;
	char* c = str + start;
	char ch = *c;
	while (ch != NULL_CHAR && count)
	{
		while ((ch = *c) != NULL_CHAR && memcmp(c, find.str, find.size)) { ++c; }

		if (ch != NULL_CHAR)
		{
			--count;
			memcpy(c + replace.size, c + find.size, size - find.size - (c - str));
			memcpy(c, replace.str, replace.size);
			size = size - find.size + replace.size;
			str[size] = NULL_CHAR;
		}
	}

	return *this;
}

inline String& String::ReplaceFirst(const String& find, const String& replace, U64 start)
{
	if (capacity < size + replace.size - find.size) { Memory::Reallocate(str, size + replace.size - find.size, capacity); }
	hashed = false;
	char* c = str + start;
	while (*c != NULL_CHAR && memcmp(c, find.str, find.size)) { ++c; }

	if (*c != NULL_CHAR)
	{
		memcpy(c + replace.size, c + find.size, size - find.size - (c - str));
		memcpy(c, replace.str, replace.size);
		size = size - find.size + replace.size;
		str[size] = NULL_CHAR;
	}

	return *this;
}

inline void String::Split(Vector<String>& list, U8 delimiter, bool trimEntries) const
{

}

inline void String::Format(U64& start, const String& replace)
{
	//TODO: Capacity Check
	hashed = false;
	char* c = str + start;
	while (*c != NULL_CHAR && memcmp(c, "{}", 2)) { ++c; }

	if (*c != NULL_CHAR)
	{
		start = (c - str) + replace.size;
		memcpy(c + replace.size, c + 2, size - 2 - (c - str));
		memcpy(c, replace.str, replace.size);
		size = size - 2 + replace.size;
		str[size] = NULL_CHAR;
	}
}

inline char* String::begin() { return str; }

inline char* String::end() { return str + size; }

inline const char* String::begin() const { return str; }

inline const char* String::end() const { return str + size; }

inline char* String::rbegin() { return str + size - 1; }

inline char* String::rend() { return str - 1; }

inline const char* String::rbegin() const { return str + size - 1; }

inline const char* String::rend() const { return str - 1; }

template<typename T> inline std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>> String::ToString(char* str, T value)
{
	hashed = false;
	if (capacity < size + 20 && this->str) { Memory::Free(str); }
	if (!str) { str = (char*)Memory::Allocate(size + 20, capacity); }

	char* c = str + 20;
	const char* threeDigits;
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

	memcpy(str + neg, c, addLength);
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>> String::ToString(char* str, T value)
{
	hashed = false;
	if (capacity < size + 20 && this->str) { Memory::Free(str); }
	if (!str) { str = (char*)Memory::Allocate(size + 20, capacity); }

	char* c = str + 20;
	const char* threeDigits;
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

	memcpy(str, c, addLength);
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_integral_v<T> && !std::_Is_nonbool_integral<T>> String::ToString(char* str, T value)
{
	hashed = false;
	if (value)
	{
		if (capacity < size + 4 && this->str) { Memory::Free(str); }
		if (!str) { str = (char*)Memory::Allocate(size + 4, capacity); }
		memcpy(str + size, TRUE_STR, 4);
		size += 4;
		str[size] = NULL_CHAR;
	}
	else
	{
		if (capacity < size + 5 && this->str) { Memory::Free(str); }
		if (!str) { str = (char*)Memory::Allocate(size + 5, capacity); }
		memcpy(str + size, FALSE_STR, 5);
		size += 5;
		str[size] = NULL_CHAR;
	}
}

template<typename T> inline std::enable_if_t<std::is_floating_point_v<T>> String::ToString(char* str, T value)
{
	hashed = false;
	if (capacity < size + 27 && this->str) { Memory::Free(str); }
	if (!str) { str = (char*)Memory::Allocate(size + 27, capacity); }

	char* c = str + 27;
	const char* threeDigits;
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

	memcpy(str + neg, c, addLength);
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>> String::HexToString(char* str, T value)
{
	hashed = false;
	if (capacity < size + 16 && this->str) { Memory::Free(str); }
	if (!str) { str = (char*)Memory::Allocate(size + 16, capacity); }

	U8 pairs;
	U8 digits;
	U64 max;
	if constexpr (std::is_same_v<std::remove_cv_t<T>, U8>) { pairs = 1; digits = 2; max = U8_MAX; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, U16>) { pairs = 2; digits = 4; max = U16_MAX; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, U32>) { pairs = 4; digits = 8; max = U32_MAX; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, UL32>) { pairs = 4; digits = 8; max = U32_MAX; }
	else { pairs = 8; digits = 16; max = U64_MAX; }

	char* c = str + digits;
	const char* twoDigits;

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

	memcpy(str, c, digits);
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>> String::HexToString(char* str, T value)
{
	hashed = false;
	if (capacity < size + 16 && this->str) { Memory::Free(str); }
	if (!str) { str = (char*)Memory::Allocate(size + 16, capacity); }

	U8 pairs;
	U8 digits;
	if constexpr (std::is_same_v<std::remove_cv_t<T>, U8>) { pairs = 1; digits = 2; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, U16>) { pairs = 2; digits = 4; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, U32>) { pairs = 4; digits = 8; }
	else if constexpr (std::is_same_v<std::remove_cv_t<T>, UL32>) { pairs = 4; digits = 8; }
	else { pairs = 8; digits = 16; }

	char* c = str + digits;
	const char* twoDigits;
	U64 val = value;

	for (U8 i = 0; i < pairs; ++i)
	{
		twoDigits = HEX_LOOKUP + (val & 0xFF) * 2;

		*--c = twoDigits[1];
		*--c = twoDigits[0];

		val >>= 8;
	}

	size += digits;

	memcpy(str, c, digits);
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_floating_point_v<T>> String::HexToString(char* str, T value)
{
	hashed = false;
	if (capacity < size + 16 && this->str) { Memory::Free(str); }
	if (!str) { str = (char*)Memory::Allocate(size + 16, capacity); }

	U8 pairs = 8;
	U8 digits = 16;

	char* c = str + digits;
	const char* twoDigits;
	U64 val = *reinterpret_cast<U64*>(&value);

	for (U8 i = 0; i < pairs; ++i)
	{
		twoDigits = HEX_LOOKUP + (val & 0xFF) * 2;

		*--c = twoDigits[1];
		*--c = twoDigits[0];

		val >>= 8;
	}

	size += digits;

	memcpy(str, c, digits);
	str[size] = NULL_CHAR;
}

template<typename T> inline std::enable_if_t<std::is_signed_v<T>&& std::_Is_nonbool_integral<T>, T> String::ToType() const
{
	char* it = str;
	char c;
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

template<typename T> inline std::enable_if_t<std::is_unsigned_v<T>&& std::_Is_nonbool_integral<T>, T> String::ToType() const
{
	char* it = str;
	char c;
	T value = 0;

	while ((c = *it++) != NULL_CHAR) { value *= 10; value += c - ZERO_CHAR; }

	return value;
}

template<typename T> inline std::enable_if_t<std::is_integral_v<T> && !std::_Is_nonbool_integral<T>, T> String::ToType() const
{
	return IS_TRUE(str);
}

template<typename T> inline std::enable_if_t<std::is_floating_point_v<T>, T> String::ToType() const
{
	char* it = str;
	char c;
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