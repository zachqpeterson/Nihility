#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"
#include "Vector.hpp"

//Use memcmp(string, TRUE_STR, 4) instead
#define IS_TRUE(c) c[0] == 't' && c[1] == 'r' && c[2] == 'u' && c[3] == 'e'

#define WHITE_SPACE(c, ptr) (c = *ptr) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f'

#define NOT_WHITE_SPACE(c, ptr) (c = *ptr) != ' ' && c != '\t' && c != '\r' && c != '\n' && c != '\v' && c != '\f'




struct C8Lookup
{
	static inline constexpr const C8* TRUE_STR = "true";
	static inline constexpr const C8* FALSE_STR = "false";
	static inline constexpr const C8 NULL_CHAR = '\0';
	static inline constexpr const C8 NEGATIVE_CHAR = '-';
	static inline constexpr const C8 DECIMAL_CHAR = '.';
	static inline constexpr const C8 ZERO_CHAR = '0';
	static inline constexpr const C8 OPEN_BRACE = '{';
	static inline constexpr const C8 CLOSE_BRACE = '}';
	static inline constexpr const C8 FMT_HEX = 'h';
	static inline constexpr const C8 FMT_DEC = '.';

	static inline constexpr const C8 DECIMAL_LOOKUP[] =
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

	static inline constexpr const C8 HEX_LOOKUP[] =
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
};

struct C16Lookup
{
	static inline constexpr const C16* TRUE_STR = L"true";
	static inline constexpr const C16* FALSE_STR = L"false";
	static inline constexpr const C16 NULL_CHAR = L'\0';
	static inline constexpr const C16 NEGATIVE_CHAR = L'-';
	static inline constexpr const C16 DECIMAL_CHAR = L'.';
	static inline constexpr const C16 ZERO_CHAR = L'0';
	static inline constexpr const C16 OPEN_BRACE = L'{';
	static inline constexpr const C16 CLOSE_BRACE = L'}';
	static inline constexpr const C16 FMT_HEX = L'h';
	static inline constexpr const C16 FMT_DEC = L'.';

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
};

/*
* TODO: Documentation
*
* TODO: Predicates / regex?
*
* TODO: Count of a character
*
* TODO: Conversions from C16 to C8
*
* TODO: More formatting options
*	TODO: {h} will convert to hexadecimal
*	TODO: {.n} will write n decimal places (only for floats)
*
* TODO: format strings without allocating new strings for every arg
*
* TODO: capacity checks should be capacity - 1 to avoid overunning the buffer when placing NULL_CHAR
*/
template<typename T, typename LU>
struct NH_API StringBase
{
	StringBase();
	StringBase(NoInit flag);
	template<typename Arg> StringBase(const Arg& value);
	template<typename Arg> StringBase(const Arg& value, Hex flag);
	StringBase(T* str);
	StringBase(const T* str);
	StringBase(const StringBase& other);
	StringBase(StringBase&& other) noexcept;
	template<typename... Args> StringBase(const T* fmt, const Args& ... args);
	template<typename... Args> StringBase(const Args& ... args);

	template<typename Arg> StringBase& operator=(const Arg& value);
	StringBase& operator=(T* str);
	StringBase& operator=(const T* str);
	StringBase& operator=(const StringBase& other);
	StringBase& operator=(StringBase&& other) noexcept;

