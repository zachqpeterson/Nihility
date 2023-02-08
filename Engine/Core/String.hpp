#pragma once

#include "Defines.hpp"

#include "Memory\Memory.hpp"

struct String
{
public:
	String();
	String(I8 value);
	String(U8 value);
	String(I16 value);
	String(U16 value);
	String(I32 value);
	String(U32 value);
	String(I64 value);
	String(U64 value);
	String(F32 value);
	String(F64 value);
	String(bool value);
	String(char* str);
	String(const char* str);
	String(const String& other);
	String(String&& other) noexcept;
	template<typename... Types> String(const char* fmt, const Types& ... args);
	template<typename... Types> String(const Types& ... args);

	String& operator=(I8 value);
	String& operator=(U8 value);
	String& operator=(I16 value);
	String& operator=(U16 value);
	String& operator=(I32 value);
	String& operator=(U32 value);
	String& operator=(I64 value);
	String& operator=(U64 value);
	String& operator=(F32 value);
	String& operator=(F64 value);
	String& operator=(bool value);
	String& operator=(char* str);
	String& operator=(const char* str);
	String& operator=(const String& other);
	String& operator=(String&& other) noexcept;

	~String();
	void Destroy();
	void Clear();

	I8  ToI8();
	U8  ToU8();
	I16 ToI16();
	U16 ToU16();
	I32 ToI32();
	U32 ToU32();
	I64 ToI64();
	U64 ToU64();
	F32 ToF32();
	F64 ToF64();
	bool ToBool();

	String& operator+=(I8  value);
	String& operator+=(U8  value);
	String& operator+=(I16 value);
	String& operator+=(U16 value);
	String& operator+=(I32 value);
	String& operator+=(U32 value);
	String& operator+=(I64 value);
	String& operator+=(U64 value);
	String& operator+=(F32 value);
	String& operator+=(F64 value);
	String& operator+=(bool value);
	String& operator+=(char* other);
	String& operator+=(const char* other);
	String& operator+=(const String& other);

	explicit operator char* ();
	explicit operator const char* () const;
	explicit operator I8() const;
	explicit operator U8() const;
	explicit operator I16() const;
	explicit operator U16() const;
	explicit operator I32() const;
	explicit operator U32() const;
	explicit operator I64() const;
	explicit operator U64() const;
	explicit operator F32() const;
	explicit operator F64() const;
	explicit operator bool() const;

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
	bool CompareN(char* other, U32 lenth, U32 start = 0) const;
	bool CompareN(const char* other, U32 lenth, U32 start = 0) const;
	bool CompareN(const String& other, U32 lenth, U32 start = 0) const;

	const U64& Length() const;
	char* Data();
	const char* Data() const;
	bool Blank() const;
	I32 IndexOf(char c, U64 start = 0) const;
	String& Trim();
	String& SubString(String& newStr, U64 start, U64 length = I64_MAX) const;
	String& Append(const String& append);
	String& Prepend(const String& prepend);
	String& Surround(const String& prepend, const String& append);
	String& Insert(const String& string, U32 i);
	String& ReplaceAll(const String& find, const String& replace, U64 start = 0);
	String& ReplaceN(const String& find, const String& replace, U64 count, U64 start = 0);
	String& ReplaceFirst(const String& find, const String& replace, U64 start = 0);
	//Vector<String> Split(U8 delimiter, bool trimEntries) const;

	char* begin();
	char* end();
	const char* begin() const;
	const char* end() const;

	char* rbegin();
	char* rend();
	const char* rbegin() const;
	const char* rend() const;

private:
	void Format(U64& start, const String& replace);

	U64 length;
	char* str;

#pragma region THREE_DIGIT_NUMBERS
	static inline constexpr const char* THREE_DIGIT_NUMBERS =
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
#pragma endregion
};

inline String::String() : length{ 0 }, str{ (char*)Memory::Allocate1kb() } {}

inline String::String(I8 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	if (value < 0)
	{
		str[0] = '-';
		U8 abs = (U8)-value;
		char* c = str + 4;
		const char* threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
		*--c = threeDigits[2];
		if (abs > 9) { *--c = threeDigits[1]; }
		if (abs > 99) { *--c = threeDigits[0]; }

		length = 5 - (c - str);
		memcpy(str + 1, c, length + 1);
	}
	else
	{
		char* c = str + 3;
		const char* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
		*--c = threeDigits[2];
		if (value > 9) { *--c = threeDigits[1]; }
		if (value > 99) { *--c = threeDigits[0]; }

		length = 3 - (c - str);
		memcpy(str, c, length + 1);
	}
}