	~StringBase();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);
	void Resize();

	template<typename Arg> EnableForSignedInt<Arg, Arg> ToType(U64 start = 0) const;
	template<typename Arg> EnableForUnsignedInt<Arg, Arg> ToType(U64 start = 0) const;
	template<typename Arg> EnableForBool<Arg, Arg> ToType(U64 start = 0) const;
	template<typename Arg> EnableForFloat<Arg, Arg> ToType(U64 start = 0) const;
	template<typename Arg> EnableForPointer<Arg, Arg> ToType(U64 start = 0) const;

	explicit operator T* ();
	explicit operator const T* () const;
	template<typename Arg> explicit operator Arg() const;

	template<typename Arg> StringBase& operator+=(const Arg& value);
	StringBase& operator+=(T* other);
	StringBase& operator+=(const T* other);
	StringBase& operator+=(const StringBase& other);

	T* operator*();
	const T* operator*() const;
	T& operator[](U64 i);
	const T& operator[](U64 i) const;

	bool operator==(T* other) const;
	bool operator==(const T* other) const;
	bool operator==(const StringBase& other) const;
	bool operator!=(T* other) const;
	bool operator!=(const T* other) const;
	bool operator!=(const StringBase& other) const;

	bool Compare(T* other) const;
	bool Compare(const T* other) const;
	bool Compare(const StringBase& other) const;
	bool CompareN(T* other, U64 nLength, U64 start = 0) const;
	bool CompareN(const T* other, U64 nLength, U64 start = 0) const;
	bool CompareN(const StringBase& other, U64 nLength, U64 start = 0) const;

	bool Blank() const;
	I64 IndexOf(const T& c, U64 start = 0) const;
	I64 LastIndexOf(const T& c, U64 start = 0) const;

	StringBase& Trim();
	StringBase& Append(const StringBase& append);
	StringBase& Prepend(const StringBase& prepend);
	StringBase& Surround(const StringBase& prepend, const StringBase& append);
	StringBase& Insert(const StringBase& string, U64 i);
	StringBase& Overwrite(const StringBase& string, U64 i = 0);
	StringBase& ReplaceAll(const StringBase& find, const StringBase& replace, U64 start = 0);
	StringBase& ReplaceN(const StringBase& find, const StringBase& replace, U64 count, U64 start = 0);
	StringBase& ReplaceFirst(const StringBase& find, const StringBase& replace, U64 start = 0);

	void SubString(StringBase& newStr, U64 start, U64 nLength = I64_MAX) const;
	void Appended(StringBase& newStr, const StringBase& append) const;
	void Prepended(StringBase& newStr, const StringBase& prepend) const;
	void Surrounded(StringBase& newStr, const StringBase& prepend, const StringBase& append) const;
	void Split(Vector<StringBase>& list, T delimiter, bool trimEntries) const;

	const U64& Size() const;
	const U64& Capacity() const;
	const U64& Hash();
	T* Data();
	const T* Data() const;

	T* begin();
	T* end();
	const T* begin() const;
	const T* end() const;

	T* rbegin();
	T* rend();
	const T* rbegin() const;
	const T* rend() const;

private:
	template<typename Arg> EnableForSignedInt<Arg> ToString(T* str, const Arg& value);
	template<typename Arg> EnableForUnsignedInt<Arg> ToString(T* str, const Arg& value);
	template<typename Arg> EnableForBool<Arg> ToString(T* str, const Arg& value);
	template<typename Arg> EnableForFloat<Arg> ToString(T* str, const Arg& value);
	template<typename Arg> EnableForPointer<Arg> ToString(T* str, const Arg& value);

	template<typename Arg> EnableForSignedInt<Arg> HexToString(T* str, const Arg& value);
	template<typename Arg> EnableForUnsignedInt<Arg> HexToString(T* str, const Arg& value);
	template<typename Arg> EnableForFloat<Arg> HexToString(T* str, const Arg& value);
	template<typename Arg> EnableForPointer<Arg> HexToString(T* str, const Arg& value);

	template<typename Arg> EnableForSignedInt<Arg, U64> InsertType(T* str, const Arg& value, U64 rmvAmt);
	template<typename Arg> EnableForUnsignedInt<Arg, U64> InsertType(T* str, const Arg& value, U64 rmvAmt);
	template<typename Arg> EnableForBool<Arg, U64> InsertType(T* str, const Arg& value, U64 rmvAmt);
	template<typename Arg> EnableForFloat<Arg, U64> InsertType(T* str, const Arg& value, U64 rmvAmt, U64 decimalCount = 5);
	template<typename Arg> EnableForPointer<Arg, U64> InsertType(T* str, const Arg& value, U64 rmvAmt);

	template<typename Arg> EnableForSignedInt<Arg, U64> InsertHex(T* str, const Arg& value, U64 rmvAmt);
	template<typename Arg> EnableForUnsignedInt<Arg, U64> InsertHex(T* str, const Arg& value, U64 rmvAmt);
	template<typename Arg> EnableForFloat<Arg, U64> InsertHex(T* str, const Arg& value, U64 rmvAmt);
	template<typename Arg> EnableForPointer<Arg, U64> InsertHex(T* str, const Arg& value, U64 rmvAmt);

	//TODO: Don't allocate string for every arg
	void Format(U64& start, const StringBase& replace);

	U64 Length(T* str);
	void Copy(T* dst, T* src, U64 length);
	bool Compare(T* a, T* b, U64 length);

	bool hashed{ false };
	U64 hash{ 0 };
	U64 size{ 0 };
	U64 capacity{ 0 };
	T* string{ nullptr };
};

using String = StringBase<C8, C8Lookup>;
using WString = StringBase<C16, C16Lookup>;

template<typename T, typename LU>
inline StringBase<T, LU>::StringBase() { Memory::AllocateArray(&string, capacity); }

template<typename T, typename LU>
inline StringBase<T, LU>::StringBase(NoInit flag) {}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>::StringBase(const Arg& value) { ToString(string, value); }

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>::StringBase(const Arg& value, Hex flag) { HexToString(string, value); }

template<typename T, typename LU>
inline StringBase<T, LU>::StringBase(T* str) : size{ Length(str) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);
	Copy(string, str, size + 1);
}

template<typename T, typename LU>
inline StringBase<T, LU>::StringBase(const T* str) : size{ Length(str) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);
	Copy(string, str, size + 1);
}

template<typename T, typename LU>
inline StringBase<T, LU>::StringBase(const StringBase<T, LU>& other) : size{ other.size }, capacity{ other.capacity }
{
	Memory::AllocateArray(&string, capacity);
	Copy(string, other.string, size + 1);
}

template<typename T, typename LU>
inline StringBase<T, LU>::StringBase(StringBase<T, LU>&& other) noexcept : size{ other.size }, capacity{ other.capacity }, string{ other.string }
{
	other.size = 0;
	other.capacity = 0;
	other.string = nullptr;
}

template<typename T, typename LU>
template<typename... Args>
inline StringBase<T, LU>::StringBase(const T* fmt, const Args& ... args) : size{ Length(fmt) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);

	Copy(string, fmt, size + 1);
	U64 start = 0;
	(Format(start, args), ...);
}