inline String::String(U8 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	char* c = str + 3;
	const char* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	length = 3 - (c - str);
	memcpy(str, c, length + 1);
}

inline String::String(I16 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	char* c = str + 6;
	const char* threeDigits;
	U8 neg = 0;

	U16 abs = (U16)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U16)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U16 newVal = abs / 1000;
		U16 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	length = 6 + neg - (c - str);

	memcpy(str + neg, c, length + 1);
}

inline String::String(U16 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	char* c = str + 5;
	const char* threeDigits;

	while (value > 999)
	{
		U16 newVal = value / 1000;
		U16 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	length = 5 - (c - str);

	memcpy(str, c, length + 1);
}

inline String::String(I32 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	char* c = str + 11;
	const char* threeDigits;
	U8 neg = 0;

	U32 abs = (U32)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U32)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U32 newVal = abs / 1000;
		U32 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	length = 11 + neg - (c - str);

	memcpy(str + neg, c, length + 1);
}

inline String::String(U32 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	char* c = str + 10;
	const char* threeDigits;

	while (value > 999)
	{
		U32 newVal = value / 1000;
		U32 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	length = 10 - (c - str);

	memcpy(str, c, length + 1);
}

inline String::String(I64 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	char* c = str + 20;
	const char* threeDigits;
	U8 neg = 0;

	U64 abs = (U64)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U64)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U64 newVal = abs / 1000;
		U64 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	length = 20 + neg - (c - str);

	memcpy(str + neg, c, length + 1);
}

inline String::String(U64 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	char* c = str + 20;
	const char* threeDigits;

	while (value > 999)
	{
		U64 newVal = value / 1000;
		U64 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	length = 20 - (c - str);

	memcpy(str, c, length + 1);
}

inline String::String(F32 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	//TODO: Keep 5 decimal places

}

inline String::String(F64 value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	//TODO: Keep 5 decimal places

}

inline String::String(bool value) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	if (value)
	{
		length = 4;
		memcpy(str, "true", 4);
	}
	else
	{
		length = 5;
		memcpy(str, "false", 5);
	}
}

inline String::String(char* str) : length{ strlen(str) }, str{ (char*)Memory::Allocate1kb() }
{
	memcpy(this->str, str, length + 1);
}

inline String::String(const char* str) : length{ strlen(str) }, str{ (char*)Memory::Allocate1kb() }
{
	memcpy(this->str, str, length + 1);
}

inline String::String(const String& other) : length{ other.length }, str{ (char*)Memory::Allocate1kb() }
{
	memcpy(str, other.str, length + 1);
}

inline String::String(String&& other) noexcept : length{ other.length }, str{ other.str }
{
	other.Destroy();
}

template<typename... Types> inline String::String(const char* fmt, const Types& ... args) : length{ strlen(fmt) }, str{ (char*)Memory::Allocate1kb() }
{
	memcpy(str, fmt, length + 1);
	U64 start = 0;
	(Format(start, args), ...);
}

template<typename... Types> inline String::String(const Types& ... args) : length{ 0 }, str{ (char*)Memory::Allocate1kb() }
{
	(Append(args), ...);
}

inline String& String::operator=(I8 value)
{
	if (value < 0)
	{
		str[0] = '-';
		U8 abs = (U8)-value;
		char* c = str + 4;
		const char* threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
		*--c = threeDigits[2];
		if (abs > 9) { *--c = threeDigits[1]; }
		if (abs > 99) { *--c = threeDigits[0]; }

		length = 5 - (c - str);
		memcpy(str + 1, c, length);
		str[length] = '\0';
	}
	else
	{
		char* c = str + 3;
		const char* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
		*--c = threeDigits[2];
		if (value > 9) { *--c = threeDigits[1]; }
		if (value > 99) { *--c = threeDigits[0]; }

		length = 3 - (c - str);
		memcpy(str, c, length);
		str[length] = '\0';
	}

	return *this;
}

inline String& String::operator=(U8 value)
{
	char* c = str + 3;
	const char* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	length = 3 - (c - str);
	memcpy(str, c, length);
	str[length] = '\0';

	return *this;
}