template<typename T, typename LU>
template<typename... Args>
inline StringBase<T, LU>::StringBase(const Args& ... args)
{
	Memory::AllocateArray(&string, capacity);
	(Append(args), ...);
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::operator=(const Arg& value)
{
	ToString(string, value);
	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::operator=(T* str)
{
	hashed = false;
	if (!str) { Destroy(); return *this; }

	size = Length(str);

	if (capacity < size || !string) { Memory::Reallocate(&string, capacity = size); }

	Copy(string, str, size + 1);

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::operator=(const T* str)
{
	hashed = false;
	if (!str) { Destroy(); return *this; }

	size = Length(str);

	if (capacity < size || !string) { Memory::Reallocate(&string, capacity = size); }

	Copy(string, str, size + 1);

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::operator=(const StringBase<T, LU>& other)
{
	hashed = false;
	if (!other.string) { Destroy(); return *this; }

	size = other.size;

	if (capacity < other.size) { Memory::Reallocate(&this->string, capacity = other.size); }

	Copy(string, other.string, size + 1);

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::operator=(StringBase<T, LU>&& other) noexcept
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

template<typename T, typename LU>
inline StringBase<T, LU>::~StringBase()
{
	Destroy();
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Destroy()
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

template<typename T, typename LU>
inline void StringBase<T, LU>::Clear()
{
	hashed = false;
	string[0] = LU::NULL_CHAR;
	size = 0;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Reserve(U64 size)
{
	if (size > capacity)
	{
		Memory::Reallocate(&string, capacity = size);
	}
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Resize(U64 size)
{
	if (size > this->capacity) { Reserve(size); }
	this->size = size;
	string[size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Resize()
{
	size = Length(string);
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::operator+=(const Arg& value)
{
	ToString(string + size, value);
	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::operator+=(T* other)
{
	hashed = false;
	U64 addLength = Length(other);
	if (capacity < size + addLength) { Memory::Reallocate(&string, capacity = size + addLength); }
	Copy(string + size, other, addLength);
	size += addLength;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::operator+=(const T* other)
{
	hashed = false;
	U64 addLength = Length(other);
	if (capacity < size + addLength) { Memory::Reallocate(&string, capacity = size + addLength); }
	Copy(string + size, other, addLength);
	size += addLength;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::operator+=(const StringBase<T, LU>& other)
{
	hashed = false;
	if (capacity < size + other.size) { Memory::Reallocate(&string, capacity = size + other.size); }
	Copy(string + size, other.string, other.size);
	size += other.size;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>::operator Arg() const { return ToType<Arg>(); }

template<typename T, typename LU>
inline StringBase<T, LU>::operator T* () { return string; }

template<typename T, typename LU>
inline StringBase<T, LU>::operator const T* () const { return string; }

template<typename T, typename LU>
inline T* StringBase<T, LU>::operator*() { return string; }

template<typename T, typename LU>
inline const T* StringBase<T, LU>::operator*() const { return string; }

template<typename T, typename LU>
inline T& StringBase<T, LU>::operator[](U64 i) { return string[i]; }

template<typename T, typename LU>
inline const T& StringBase<T, LU>::operator[](U64 i) const { return string[i]; }

template<typename T, typename LU>
inline bool StringBase<T, LU>::operator==(T* other) const
{
	U64 len = Length(other);
	if (len != size) { return false; }

	return Compare(string, other, size) == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::operator==(const T* other) const
{
	U64 len = Length(other);
	if (len != size) { return false; }

	return Compare(string, other, size) == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::operator==(const StringBase<T, LU>& other) const
{
	if (other.size != size) { return false; }

	return Compare(string, other.string, size) == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::operator!=(T* other) const
{
	U64 len = Length(other);
	if (len != size) { return true; }

	return Compare(string, other, size);
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::operator!=(const T* other) const
{
	U64 len = Length(other);
	if (len != size) { return true; }

	return Compare(string, other, size);
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::operator!=(const StringBase<T, LU>& other) const
{
	if (other.size != size) { return true; }

	return Compare(string, other.string, size);
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::Compare(T* other) const
{
	U64 len = Length(other);
	if (len != size) { return false; }

	return Compare(string, other, size) == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::Compare(const T* other) const
{
	U64 len = Length(other);
	if (len != size) { return false; }

	return Compare(string, other, size) == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::Compare(const StringBase<T, LU>& other) const
{
	if (other.size != size) { return false; }

	return Compare(string, other.string, size) == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::CompareN(T* other, U64 nLength, U64 start) const
{
	U64 len = Length(other);
	if (len != nLength) { return false; }

	return Compare(string + start, other, nLength) == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::CompareN(const T* other, U64 nLength, U64 start) const
{
	U64 len = Length(other);
	if (len != nLength) { return false; }

	return Compare(string + start, other, nLength) == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::CompareN(const StringBase<T, LU>& other, U64 nLength, U64 start) const
{
	if (other.size != nLength) { return false; }

	return Compare(string + start, other.string, nLength) == 0;
}

template<typename T, typename LU>
inline const U64& StringBase<T, LU>::Size() const { return size; }

template<typename T, typename LU>
inline const U64& StringBase<T, LU>::Capacity() const { return capacity; }

template<typename T, typename LU>
inline const U64& StringBase<T, LU>::Hash()
{
	if (hashed) { return hash; }

	hash = 0;
	const T* c = string;
	while (*c) { hash = hash * 101 + *c++; }
	hashed = true;

	return hash;
}

template<typename T, typename LU>
inline T* StringBase<T, LU>::Data() { return string; }

template<typename T, typename LU>
inline const T* StringBase<T, LU>::Data() const { return string; }

template<typename T, typename LU>
inline bool StringBase<T, LU>::Blank() const
{
	if (size == 0) { return true; }
	T* start = string;
	T c;

	//TODO:
	while (WHITE_SPACE(c, start));

	return start - string == size;
}

template<typename T, typename LU>
inline I64 StringBase<T, LU>::IndexOf(const T& ch, U64 start) const
{
	T* it = string + start;
	T c;

	while ((c = *it) != ch && c != LU::NULL_CHAR) { ++it; }

	if (*it == LU::NULL_CHAR) { return -1; }
	return (I64)(it - string);
}

template<typename T, typename LU>
inline I64 StringBase<T, LU>::LastIndexOf(const T& c, U64 start) const
{
	T* it = string + size - start - 1;

	U64 len = size;
	while (*it != c && len > 0) { --it; --len; }

	if (len) { return (I64)(it - string); }
	return -1;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::Trim()
{
	hashed = false;
	T* start = string;
	T* end = string + size;
	T c;

	//TODO:
	while (WHITE_SPACE(c, start)) { ++start; }
	while (WHITE_SPACE(c, end)) { --end; }

	size = end - start;
	Copy(string, start, size);
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::Append(const StringBase<T, LU>& append)
{
	if (capacity < size + append.size) { Memory::Reallocate(&string, capacity = size + append.size); }
	hashed = false;
	Copy(string + size, append.string, append.size);
	size += append.size;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::Prepend(const StringBase<T, LU>& prepend)
{
	if (capacity < size + prepend.size) { Memory::Reallocate(&string, capacity = size + prepend.size); }
	hashed = false;
	Copy(string + size, string, size);
	Copy(string, prepend.string, prepend.size);
	size += prepend.size;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::Surround(const StringBase<T, LU>& prepend, const StringBase<T, LU>& append)
{
	if (capacity < size + append.size + prepend.size) { Memory::Reallocate(&string, capacity = size + append.size + prepend.size); }
	hashed = false;
	Copy(string + prepend.size, string, size);
	Copy(string, prepend.string, prepend.size);
	size += prepend.size;

	Copy(string + size, append.string, append.size);
	size += append.size;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::Insert(const StringBase<T, LU>& other, U64 i)
{
	if (capacity < size + other.size) { Memory::Reallocate(&string, capacity = size + other.size); }
	hashed = false;
	Copy(string + i + other.size, string + i, size - i);
	Copy(string + i, other.string, other.size);
	size += other.size;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::Overwrite(const StringBase<T, LU>& other, U64 i)
{
	T* c = string + i;
	Copy(c, other.string, other.size + 1);

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::ReplaceAll(const StringBase<T, LU>& find, const StringBase<T, LU>& replace, U64 start)
{
	hashed = false;
	T* c = string + start;
	T ch = *c;
	while (ch != LU::NULL_CHAR)
	{
		while ((ch = *c) != LU::NULL_CHAR && Compare(c, find.string, find.size)) { ++c; }

		if (ch != LU::NULL_CHAR)
		{
			if (capacity < size - find.size + replace.size) { Memory::Reallocate(&string, capacity = size - find.size + replace.size); }

			Copy(c + replace.size, c + find.size, size - find.size - (c - string));
			Copy(c, replace.string, replace.size);
			size = size - find.size + replace.size;
			string[size] = LU::NULL_CHAR;
		}
	}

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::ReplaceN(const StringBase<T, LU>& find, const StringBase<T, LU>& replace, U64 count, U64 start)
{
	if (capacity < size + replace.size * count - find.size * count) { Memory::Reallocate(&string, capacity = size + replace.size * count - find.size * count); }

	hashed = false;
	T* c = string + start;
	T ch = *c;
	while (ch != LU::NULL_CHAR && count)
	{
		while ((ch = *c) != LU::NULL_CHAR && Compare(c, find.string, find.size)) { ++c; }

		if (ch != LU::NULL_CHAR)
		{
			--count;
			Copy(c + replace.size, c + find.size, size - find.size - (c - string));
			Copy(c, replace.string, replace.size);
			size = size - find.size + replace.size;
			string[size] = LU::NULL_CHAR;
		}
	}

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::ReplaceFirst(const StringBase<T, LU>& find, const StringBase<T, LU>& replace, U64 start)
{
	if (capacity < size + replace.size - find.size) { Memory::Reallocate(&string, capacity = size + replace.size - find.size); }

	hashed = false;
	T* it = string + start;
	T c;

	while ((c = *it) != LU::NULL_CHAR && Compare(it, find.string, find.size)) { ++c; }

	if (c != LU::NULL_CHAR)
	{
		Copy(it + replace.size, it + find.size, size - find.size - (it - string));
		Copy(it, replace.string, replace.size);
		size = size - find.size + replace.size;
		string[size] = LU::NULL_CHAR;
	}

	return *this;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::SubString(StringBase<T, LU>& newStr, U64 start, U64 nLength) const
{
	if (nLength < U64_MAX) { newStr.size = nLength; }
	else { newStr.size = size - start; }

	Copy(newStr.string, string + start, newStr.size);
	newStr.string[newStr.size] = LU::NULL_CHAR;

	return newStr;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Appended(StringBase& newStr, const StringBase& append) const
{
	newStr.hashed = false;
	newStr.Resize(size + append.size);
	Copy(newStr.string, string, size);
	Copy(newStr.string + size, append.string, append.size);
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Prepended(StringBase& newStr, const StringBase& prepend) const
{
	newStr.hashed = false;
	newStr.Resize(size + prepend.size);
	Copy(newStr.string, prepend.string, prepend.size);
	Copy(newStr.string + prepend.size, string, size);
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Surrounded(StringBase& newStr, const StringBase& prepend, const StringBase& append) const
{
	newStr.hashed = false;
	newStr.Resize(size + append.size + prepend.size);
	Copy(newStr.string, prepend.string, prepend.size);
	Copy(newStr.string + prepend.size, string, size);
	Copy(newStr.string + prepend.size + size, append.string, append.size);
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Split(Vector<StringBase<T, LU>>& list, T delimiter, bool trimEntries) const
{
	//TODO
}

template<typename T, typename LU>
inline T* StringBase<T, LU>::begin() { return string; }

template<typename T, typename LU>
inline T* StringBase<T, LU>::end() { return string + size; }

template<typename T, typename LU>
inline const T* StringBase<T, LU>::begin() const { return string; }

template<typename T, typename LU>
inline const T* StringBase<T, LU>::end() const { return string + size; }

template<typename T, typename LU>
inline T* StringBase<T, LU>::rbegin() { return string + size - 1; }

template<typename T, typename LU>
inline T* StringBase<T, LU>::rend() { return string - 1; }

template<typename T, typename LU>
inline const T* StringBase<T, LU>::rbegin() const { return string + size - 1; }

template<typename T, typename LU>
inline const T* StringBase<T, LU>::rend() const { return string - 1; }

template<typename T, typename LU>
template<typename Arg>
EnableForSignedInt<Arg>
inline StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 20) { Memory::Reallocate(&string, capacity = size + 20); }

	T* c = str + 20;
	const T* threeDigits;
	U8 neg = 0;

	U64 abs = (U64)value;

	if (value < 0)
	{
		str[0] = LU::NEGATIVE_CHAR;
		abs = (U64)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U64 newVal = abs / 1000;
		U64 remainder = abs % 1000;
		threeDigits = LU::DECIMAL_LOOKUP + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = LU::DECIMAL_LOOKUP + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	U64 addLength = 20 + neg - (c - str);
	size += addLength;

	Copy(str + neg, c, addLength);
	str[size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
template<typename Arg>
EnableForUnsignedInt<Arg>
inline StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 20) { Memory::Reallocate(&string, capacity = size + 20); }

	T* c = str + 20;
	const T* threeDigits;
	U64 val = value;

	while (val > 999)
	{
		U64 newVal = val / 1000;
		U64 remainder = val % 1000;
		threeDigits = LU::DECIMAL_LOOKUP + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		val = newVal;
	}

	threeDigits = LU::DECIMAL_LOOKUP + (val * 3);
	*--c = threeDigits[2];
	if (val > 9) { *--c = threeDigits[1]; }
	if (val > 99) { *--c = threeDigits[0]; }

	U64 addLength = 20 - (c - str);
	size += addLength;

	Copy(str, c, addLength);
	str[size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
template<typename Arg>
EnableForBool<Arg>
inline StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (value)
	{
		if (!string || capacity < size + 4) { Memory::Reallocate(&string, capacity = size + 4); }
		Copy(str + size, LU::TRUE_STR, 4);
		size += 4;
		str[size] = LU::NULL_CHAR;
	}
	else
	{
		if (!string || capacity < size + 5) { Memory::Reallocate(&string, capacity = size + 5); }
		Copy(str + size, LU::FALSE_STR, 5);
		size += 5;
		str[size] = LU::NULL_CHAR;
	}
}

template<typename T, typename LU>
template<typename Arg>
EnableForFloat<Arg>
inline StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 27) { Memory::Reallocate(&string, capacity = size + 27); }

	T* c = str + 27;
	const T* threeDigits;
	U8 neg = 0;

	F64 abs = value;

	if (value < 0)
	{
		str[0] = LU::NEGATIVE_CHAR;
		abs = -value;
		neg = 1;
	}

	U64 dec = (U64)((abs - (F64)(U64)abs) * 100000.0f);

	U64 newVal = dec / 1000;
	U64 remainder = dec % 1000;
	threeDigits = LU::DECIMAL_LOOKUP + (remainder * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = threeDigits[0];
	dec = newVal;

	threeDigits = LU::DECIMAL_LOOKUP + (dec * 3);
	*--c = threeDigits[2];
	*--c = threeDigits[1];
	*--c = LU::DECIMAL_CHAR;

	U64 whole = (U64)abs;

	while (whole > 999)
	{
		U64 newVal = whole / 1000;
		U64 remainder = whole % 1000;
		threeDigits = LU::DECIMAL_LOOKUP + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		whole = newVal;
	}

	threeDigits = LU::DECIMAL_LOOKUP + (whole * 3);
	*--c = threeDigits[2];
	if (whole > 9) { *--c = threeDigits[1]; }
	if (whole > 99) { *--c = threeDigits[0]; }

	U64 addLength = 27 + neg - (c - str);
	size += addLength;

	Copy(str + neg, c, addLength);
	str[size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
template<typename Arg>
EnableForPointer<Arg>
inline StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	ToString(str, (U64)value);
}

template<typename T, typename LU>
template<typename Arg>
EnableForSignedInt<Arg>
inline StringBase<T, LU>::HexToString(T* str, const Arg& value)
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

	T* c = str + digits;
	const T* twoDigits;

	U64 abs;
	if (value < 0)
	{
		abs = max - ((U64)-value - 1);

		for (U8 i = 0; i < pairs; ++i)
		{
			U64 j = abs & 0xFF;
			twoDigits = LU::HEX_LOOKUP + (abs & 0xFF) * 2;

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
			twoDigits = LU::HEX_LOOKUP + (abs & 0xFF) * 2;

			*--c = twoDigits[1];
			*--c = twoDigits[0];

			abs >>= 8;
		}
	}

	size += digits;

	Copy(str, c, digits);
	str[size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
template<typename Arg>
EnableForUnsignedInt<Arg>
inline StringBase<T, LU>::HexToString(T* str, const Arg& value)
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

	T* c = str + digits;
	const T* twoDigits;
	U64 val = value;

	for (U8 i = 0; i < pairs; ++i)
	{
		twoDigits = LU::HEX_LOOKUP + (val & 0xFF) * 2;

		*--c = twoDigits[1];
		*--c = twoDigits[0];

		val >>= 8;
	}

	size += digits;

	Copy(str, c, digits);
	str[size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
template<typename Arg>
EnableForFloat<Arg>
inline StringBase<T, LU>::HexToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 16) { Memory::Reallocate(&string, capacity = size + 16); }

	U8 pairs = 8;
	U8 digits = 16;

	T* c = str + digits;
	const T* twoDigits;
	U64 val = reinterpret_cast<U64>(value);

	for (U8 i = 0; i < pairs; ++i)
	{
		twoDigits = LU::HEX_LOOKUP + (val & 0xFF) * 2;

		*--c = twoDigits[1];
		*--c = twoDigits[0];

		val >>= 8;
	}

	size += digits;

	Copy(str, c, digits);
	str[size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
template<typename Arg>
EnableForSignedInt<Arg, Arg>
inline StringBase<T, LU>::ToType(U64 start) const
{
	T* it = string + start;
	T c;
	I64 value = 0;

	if (*string == LU::NEGATIVE_CHAR)
	{
		++it;
		while (NOT_WHITE_SPACE(c, it) && c != LU::NULL_CHAR) { value *= 10; value -= c - LU::ZERO_CHAR; ++it; }
	}
	else
	{
		while (NOT_WHITE_SPACE(c, it)) { value *= 10; value += c - LU::ZERO_CHAR; ++it; }
	}

	return value;
}

template<typename T, typename LU>
template<typename Arg>
EnableForUnsignedInt<Arg, Arg>
inline StringBase<T, LU>::ToType(U64 start) const
{
	C8* it = string + start;
	C8 c;
	T value = 0;

	while (NOT_WHITE_SPACE(c, it) && c != LU::NULL_CHAR) { value *= 10; value += c - LU::ZERO_CHAR; ++it; }

	return value;
}

template<typename T, typename LU>
template<typename Arg>
EnableForBool<Arg, Arg>
inline StringBase<T, LU>::ToType(U64 start) const
{
	T* c = string + start;
	return IS_TRUE(c);
}

template<typename T, typename LU>
template<typename Arg>
EnableForFloat<Arg, Arg>
inline StringBase<T, LU>::ToType(U64 start) const
{
	T* it = string + start;
	T c;
	F64 value = 0.0f;
	F64 mul = 0.1f;

	if (*string == LU::NEGATIVE_CHAR)
	{
		++it;
		while (NOT_WHITE_SPACE(c, it) && c != LU::NULL_CHAR && c != LU::DECIMAL_CHAR) { value *= 10; value -= c - LU::ZERO_CHAR; ++it; }
		while (NOT_WHITE_SPACE(c, it) && c != LU::NULL_CHAR) { value -= (c - LU::ZERO_CHAR) * mul; mul *= 0.1f; ++it; }
	}
	else
	{
		while (NOT_WHITE_SPACE(c, it) && c != LU::NULL_CHAR && c != LU::DECIMAL_CHAR) { value *= 10; value += c - LU::ZERO_CHAR; ++it; }
		while (NOT_WHITE_SPACE(c, it) && c != LU::NULL_CHAR) { value += (c - LU::ZERO_CHAR) * mul; mul *= 0.1f; ++it; }
	}

	return value;
}

template<typename T, typename LU>
template<typename Arg>
EnableForPointer<Arg, Arg>
inline StringBase<T, LU>::ToType(U64 start) const
{
	return (Arg)ToType<U64>(start);
}

template<typename T, typename LU>
inline U64 StringBase<T, LU>::Length(T* str)
{
	U64 length = 0;
	T* c = str;

	while (*c++ != LU::NULL_CHAR) { ++length; }

	return length;
}

template<>
inline U64 StringBase<C8, C8Lookup>::Length(C8* str)
{
	return strlen(str);
}

template<>
inline U64 StringBase<C16, C16Lookup>::Length(C16* str)
{
	return wcslen(str);
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Copy(T* dst, T* src, U64 length)
{
	memcpy(dst, src, length * sizeof(T));
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::Compare(T* a, T* b, U64 length)
{
	T* c0 = a;
	T* c1 = b;

	T c;

	while ((c = *c0++) == *c1++ && --length) { if (c == LU::NULL_CHAR) { return true; } }

	return false;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Format(U64& start, const StringBase<T, LU>& replace)
{
	hashed = false;
	T* it = string + start;
	T c;

	while ((c = *it++) != LU::NULL_CHAR && c != LU::OPEN_BRACE);

	if (c != LU::NULL_CHAR)
	{
		//TODO: check for CLOSE_BRACE
		c = *it++;
		switch (c)
		{
		case LU::NULL_CHAR: { start = it - string; }
		case LU::CLOSE_BRACE: { start = InsertType(it - 2, value, 2); }
		case LU::FMT_HEX: { start = InsertHex(it - 2, value, 3); }
		case LU::FMT_DEC: { start = InsertType(it - 3, value, 4, *it++); } //TODO: More checks, check if float (constexpr IsSame), check if single digit n
		}
	}


	while (*c != LU::NULL_CHAR && Compare(c, "{}", 2)) { ++c; }

	if (*c != LU::NULL_CHAR)
	{
		start = (c - string) + replace.size;
		Copy(c + replace.size, c + 2, size - 2 - (c - string));
		Copy(c, replace.string, replace.size);
		size = size - 2 + replace.size;
		string[size] = LU::NULL_CHAR;
	}
}

template<typename T, typename LU>
template<typename Arg>
EnableForSignedInt<Arg, U64> inline
StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<typename Arg>
EnableForUnsignedInt<Arg, U64> inline
StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<typename Arg>
EnableForBool<Arg, U64> inline
StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<typename Arg>
EnableForFloat<Arg, U64> inline
StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt, U64 decimalCount)
{
	//TODO
}

template<typename T, typename LU>
template<typename Arg>
EnableForPointer<Arg, U64> inline
StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<typename Arg>
EnableForSignedInt<Arg, U64> inline
StringBase<T, LU>::InsertHex(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<typename Arg>
EnableForUnsignedInt<Arg, U64> inline
StringBase<T, LU>::InsertHex(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<typename Arg>
EnableForFloat<Arg, U64> inline
StringBase<T, LU>::InsertHex(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<typename Arg>
EnableForPointer<Arg, U64> inline
StringBase<T, LU>::InsertHex(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}