inline String& String::operator=(I16 value)
{
	char* c = str + 6;
	const char* threeDigits;
	U8 neg = 0;

	U16 abs = (U16)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U16)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U16 newVal = abs / 1000;
		U16 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	length = 6 + neg - (c - str);

	memcpy(str + neg, c, length);
	str[length] = '\0';

	return *this;
}

inline String& String::operator=(U16 value)
{
	char* c = str + 5;
	const char* threeDigits;

	while (value > 999)
	{
		U16 newVal = value / 1000;
		U16 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	length = 5 - (c - str);

	memcpy(str, c, length);
	str[length] = '\0';

	return *this;
}

inline String& String::operator=(I32 value)
{
	char* c = str + 11;
	const char* threeDigits;
	U8 neg = 0;

	U32 abs = (U32)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U32)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U32 newVal = abs / 1000;
		U32 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	length = 11 + neg - (c - str);

	memcpy(str + neg, c, length);
	str[length] = '\0';

	return *this;
}

inline String& String::operator=(U32 value)
{
	char* c = str + 10;
	const char* threeDigits;

	while (value > 999)
	{
		U32 newVal = value / 1000;
		U32 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	length = 10 - (c - str);

	memcpy(str, c, length);
	str[length] = '\0';

	return *this;
}

inline String& String::operator=(I64 value)
{
	char* c = str + 20;
	const char* threeDigits;
	U8 neg = 0;

	U64 abs = (U64)value;

	if (value < 0)
	{
		str[0] = '-';
		abs = (U64)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U64 newVal = abs / 1000;
		U64 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	length = 20 + neg - (c - str);

	memcpy(str + neg, c, length);
	str[length] = '\0';

	return *this;
}

inline String& String::operator=(U64 value)
{
	char* c = str + 20;
	const char* threeDigits;

	while (value > 999)
	{
		U64 newVal = value / 1000;
		U64 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	length = 20 - (c - str);

	memcpy(str, c, length);
	str[length] = '\0';

	return *this;
}

inline String& String::operator=(F32 value)
{

	return *this;
}

inline String& String::operator=(F64 value)
{

	return *this;
}

inline String& String::operator=(bool value)
{
	if (value)
	{
		length = 4;
		memcpy(str, "true", 4);
		str[length] = '\0';
	}
	else
	{
		length = 5;
		memcpy(str, "false", 5);
		str[length] = '\0';
	}

	return *this;
}

inline String& String::operator=(char* str)
{
	length = strlen(str);
	memcpy(this->str, str, length + 1);

	return *this;
}

inline String& String::operator=(const char* str)
{
	length = strlen(str);
	memcpy(this->str, str, length + 1);

	return *this;
}

inline String& String::operator=(const String& other)
{
	length = other.length;
	memcpy(this->str, str, length + 1);

	return *this;
}

inline String& String::operator=(String&& other) noexcept
{
	length = other.length;
	memcpy(this->str, str, length + 1);
	other.Destroy();

	return *this;
}

inline String::~String()
{
	Destroy();
}

inline void String::Destroy()
{
	if (str)
	{
		length = 0;
		Memory::Free1kb(str);
	}
}

inline void String::Clear()
{
	str[0] = '\0';
	length = 0;
}

inline I8 String::ToI8()
{
	char* it = str;
	char c;
	I8 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline U8 String::ToU8()
{
	char* it = str;
	char c;
	U8 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline I16 String::ToI16()
{
	char* it = str;
	char c;
	I16 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline U16 String::ToU16()
{
	char* it = str;
	char c;
	U16 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline I32 String::ToI32()
{
	char* it = str;
	char c;
	I32 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline U32 String::ToU32()
{
	char* it = str;
	char c;
	U32 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline I64 String::ToI64()
{
	char* it = str;
	char c;
	I64 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline U64 String::ToU64()
{
	char* it = str;
	char c;
	U64 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline F32 String::ToF32()
{
	char* it = str;
	char c;
	F32 value = 0.0f;
	F32 mul = 0.1f;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value -= c - '0'; }
		while ((c = *it++) != '\0') { value -= (c - '0') * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value += c - '0'; }
		while ((c = *it++) != '\0') { value += (c - '0') * mul; mul *= 0.1f; }
	}

	return value;
}

inline F64 String::ToF64()
{
	char* it = str;
	char c;
	F64 value = 0.0f;
	F64 mul = 0.1f;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value -= c - '0'; }
		while ((c = *it++) != '\0') { value -= (c - '0') * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value += c - '0'; }
		while ((c = *it++) != '\0') { value += (c - '0') * mul; mul *= 0.1f; }
	}

	return value;
}

inline bool String::ToBool()
{
	return str[0] == 't' && str[1] == 'r' && str[2] == 'u' && str[3] == 'e';
}

inline String& String::operator+=(I8 value)
{
	char* start = str + length;

	if (value < 0)
	{
		start[0] = '-';
		U8 abs = (U8)-value;
		char* c = start + 4;
		const char* threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
		*--c = threeDigits[2];
		if (abs > 9) { *--c = threeDigits[1]; }
		if (abs > 99) { *--c = threeDigits[0]; }

		U64 addLength = 5 - (c - start);
		length += addLength;
		memcpy(start + 1, c, addLength);
		str[length] = '\0';
	}
	else
	{
		char* c = start + 3;
		const char* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
		*--c = threeDigits[2];
		if (value > 9) { *--c = threeDigits[1]; }
		if (value > 99) { *--c = threeDigits[0]; }

		U64 addLength = 3 - (c - start);
		length += addLength;
		memcpy(start, c, addLength);
		str[length] = '\0';
	}

	return *this;
}

inline String& String::operator+=(U8 value)
{
	char* start = str + length;
	char* c = start + 3;
	const char* threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	U64 addLength = 3 - (c - start);
	length += addLength;
	memcpy(start, c, addLength);
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(I16 value)
{
	char* start = str + length;
	char* c = start + 6;
	const char* threeDigits;
	U8 neg = 0;

	U16 abs = (U16)value;

	if (value < 0)
	{
		start[0] = '-';
		abs = (U16)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U16 newVal = abs / 1000;
		U16 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	U64 addLength = 6 + neg - (c - start);
	length += addLength;

	memcpy(start + neg, c, addLength);
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(U16 value)
{
	char* start = str + length;
	char* c = start + 5;
	const char* threeDigits;

	while (value > 999)
	{
		U16 newVal = value / 1000;
		U16 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	U64 addLength = 5 - (c - start);
	length += addLength;

	memcpy(start, c, addLength);
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(I32 value)
{
	char* start = str + length;
	char* c = start + 11;
	const char* threeDigits;
	U8 neg = 0;

	U32 abs = (U32)value;

	if (value < 0)
	{
		start[0] = '-';
		abs = (U32)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U32 newVal = abs / 1000;
		U32 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	U64 addLength = 11 + neg - (c - start);
	length += addLength;

	memcpy(start + neg, c, addLength);
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(U32 value)
{
	char* start = str + length;
	char* c = start + 10;
	const char* threeDigits;

	while (value > 999)
	{
		U32 newVal = value / 1000;
		U32 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	U64 addLength = 10 - (c - start);
	length += addLength;

	memcpy(start, c, addLength);
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(I64 value)
{
	char* start = str + length;
	char* c = start + 20;
	const char* threeDigits;
	U8 neg = 0;

	U64 abs = (U64)value;

	if (value < 0)
	{
		start[0] = '-';
		abs = (U64)-value;
		neg = 1;
	}

	while (abs > 999)
	{
		U64 newVal = abs / 1000;
		U64 remainder = abs % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		abs = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (abs * 3);
	*--c = threeDigits[2];
	if (abs > 9) { *--c = threeDigits[1]; }
	if (abs > 99) { *--c = threeDigits[0]; }

	U64 addLength = 20 + neg - (c - start);
	length += addLength;

	memcpy(start + neg, c, addLength);
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(U64 value)
{
	char* start = str + length;
	char* c = start + 20;
	const char* threeDigits;

	while (value > 999)
	{
		U64 newVal = value / 1000;
		U64 remainder = value % 1000;
		threeDigits = THREE_DIGIT_NUMBERS + (remainder * 3);
		*--c = threeDigits[2];
		*--c = threeDigits[1];
		*--c = threeDigits[0];
		value = newVal;
	}

	threeDigits = THREE_DIGIT_NUMBERS + (value * 3);
	*--c = threeDigits[2];
	if (value > 9) { *--c = threeDigits[1]; }
	if (value > 99) { *--c = threeDigits[0]; }

	U64 addLength = 20 - (c - start);
	length += addLength;

	memcpy(start, c, addLength);
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(F32 value)
{

}

inline String& String::operator+=(F64 value)
{

}

inline String& String::operator+=(bool value)
{
	if (value)
	{
		memcpy(str + length, "true", 4);
		length += 4;
		str[length] = '\0';
	}
	else
	{
		memcpy(str + length, "false", 5);
		length += 5;
		str[length] = '\0';
	}

	return *this;
}

inline String& String::operator+=(char* other)
{
	U64 addLength = strlen(other);
	memcpy(str + length, other, addLength);
	length += addLength;
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(const char* other)
{
	U64 addLength = strlen(other);
	memcpy(str + length, other, addLength);
	length += addLength;
	str[length] = '\0';

	return *this;
}

inline String& String::operator+=(const String& other)
{
	U64 addLength = strlen(other.str);
	memcpy(str + length, other.str, addLength);
	length += addLength;
	str[length] = '\0';

	return *this;
}

inline String::operator char* () { return str; }

inline String::operator const char* () const { return str; }

inline String::operator I8() const
{
	char* it = str;
	char c;
	I8 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline String::operator U8() const
{
	char* it = str;
	char c;
	U8 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline String::operator I16() const
{
	char* it = str;
	char c;
	I16 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline String::operator U16() const
{
	char* it = str;
	char c;
	U16 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline String::operator I32() const
{
	char* it = str;
	char c;
	I32 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline String::operator U32() const
{
	char* it = str;
	char c;
	U32 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline String::operator I64() const
{
	char* it = str;
	char c;
	I64 value = 0;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0') { value *= 10; value -= c - '0'; }
	}
	else
	{
		while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }
	}

	return value;
}

inline String::operator U64() const
{
	char* it = str;
	char c;
	U64 value = 0;

	while ((c = *it++) != '\0') { value *= 10; value += c - '0'; }

	return value;
}

inline String::operator F32() const
{
	char* it = str;
	char c;
	F32 value = 0.0f;
	F32 mul = 0.1f;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value -= c - '0'; }
		while ((c = *it++) != '\0') { value -= (c - '0') * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value += c - '0'; }
		while ((c = *it++) != '\0') { value += (c - '0') * mul; mul *= 0.1f; }
	}

	return value;
}

inline String::operator F64() const
{
	char* it = str;
	char c;
	F64 value = 0.0f;
	F64 mul = 0.1f;

	if (*str == '-')
	{
		++it;
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value -= c - '0'; }
		while ((c = *it++) != '\0') { value -= (c - '0') * mul; mul *= 0.1f; }
	}
	else
	{
		while ((c = *it++) != '\0' && c != '.') { value *= 10; value += c - '0'; }
		while ((c = *it++) != '\0') { value += (c - '0') * mul; mul *= 0.1f; }
	}

	return value;
}

inline String::operator bool() const
{
	return str[0] == 't' && str[1] == 'r' && str[2] == 'u' && str[3] == 'e';
}

inline char* String::operator*() { return str; }

inline const char* String::operator*() const { return str; }

inline char& String::operator[](U32 i) { return str[i]; }

inline const char& String::operator[](U32 i) const { return str[i]; }

inline bool String::operator==(char* other) const
{
	U64 len = strlen(other);
	if (len != length) { return false; }

	return memcmp(str, other, length) == 0;
}

inline bool String::operator==(const char* other) const
{
	U64 len = strlen(other);
	if (len != length) { return false; }

	return memcmp(str, other, length) == 0;
}

inline bool String::operator==(const String& other) const
{
	if (other.length != length) { return false; }

	return memcmp(str, other.str, length) == 0;
}

inline bool String::operator!=(char* other) const
{
	U64 len = strlen(other);
	if (len != length) { return true; }

	return memcmp(str, other, length);
}

inline bool String::operator!=(const char* other) const
{
	U64 len = strlen(other);
	if (len != length) { return true; }

	return memcmp(str, other, length);
}

inline bool String::operator!=(const String& other) const
{
	if (other.length != length) { return true; }

	return memcmp(str, other.str, length);
}

inline bool String::Compare(char* other) const
{
	U64 len = strlen(other);
	if (len != length) { return false; }

	return memcmp(str, other, length) == 0;
}

inline bool String::Compare(const char* other) const
{
	U64 len = strlen(other);
	if (len != length) { return false; }

	return memcmp(str, other, length) == 0;
}

inline bool String::Compare(const String& other) const
{
	if (other.length != length) { return false; }

	return memcmp(str, other.str, length) == 0;
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
	if (other.length != nLength) { return false; }

	return memcmp(str + start, other.str, nLength) == 0;
}

inline const U64& String::Length() const { return length; }

inline char* String::Data() { return str; }

inline const char* String::Data() const { return str; }

inline bool String::Blank() const
{
	if (length == 0) { return true; }
	char* start = str;
	char c;

	while ((c = *start) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f');

	return start - str == length;
}

inline I32 String::IndexOf(char c, U64 start) const
{
	char* it = str;

	while (*it != c && *it != '\0') { ++it; }

	if (*it == '\0') { return -1; }
	return it - str;
}

inline String& String::Trim()
{
	char* start = str;
	char* end = str + length;
	char c;

	while ((c = *start) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f' || c == '\0') { ++start; }
	while ((c = *end) == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f' || c == '\0') { --end; }

	length = end - start;
	memcpy(str, start, length);
	str[length] = '\0';

	return *this;
}

inline String& String::SubString(String& newStr, U64 start, U64 length) const
{
	if (length < U64_MAX)
	{
		newStr.length = length;
		memcpy(newStr.str, str + start, length);
		newStr.str[length] = '\0';
	}
	else
	{
		newStr.length = length - start;
		memcpy(newStr.str, str + start, newStr.length);
		newStr.str[newStr.length] = '\0';
	}

	return newStr;
}

inline String& String::Append(const String& append)
{
	memcpy(str + length, append.str, append.length);
	length += append.length;
	str[length] = '\0';

	return *this;
}

inline String& String::Prepend(const String& prepend)
{
	memcpy(str + length, str, length);
	memcpy(str, prepend.str, prepend.length);
	length += prepend.length;
	str[length] = '\0';

	return *this;
}

inline String& String::Surround(const String& prepend, const String& append)
{
	memcpy(str + prepend.length, str, length);
	memcpy(str, prepend.str, prepend.length);
	length += prepend.length;

	memcpy(str + length, append.str, append.length);
	length += append.length;
	str[length] = '\0';

	return *this;
}

inline String& String::Insert(const String& other, U32 i)
{
	memcpy(str + i + other.length, str + i, length - i);
	memcpy(str + i, other.str, other.length);
	length += other.length;
	str[length] = '\0';

	return *this;
}

inline String& String::ReplaceAll(const String& find, const String& replace, U64 start)
{
	char* c = str + start;
	char ch = *c;
	while (ch != '\0')
	{
		while ((ch = *c) != '\0' && memcmp(c, find.str, find.length)) { ++c; }

		if (ch != '\0')
		{
			memcpy(c + replace.length, c + find.length, length - find.length - (c - str));
			memcpy(c, replace.str, replace.length);
			length = length - find.length + replace.length;
			str[length] = '\0';
		}
	}

	return *this;
}

inline String& String::ReplaceN(const String& find, const String& replace, U64 count, U64 start)
{
	char* c = str + start;
	char ch = *c;
	while (ch != '\0' && count)
	{
		while ((ch = *c) != '\0' && memcmp(c, find.str, find.length)) { ++c; }

		if (ch != '\0')
		{
			--count;
			memcpy(c + replace.length, c + find.length, length - find.length - (c - str));
			memcpy(c, replace.str, replace.length);
			length = length - find.length + replace.length;
			str[length] = '\0';
		}
	}

	return *this;
}

inline String& String::ReplaceFirst(const String& find, const String& replace, U64 start)
{
	char* c = str + start;
	while (*c != '\0' && memcmp(c, find.str, find.length)) { ++c; }

	if (*c != '\0')
	{
		memcpy(c + replace.length, c + find.length, length - find.length - (c - str));
		memcpy(c, replace.str, replace.length);
		length = length - find.length + replace.length;
		str[length] = '\0';
	}

	return *this;
}

inline void String::Format(U64& start, const String& replace)
{
	char* c = str + start;
	while (*c != '\0' && memcmp(c, "{}", 2)) { ++c; }

	if (*c != '\0')
	{
		start = (c - str) + replace.length;
		memcpy(c + replace.length, c + 2, length - 2 - (c - str));
		memcpy(c, replace.str, replace.length);
		length = length - 2 + replace.length;
		str[length] = '\0';
	}
}

//inline Vector<String> String::Split(U8 delimiter, bool trimEntries) const

inline char* String::begin() { return str; }

inline char* String::end() { return str + length; }

inline const char* String::begin() const { return str; }

inline const char* String::end() const { return str + length; }

inline char* String::rbegin() { return str + length - 1; }

inline char* String::rend() { return str - 1; }

inline const char* String::rbegin() const { return str + length - 1; }

inline const char* String::rend() const { return str - 1; }