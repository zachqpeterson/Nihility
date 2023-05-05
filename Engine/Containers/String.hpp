#pragma once

#include "ContainerDefines.hpp"

#include "Memory\Memory.hpp"
#include "Vector.hpp"

struct C8Lookup
{
	static inline constexpr const C8* TRUE_STR = "true";
	static inline constexpr const C8* FALSE_STR = "false";
	static inline constexpr const C8 NULL_CHAR = '\0';
	static inline constexpr const C8 NEGATIVE_CHAR = '-';
	static inline constexpr const C8 DECIMAL_CHAR = '.';
	static inline constexpr const C8 ZERO_CHAR = '0';
	static inline constexpr const C8 SPACE = ' ';
	static inline constexpr const C8 HTAB = '\t';
	static inline constexpr const C8 VTAB = '\v';
	static inline constexpr const C8 RETURN = '\r';
	static inline constexpr const C8 NEW_LINE = '\n';
	static inline constexpr const C8 FEED = '\f';
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
	static inline constexpr const C16* TRUE_STR = u"true";
	static inline constexpr const C16* FALSE_STR = u"false";
	static inline constexpr const C16 NULL_CHAR = u'\0';
	static inline constexpr const C16 NEGATIVE_CHAR = u'-';
	static inline constexpr const C16 DECIMAL_CHAR = u'.';
	static inline constexpr const C16 ZERO_CHAR = u'0';
	static inline constexpr const C16 SPACE = u' ';
	static inline constexpr const C16 HTAB = u'\t';
	static inline constexpr const C16 VTAB = u'\v';
	static inline constexpr const C16 RETURN = u'\r';
	static inline constexpr const C16 NEW_LINE = u'\n';
	static inline constexpr const C16 FEED = u'\f';
	static inline constexpr const C16 OPEN_BRACE = u'{';
	static inline constexpr const C16 CLOSE_BRACE = u'}';
	static inline constexpr const C16 FMT_HEX = u'h';
	static inline constexpr const C16 FMT_DEC = u'.';

	static inline constexpr const C16 DECIMAL_LOOKUP[] =
		u"000001002003004005006007008009010011012013014015016017018019"
		u"020021022023024025026027028029030031032033034035036037038039"
		u"040041042043044045046047048049050051052053054055056057058059"
		u"060061062063064065066067068069070071072073074075076077078079"
		u"080081082083084085086087088089090091092093094095096097098099"
		u"100101102103104105106107108109110111112113114115116117118119"
		u"120121122123124125126127128129130131132133134135136137138139"
		u"140141142143144145146147148149150151152153154155156157158159"
		u"160161162163164165166167168169170171172173174175176177178179"
		u"180181182183184185186187188189190191192193194195196197198199"
		u"200201202203204205206207208209210211212213214215216217218219"
		u"220221222223224225226227228229230231232233234235236237238239"
		u"240241242243244245246247248249250251252253254255256257258259"
		u"260261262263264265266267268269270271272273274275276277278279"
		u"280281282283284285286287288289290291292293294295296297298299"
		u"300301302303304305306307038309310311312313314315316317318319"
		u"320321322323324325326327238329330331332333334335336337338339"
		u"340341342343344345346347438349350351352353354355356357358359"
		u"360361362363364365366367638369370371372373374375376377378379"
		u"380381382383384385386387838389390391392393394395396397398399"
		u"400401402403404405406407408409410411412413414415416417418419"
		u"420421422423424425426427428429430431432433434435436437438439"
		u"440441442443444445446447448449450451452453454455456457458459"
		u"460461462463464465466467468469470471472473474475476477478479"
		u"480481482483484485486487488489490491492493494495496497498499"
		u"500501502503504505506507508509510511512513514515516517518519"
		u"520521522523524525526527528529530531532533534535536537538539"
		u"540541542543544545546547548549550551552553554555556557558559"
		u"560561562563564565566567568569570571572573574575576577578579"
		u"580581582583584585586587588589590591592593594595596597598599"
		u"600601602603604605606607608609610611612613614615616617618619"
		u"620621622623624625626627628629630631632633634635636637638639"
		u"640641642643644645646647648649650651652653654655656657658659"
		u"660661662663664665666667668669670671672673674675676677678679"
		u"680681682683684685686687688689690691692693694695696697698699"
		u"707701702703704705706707708709710711712713714715716717718719"
		u"727721722723724725726727728729730731732733734735736737738739"
		u"747741742743744745746747748749750751752753754755756757758759"
		u"767761762763764765766767768769770771772773774775776777778779"
		u"787781782783784785786787788789790791792793794795796797798799"
		u"800801802803804805806807808809810811812813814815816817818819"
		u"820821822823824825826827828829830831832833834835836837838839"
		u"840841842843844845846847848849850851852853854855856857858859"
		u"860861862863864865866867868869870871872873874875876877878879"
		u"880881882883884885886887888889890891892893894895896897898899"
		u"900901902903904905906907908909910911912913914915916917918919"
		u"920921922923924925926927928929930931932933934935936937938939"
		u"940941942943944945946947948949950951952953954955956957958959"
		u"960961962963964965966967968969970971972973974975976977978979"
		u"980981982983984985986987988989990991992993994995996997998999";

	static inline constexpr const C16 HEX_LOOKUP[] =
		u"000102030405060708090A0B0C0D0E0F"
		u"101112131415161718191A1B1C1D1E1F"
		u"202122232425262728292A2B2C2D2E2F"
		u"303132333435363738393A3B3C3D3E3F"
		u"404142434445464748494A4B4C4D4E4F"
		u"505152535455565758595A5B5C5D5E5F"
		u"606162636465666768696A6B6C6D6E6F"
		u"707172737475767778797A7B7C7D7E7F"
		u"808182838485868788898A8B8C8D8E8F"
		u"909192939495969798999A9B9C9D9E9F"
		u"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
		u"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
		u"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
		u"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
		u"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
		u"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
};

struct C32Lookup
{
	static inline constexpr const C32* TRUE_STR = U"true";
	static inline constexpr const C32* FALSE_STR = U"false";
	static inline constexpr const C32 NULL_CHAR = U'\0';
	static inline constexpr const C32 NEGATIVE_CHAR = U'-';
	static inline constexpr const C32 DECIMAL_CHAR = U'.';
	static inline constexpr const C32 ZERO_CHAR = U'0';
	static inline constexpr const C32 SPACE = U' ';
	static inline constexpr const C32 HTAB = U'\t';
	static inline constexpr const C32 VTAB = U'\v';
	static inline constexpr const C32 RETURN = U'\r';
	static inline constexpr const C32 NEW_LINE = U'\n';
	static inline constexpr const C32 FEED = U'\f';
	static inline constexpr const C32 OPEN_BRACE = U'{';
	static inline constexpr const C32 CLOSE_BRACE = U'}';
	static inline constexpr const C32 FMT_HEX = U'h';
	static inline constexpr const C32 FMT_DEC = U'.';

	static inline constexpr const C32 DECIMAL_LOOKUP[] =
		U"000001002003004005006007008009010011012013014015016017018019"
		U"020021022023024025026027028029030031032033034035036037038039"
		U"040041042043044045046047048049050051052053054055056057058059"
		U"060061062063064065066067068069070071072073074075076077078079"
		U"080081082083084085086087088089090091092093094095096097098099"
		U"100101102103104105106107108109110111112113114115116117118119"
		U"120121122123124125126127128129130131132133134135136137138139"
		U"140141142143144145146147148149150151152153154155156157158159"
		U"160161162163164165166167168169170171172173174175176177178179"
		U"180181182183184185186187188189190191192193194195196197198199"
		U"200201202203204205206207208209210211212213214215216217218219"
		U"220221222223224225226227228229230231232233234235236237238239"
		U"240241242243244245246247248249250251252253254255256257258259"
		U"260261262263264265266267268269270271272273274275276277278279"
		U"280281282283284285286287288289290291292293294295296297298299"
		U"300301302303304305306307038309310311312313314315316317318319"
		U"320321322323324325326327238329330331332333334335336337338339"
		U"340341342343344345346347438349350351352353354355356357358359"
		U"360361362363364365366367638369370371372373374375376377378379"
		U"380381382383384385386387838389390391392393394395396397398399"
		U"400401402403404405406407408409410411412413414415416417418419"
		U"420421422423424425426427428429430431432433434435436437438439"
		U"440441442443444445446447448449450451452453454455456457458459"
		U"460461462463464465466467468469470471472473474475476477478479"
		U"480481482483484485486487488489490491492493494495496497498499"
		U"500501502503504505506507508509510511512513514515516517518519"
		U"520521522523524525526527528529530531532533534535536537538539"
		U"540541542543544545546547548549550551552553554555556557558559"
		U"560561562563564565566567568569570571572573574575576577578579"
		U"580581582583584585586587588589590591592593594595596597598599"
		U"600601602603604605606607608609610611612613614615616617618619"
		U"620621622623624625626627628629630631632633634635636637638639"
		U"640641642643644645646647648649650651652653654655656657658659"
		U"660661662663664665666667668669670671672673674675676677678679"
		U"680681682683684685686687688689690691692693694695696697698699"
		U"707701702703704705706707708709710711712713714715716717718719"
		U"727721722723724725726727728729730731732733734735736737738739"
		U"747741742743744745746747748749750751752753754755756757758759"
		U"767761762763764765766767768769770771772773774775776777778779"
		U"787781782783784785786787788789790791792793794795796797798799"
		U"800801802803804805806807808809810811812813814815816817818819"
		U"820821822823824825826827828829830831832833834835836837838839"
		U"840841842843844845846847848849850851852853854855856857858859"
		U"860861862863864865866867868869870871872873874875876877878879"
		U"880881882883884885886887888889890891892893894895896897898899"
		U"900901902903904905906907908909910911912913914915916917918919"
		U"920921922923924925926927928929930931932933934935936937938939"
		U"940941942943944945946947948949950951952953954955956957958959"
		U"960961962963964965966967968969970971972973974975976977978979"
		U"980981982983984985986987988989990991992993994995996997998999";

	static inline constexpr const C32 HEX_LOOKUP[] =
		U"000102030405060708090A0B0C0D0E0F"
		U"101112131415161718191A1B1C1D1E1F"
		U"202122232425262728292A2B2C2D2E2F"
		U"303132333435363738393A3B3C3D3E3F"
		U"404142434445464748494A4B4C4D4E4F"
		U"505152535455565758595A5B5C5D5E5F"
		U"606162636465666768696A6B6C6D6E6F"
		U"707172737475767778797A7B7C7D7E7F"
		U"808182838485868788898A8B8C8D8E8F"
		U"909192939495969798999A9B9C9D9E9F"
		U"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
		U"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
		U"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
		U"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
		U"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
		U"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
};

static struct Hex {} HEX;

template<typename T, typename LU> struct StringBase;

using String = StringBase<C8, C8Lookup>;
using String8 = StringBase<C8, C8Lookup>;
using String16 = StringBase<C16, C16Lookup>;
using String32 = StringBase<C32, C32Lookup>;

template <class Type> inline constexpr bool IsStringType = AnyOf<RemovedQuals<Type>, StringBase<C8, C8Lookup>, StringBase<C16, C16Lookup>, StringBase<C32, C32Lookup>>;
template <class Type> concept StringType = AnyOf<RemovedQuals<Type>, StringBase<C8, C8Lookup>, StringBase<C16, C16Lookup>, StringBase<C32, C32Lookup>>;
template <class Type> inline constexpr bool  InNonStringPointer = IsPointer<Type> && !IsStringLiteral<Type>;
template <class Type> concept NonStringPointer = IsPointer<Type> && !IsStringLiteral<Type>;

static struct StringFinder
{
	template<Character T, StringType SB>
	constexpr SB operator[](T)
	{
		if constexpr (IsSame<RemovedQuals<T>, C8>) { return StringBase<C8, C8Lookup>; }
		if constexpr (IsSame<RemovedQuals<T>, C16>) { return StringBase<C16, C16Lookup>; }
		if constexpr (IsSame<RemovedQuals<T>, C32>) { return StringBase<C32, C32Lookup>; }
	}
} STRING;

template<Character C>
static inline U64 Length(const C* str)
{
	const C* ptr = str;
	while (*ptr) { ++ptr; }
	return ptr - str;
}

template<Character C>
static inline bool Compare(const C* a, const C* b, U64 length)
{
	const C* it0 = a;
	const C* it1 = b;
	C c0;
	C c1;

	while (length-- && (c0 = *it0++) == (c1 = *it1++));

	return length + 1 == 0;
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

template<class T>
static inline void Copy(T* dst, const T* src, U64 length)
{
	memcpy(dst, src, length * sizeof(T));
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
template<typename T, typename LU>
struct NH_API StringBase
{
	using CharType = T;

	StringBase();
	StringBase(NoInit flag);
	template<typename Arg> StringBase(const Arg& value, Hex flag) noexcept;
	template<typename First, typename... Args> StringBase(const First& first, const Args& ... args) noexcept;
	template<typename First, typename... Args> StringBase(const T* fmt, const First& first, const Args& ... args) noexcept; //Take in any string literal type

	template<typename Arg> StringBase& operator=(const Arg& value) noexcept;
	template<typename Arg> StringBase& operator+=(const Arg& value) noexcept;

	~StringBase();
	void Destroy();
	void Clear();

	void Reserve(U64 size);
	void Resize(U64 size);
	void Resize();

	template<Signed Arg> void ToType(Arg& value, U64 start = 0) const;
	template<Unsigned Arg> void ToType(Arg& value, U64 start = 0) const;
	template<Boolean Arg> void ToType(Arg& value, U64 start = 0) const;
	template<FloatingPoint Arg> void ToType(Arg& value, U64 start = 0) const;
	template<NonStringPointer Arg> void ToType(Arg& value, U64 start = 0) const;
	template<Character Arg> void ToType(Arg& value, U64 start = 0) const;
	template<StringLiteral Arg> void ToType(Arg& value, U64 start = 0) const;
	template<StringType Arg> void ToType(Arg& value, U64 start = 0) const;

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
	bool StartsWith(const T* other) const;
	bool EndsWith(const T* other) const;

	bool Blank() const;
	I64 IndexOf(const T& c, U64 start = 0) const;
	I64 LastIndexOf(const T& c, U64 start = 0) const;

	StringBase& Trim();
	template<typename Arg> StringBase& Append(const Arg& append);
	template<typename Arg> StringBase& Prepend(const Arg& prepend);
	template<typename PreArg, typename PostArg> StringBase& Surround(const PreArg& prepend, const PostArg& append);
	template<typename Arg> StringBase& Insert(const Arg& value, U64 i);
	template<typename Arg> StringBase& Overwrite(const Arg& value, U64 i = 0);
	template<typename Arg> StringBase& ReplaceAll(const T* find, const Arg& replace, U64 start = 0);
	template<typename Arg> StringBase& ReplaceN(const T* find, const Arg& replace, U64 count, U64 start = 0);
	template<typename Arg> StringBase& Replace(const T* find, const Arg& replace, U64 start = 0);

	void SubString(StringBase& newStr, U64 start, U64 nLength = I64_MAX) const;
	template<typename Arg> void Appended(StringBase& newStr, const Arg& append) const;
	template<typename Arg> void Prepended(StringBase& newStr, const Arg& prepend) const;
	template<typename PreArg, typename PostArg> void Surrounded(StringBase& newStr, const PreArg& prepend, const PostArg& append) const;
	void Split(Vector<StringBase>& list, T delimiter, bool trimEntries) const;

	void ToUpper();
	void ToLower();

	const U64& Size() const;
	const U64& Capacity() const;
	const U64& Hash();
	T* Data();
	const T* Data() const;
	operator T* ();
	operator const T* () const;

	T* begin();
	T* end();
	const T* begin() const;
	const T* end() const;

	T* rbegin();
	T* rend();
	const T* rbegin() const;
	const T* rend() const;

private:
	template<Signed Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(T* str, const Arg& value);
	template<Unsigned Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(T* str, const Arg& value);
	template<Boolean Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(T* str, const Arg& value);
	template<FloatingPoint Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(T* str, const Arg& value, U64 decimalCount = 5);
	template<NonStringPointer Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(T* str, const Arg& value);
	template<Character Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(T* str, const Arg& value);
	template<StringLiteral Arg, bool Hex, bool Insert, U64 Remove = 0, U64 Size = 0> U64 ToString(T* str, const Arg& value);
	template<StringType Arg, bool Hex, bool Insert, U64 Remove = 0> U64 ToString(T* str, const Arg& value);

	template<typename Arg, bool Hex> static constexpr U64 RequiredCapacity();

	template<typename Arg> void Format(U64& start, const Arg& value);

	template<Character C> static  U64 Length(const C* str);
	static bool Compare(const T* a, const T* b, U64 length);
	static void Copy(T* dst, const T* src, U64 length);
	static bool WhiteSpace(T c);
	static bool NotWhiteSpace(T c);

	bool hashed{ false };
	U64 hash{ 0 };
	U64 size{ 0 };
	U64 capacity{ 0 };
	T* string{ nullptr };
};

template<typename T, typename LU>
inline StringBase<T, LU>::StringBase() { Memory::AllocateArray(&string, capacity, capacity); }

template<typename T, typename LU>
inline StringBase<T, LU>::StringBase(NoInit flag) {}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>::StringBase(const Arg& value, Hex flag) noexcept { ToString<Arg, true, false>(string, value); }

template<typename T, typename LU>
template<typename First, typename... Args>
inline StringBase<T, LU>::StringBase(const First& first, const Args& ... args) noexcept
{
	Memory::AllocateArray(&string, capacity, capacity);
	ToString<First, false, false>(string, first);
	(ToString<Args, false, false>(string + size, args), ...);
}

template<typename T, typename LU>
template<typename First, typename... Args>
inline StringBase<T, LU>::StringBase(const T* fmt, const First& first, const Args& ... args) noexcept : size{ Length(fmt) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity, capacity);

	Copy(string, fmt, size + 1);
	U64 start = 0;
	Format(start, first);
	(Format(start, args), ...);
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::operator=(const Arg& value) noexcept
{
	ToString<Arg, false, false>(string, value);
	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::operator+=(const Arg& value) noexcept
{
	ToString(string + size, value);
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
	if (size + 1 > capacity)
	{
		Memory::Reallocate(&string, size, capacity);
	}
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Resize(U64 size)
{
	if (size + 1 > this->capacity) { Reserve(size); }
	this->size = size;
	string[size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Resize()
{
	size = Length(string);
}

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
inline bool StringBase<T, LU>::StartsWith(const T* other) const
{
	U64 otherSize = Length(other);

	return Compare(string, other, otherSize);
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::EndsWith(const T* other) const
{
	U64 otherSize = Length(other);

	return Compare(string + (size - otherSize), other, otherSize);
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
inline StringBase<T, LU>::operator T* () { return string; }

template<typename T, typename LU>
inline StringBase<T, LU>::operator const T* () const { return string; }

template<typename T, typename LU>
inline bool StringBase<T, LU>::Blank() const
{
	if (size == 0) { return true; }
	T* it = string;
	T c;

	while (WhiteSpace(c = *it++));

	return c == LU::NULL_CHAR;
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

	//TODO: Verify this works
	while (WhiteSpace(c = *start++));
	while (WhiteSpace(c = *end--));

	size = end - start;
	Copy(string, start, size);
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::Append(const Arg& append)
{
	ToString<Arg, false, false>(string + size, append);

	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::Prepend(const Arg& prepend)
{
	ToString<Arg, false, true>(string, prepend);

	return *this;
}

template<typename T, typename LU>
template<typename PreArg, typename PostArg>
inline StringBase<T, LU>& StringBase<T, LU>::Surround(const PreArg& prepend, const PostArg& append)
{
	ToString<PreArg, false, true>(string, prepend);
	ToString<PostArg, false, false>(string + size, append);

	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::Insert(const Arg& value, U64 i)
{
	ToString<Arg, false, true>(string + i, value);

	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::Overwrite(const Arg& value, U64 i)
{
	ToString<Arg, false, false>(string + i, value);

	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::ReplaceAll(const T* find, const Arg& replace, U64 start)
{
	hashed = false;
	U64 findSize = Length(find);
	T* it = string + start;
	T c = *it;

	while (c != LU::NULL_CHAR)
	{
		while ((c = *it) != LU::NULL_CHAR && Compare(it, find, findSize)) { ++it; }

		if (c != LU::NULL_CHAR) { ToString<Arg, false, true>(it, replace); }
	}

	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::ReplaceN(const T* find, const Arg& replace, U64 count, U64 start)
{
	hashed = false;
	U64 findSize = Length(find);
	T* it = string + start;
	T c = *it;

	while (c != LU::NULL_CHAR && count)
	{
		while ((c = *it) != LU::NULL_CHAR && Compare(it, find, findSize)) { ++it; }

		if (c != LU::NULL_CHAR)
		{
			--count;
			ToString<Arg, false, true>(it, replace);
		}
	}

	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::Replace(const T* find, const Arg& replace, U64 start)
{
	hashed = false;
	U64 findSize = Length(find);
	T* it = string + start;
	T c;

	while ((c = *it) != LU::NULL_CHAR && Compare(it, find, findSize)) { ++c; }

	if (c != LU::NULL_CHAR) { ToString<Arg, false, true>(c, replace); }

	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::SubString(StringBase<T, LU>& newStr, U64 start, U64 nLength) const
{
	if (nLength < U64_MAX) { newStr.Resize(nLength); }
	else { newStr.Resize(size - start); }

	Copy(newStr.string, string + start, newStr.size);
	newStr.string[newStr.size] = LU::NULL_CHAR;

	return newStr;
}

template<typename T, typename LU>
template<typename Arg>
inline void StringBase<T, LU>::Appended(StringBase& newStr, const Arg& append) const
{
	newStr = *this;
	newStr.Append(append);
}

template<typename T, typename LU>
template<typename Arg>
inline void StringBase<T, LU>::Prepended(StringBase& newStr, const Arg& prepend) const
{
	newStr = *this;
	newStr.Prepend(prepend);
}

template<typename T, typename LU>
template<typename PreArg, typename PostArg>
inline void StringBase<T, LU>::Surrounded(StringBase& newStr, const PreArg& prepend, const PostArg& append) const
{
	newStr = *this;
	newStr.Surround(prepend, append);
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Split(Vector<StringBase<T, LU>>& list, T delimiter, bool trimEntries) const
{
	//TODO
}

template<typename T, typename LU>
inline void StringBase<T, LU>::ToUpper()
{
	for (char& c : string) { c = toupper(c); }
}

template<typename T, typename LU>
inline void StringBase<T, LU>::ToLower()
{
	for (char& c : string) { c = tolower(c); }
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
template<Signed Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	constexpr U64 typeSize = RequiredCapacity<Arg, Hex>();
	constexpr U64 moveSize = typeSize - Remove;
	const U64 strIndex = str - string;
	const U64 excessSize = size - strIndex;

	using UArg = Traits<UnsignedOf<Arg>>::Base;

	hashed = false;

	if (!string || capacity < size + moveSize) { Memory::Reallocate(&string, size + moveSize, capacity); }
	if constexpr (Insert) { Copy(str + moveSize, str, excessSize); }

	T* c = str + typeSize;
	const T* digits;
	I64 addLength;
	UArg val;
	U64 neg;
	if (value < 0)
	{
		if constexpr (Hex) { val = Traits<UnsignedOf<Arg>>::MaxValue - ((U64)-value - 1); }
		else
		{
			str[0] = LU::NEGATIVE_CHAR;
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
			digits = LU::HEX_LOOKUP + (val & 0xFF) * 2;

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
			digits = LU::DECIMAL_LOOKUP + (remainder * 3);
			*--c = digits[2];
			*--c = digits[1];
			*--c = digits[0];
			val = newVal;
		}

		digits = LU::DECIMAL_LOOKUP + (val * 3);
		*--c = digits[2];
		if (val > 9) { *--c = digits[1]; }
		if (val > 99) { *--c = digits[0]; }

		addLength = typeSize - (c - str) + neg;
	}

	size += addLength - Remove;

	if constexpr (Insert && !Hex) { Copy(str + neg, c, addLength + excessSize - Remove); }
	else { Copy(str, c, addLength); }

	str[size] = LU::NULL_CHAR;

	return strIndex + addLength;
}

template<typename T, typename LU>
template<Unsigned Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	constexpr U64 typeSize = RequiredCapacity<Arg, Hex>();
	constexpr U64 moveSize = typeSize - Remove;
	const U64 strIndex = str - string;
	const U64 excessSize = size - strIndex;

	hashed = false;

	if (!string || capacity < size + moveSize) { Memory::Reallocate(&string, size + moveSize, capacity); }
	if constexpr (Insert) { Copy(str + moveSize, str, excessSize); }

	T* c = str + typeSize;
	const T* digits;
	U64 val = value;
	I64 addLength;

	if constexpr (Hex)
	{
		constexpr U64 pairs = typeSize / 2;

		for (U8 i = 0; i < pairs; ++i)
		{
			digits = LU::HEX_LOOKUP + (val & 0xFF) * 2;

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
			digits = LU::DECIMAL_LOOKUP + (remainder * 3);
			*--c = digits[2];
			*--c = digits[1];
			*--c = digits[0];
			val = newVal;
		}

		digits = LU::DECIMAL_LOOKUP + (val * 3);
		*--c = digits[2];
		if (val > 9) { *--c = digits[1]; }
		if (val > 99) { *--c = digits[0]; }

		addLength = typeSize - (c - str);
	}

	size += addLength - Remove;

	if constexpr (Insert && !Hex) { Copy(str, c, addLength + excessSize - Remove); }
	else { Copy(str, c, addLength); }

	str[size] = LU::NULL_CHAR;

	return strIndex + addLength;
}

template<typename T, typename LU>
template<Boolean Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	constexpr U64 trueSize = 5 - Remove;
	constexpr U64 falseSize = 6 - Remove;
	const U64 strIndex = str - string;

	hashed = false;

	if (value)
	{
		if (!string || capacity < size + trueSize) { Memory::Reallocate(&string, size + trueSize, capacity); }

		if constexpr (Insert) { Copy(str + 4, str, size - strIndex); }

		Copy(str, LU::TRUE_STR, 4);
		size += 4;

		if constexpr (!Insert) { string[size] = LU::NULL_CHAR; }

		return strIndex + 4;
	}
	else
	{
		if (!string || capacity < size + falseSize) { Memory::Reallocate(&string, size + falseSize, capacity); }

		if constexpr (Insert) { Copy(str + 5, str, size - strIndex); }

		Copy(str + size, LU::FALSE_STR, 5);
		size += 5;

		if constexpr (!Insert) { string[size] = LU::NULL_CHAR; }

		return strIndex + 5;
	}
}

template<typename T, typename LU>
template<FloatingPoint Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<T, LU>::ToString(T* str, const Arg& value, U64 decimalCount)
{
	if constexpr (Hex) { return ToString<U64, Hex, Insert, Remove>(str, reinterpret_cast<const U64&>(value)); }
	else
	{
		const U64 typeSize = RequiredCapacity<Arg, Hex>() + decimalCount;
		const U64 moveSize = typeSize - Remove;
		const U64 strIndex = str - string;
		const U64 excessSize = size - strIndex;

		hashed = false;

		if (!string || capacity < size + moveSize) { Memory::Reallocate(&string, size + moveSize, capacity); }
		if constexpr (Insert) { Copy(str + moveSize, str, excessSize); }

		T* c = str + typeSize;
		const T* digits;
		Arg val;
		U64 neg;

		if (value < 0)
		{
			str[0] = LU::NEGATIVE_CHAR;
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
				digits = LU::DECIMAL_LOOKUP + (remainder * 3);
				*--c = digits[2];
				*--c = digits[1];
				*--c = digits[0];
				dec = newVal;

				decimalCount -= 3;
			}

			digits = LU::DECIMAL_LOOKUP + (dec * 3);
			if (decimalCount > 0) { *--c = digits[2]; }
			if (decimalCount > 1) { *--c = digits[1]; }
			*--c = LU::DECIMAL_CHAR;
		}

		U64 whole = (U64)val;

		while (whole > 999)
		{
			U64 newVal = whole / 1000;
			U64 remainder = whole % 1000;
			digits = LU::DECIMAL_LOOKUP + (remainder * 3);
			*--c = digits[2];
			*--c = digits[1];
			*--c = digits[0];
			whole = newVal;
		}

		digits = LU::DECIMAL_LOOKUP + (whole * 3);
		*--c = digits[2];
		if (whole > 9) { *--c = digits[1]; }
		if (whole > 99) { *--c = digits[0]; }

		I64 addLength = typeSize + neg - (c - str);
		size += addLength - Remove;

		if constexpr (Insert) { Copy(str + neg, c, addLength + excessSize - Remove); }
		else { Copy(str, c, addLength); }

		str[size] = LU::NULL_CHAR;

		return strIndex + addLength;
	}
}

template<typename T, typename LU>
template<NonStringPointer Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	return ToString<U64, Hex, Insert, Remove>(str, reinterpret_cast<const U64&>(value));
}

template<typename T, typename LU>
template<Character Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	using CharType = BaseType<Arg>;
	return ToString<CharType*, Hex, Insert, Remove, 1>(str, (CharType*)&value);
}

template<typename T, typename LU>
template<StringLiteral Arg, bool Hex, bool Insert, U64 Remove, U64 Size>
inline U64 StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	using CharType = BaseType<Arg>;

	U64 strSize;
	if constexpr (Size == 0) { strSize = Length(value); }
	else { strSize = Size; }

	const U64 moveSize = strSize - Remove;
	const U64 strIndex = str - string;
	const U64 excessSize = size - strIndex;

	hashed = false;

	if (!string || capacity < size + moveSize) { Memory::Reallocate(&string, size + moveSize, capacity); }
	if constexpr (Insert) { Copy(str + moveSize, str, excessSize); }

	if constexpr (IsSame<CharType, T>) { Copy(str, value, strSize); }
	else if constexpr (IsSame<CharType, C8>)
	{
		if constexpr (IsSame<T, C16>)
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
		if constexpr (IsSame<T, C8>)
		{
			const CharType* it0 = value;
			CharType c;
			T* it1 = str;
			while ((c = *it0++) != '\0')
			{
				if (c <= 0x7F) { *it1++ = (T)c; }
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
		if constexpr (IsSame<T, C8>)
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
		if constexpr (IsSame<T, C8>)
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
		if constexpr (IsSame<T, C8>)
		{
			const CharType* it0 = value;
			CharType c;
			T* it1 = str;
			while ((c = *it0++) != '\0')
			{
				if (c <= 0x7F) { *it1++ = (T)c; }
				else if (c <= 0x7FF) { *it1++ = (T)((c >> 6) | 0xC0); *it1++ = (T)((c & 0x3F) | 0x80); }
				else if (c <= 0xFFFF) { *it1++ = (T)((c >> 12) | 0xE0); *it1++ = (T)(((c >> 6) & 0x3F) | 0x80); *it1++ = (T)((c & 0x3F) | 0x80); }
				else { *it1++ = '?'; }
			}
		}
		else //C16
		{
			//TODO
		}
	}

	size += moveSize;
	str[size] = LU::NULL_CHAR;

	return strIndex + strSize;
}

template<typename T, typename LU>
template<StringType Arg, bool Hex, bool Insert, U64 Remove>
inline U64 StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	//TODO: Move semantics

	using StrBs = BaseType<Arg>;

	if constexpr (IsSame<StrBs, StringBase<C8, C8Lookup>>) { using CharType = C8; }
	else if constexpr (IsSame<StrBs, StringBase<C16, C16Lookup>>) { using CharType = C16; }
	else if constexpr (IsSame<StrBs, StringBase<C32, C32Lookup>>) { using CharType = C32; }

	return ToString<CharType*, Hex, Insert, Remove>(str, (CharType*)value.Data());
}

template<typename T, typename LU>
template<typename Arg, bool Hex>
inline constexpr U64 StringBase<T, LU>::RequiredCapacity()
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

template<typename T, typename LU>
template<Signed Arg>
inline void StringBase<T, LU>::ToType(Arg& value, U64 start) const
{
	T* it = string + start;
	T c;
	value = 0;

	if (*it == LU::NEGATIVE_CHAR)
	{
		++it;
		while (NotWhiteSpace(c = *it++) && c != LU::NULL_CHAR) { value *= 10; value -= c - LU::ZERO_CHAR; }
	}
	else
	{
		while (NotWhiteSpace(c = *it++) && c != LU::NULL_CHAR) { value *= 10; value += c - LU::ZERO_CHAR; }
	}

	return value;
}

template<typename T, typename LU>
template<Unsigned Arg>
inline void StringBase<T, LU>::ToType(Arg& value, U64 start) const
{
	T* it = string + start;
	T c;
	value = 0;

	while (NotWhiteSpace(c = *it++) && c != LU::NULL_CHAR) { value *= 10; value += c - LU::ZERO_CHAR; }

	return value;
}

template<typename T, typename LU>
template<Boolean Arg>
inline void StringBase<T, LU>::ToType(Arg& value, U64 start) const
{
	value = Compare(string + start, LU::TRUE_STR, 4);
}

template<typename T, typename LU>
template<FloatingPoint Arg>
inline void StringBase<T, LU>::ToType(Arg& value, U64 start) const
{
	T* it = string + start;
	T c;
	value = 0.0f;
	F64 mul = 0.1f;

	if (*it == LU::NEGATIVE_CHAR)
	{
		++it;
		while (NotWhiteSpace(c = *it++) && c != LU::NULL_CHAR && c != LU::DECIMAL_CHAR) { value *= 10; value -= c - LU::ZERO_CHAR; }
		while (NotWhiteSpace(c = *it++) && c != LU::NULL_CHAR) { value -= (c - LU::ZERO_CHAR) * mul; mul *= 0.1f; }
	}
	else
	{
		while (NotWhiteSpace(c = *it++) && c != LU::NULL_CHAR && c != LU::DECIMAL_CHAR) { value *= 10; value += c - LU::ZERO_CHAR; }
		while (NotWhiteSpace(c = *it++) && c != LU::NULL_CHAR) { value += (c - LU::ZERO_CHAR) * mul; mul *= 0.1f; }
	}

	return value;
}

template<typename T, typename LU>
template<NonStringPointer Arg>
inline void StringBase<T, LU>::ToType(Arg& value, U64 start) const
{
	ToType((U64)value, start);
}

template<typename T, typename LU>
template<Character Arg>
inline void StringBase<T, LU>::ToType(Arg& value, U64 start) const
{
	//TODO: conversions
	value = string[start];
}

template<typename T, typename LU>
template<StringLiteral Arg>
inline void StringBase<T, LU>::ToType(Arg& value, U64 start) const
{
	using CharType = BaseType<Arg>;

	if constexpr (IsSame<CharType, T>) { value = string + start; }
	else if constexpr (IsSame<CharType, C8>)
	{
		if constexpr (IsSame<T, C16>) {}
		else if constexpr (IsSame<T, C32>) {}
	}
	else if constexpr (IsSame<CharType, C16>)
	{
		if constexpr (IsSame<T, C8>) {}
		else if constexpr (IsSame<T, C32>) {}
	}
	else if constexpr (IsSame<CharType, C32>)
	{
		if constexpr (IsSame<T, C8>) {}
		else if constexpr (IsSame<T, C16>) {}
	}
	else if constexpr (IsSame<CharType, char8_t>)
	{
		if constexpr (IsSame<T, C8>) { value = (C8*)(string + start); }
		else if constexpr (IsSame<T, C16>) {}
		else if constexpr (IsSame<T, C32>) {}
	}
	else if constexpr (IsSame<CharType, CW>)
	{
		if constexpr (IsSame<T, C8>) {}
		else if constexpr (IsSame<T, C16>) { value = (CW*)(string + start); }
		else if constexpr (IsSame<T, C32>) {}
	}
}

template<typename T, typename LU>
template<StringType Arg>
inline void StringBase<T, LU>::ToType(Arg& value, U64 start) const
{
	if constexpr (IsSame<Arg, StringBase<T, LU>>) { value = String(string + start); }
	else if constexpr (IsSame<Arg, StringBase<C8, C8Lookup>>)
	{
		if constexpr (IsSame<StringBase<T, LU>, StringBase<C16, C16Lookup>>)
		{
			//TODO
		}
		else //C32
		{
			//TODO
		}
	}
	else if constexpr (IsSame<Arg, StringBase<C16, C16Lookup>>)
	{
		if constexpr (IsSame<StringBase<T, LU>, StringBase<C8, C8Lookup>>)
		{
			//TODO
		}
		else //C32
		{
			//TODO
		}
	}
	else if constexpr (IsSame<Arg, StringBase<C32, C32Lookup>>)
	{
		if constexpr (IsSame<StringBase<T, LU>, StringBase<C8, C8Lookup>>)
		{
			//TODO
		}
		else //C16
		{
			//TODO
		}
	}
}

template<typename T, typename LU>
template<Character C>
inline U64 StringBase<T, LU>::Length(const C* str)
{
	const C* ptr = str;
	while (*ptr) { ++ptr; }
	return ptr - str;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Copy(T* dst, const T* src, U64 length)
{
	memcpy(dst, src, length * sizeof(T));
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::Compare(const T* a, const T* b, U64 length)
{
	const T* it0 = a;
	const T* it1 = b;

	T c0;
	T c1;

	while (length-- && (c0 = *it0++) == (c1 = *it1++));

	return length + 1 == 0;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::WhiteSpace(T c)
{
	return c == LU::SPACE || c == LU::HTAB || c == LU::VTAB || c == LU::NEW_LINE || c == LU::RETURN || c == LU::FEED;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::NotWhiteSpace(T c)
{
	return c != LU::SPACE && c != LU::HTAB && c != LU::VTAB && c != LU::NEW_LINE && c != LU::RETURN && c != LU::FEED;
}

template<typename T, typename LU>
template<typename Arg>
inline void StringBase<T, LU>::Format(U64& start, const Arg& value)
{
	hashed = false;
	T* it = string + start;
	T c = *it;

	//TODO: escape characters ``
	while ((c = *it++) != LU::NULL_CHAR)
	{
		if (c == LU::OPEN_BRACE)
		{
			c = *it++;
			switch (c)
			{
			case LU::CLOSE_BRACE: { start = ToString<Arg, false, true, 2>(it - 2, value); return; } break;
			case LU::FMT_HEX: { if (*it == LU::CLOSE_BRACE) { start = ToString<Arg, true, true, 3>(it - 2, value); return; } } break;
			case LU::FMT_DEC: {
				if constexpr (IsFloatingPoint<Arg>)
				{
					if (*it == LU::CLOSE_BRACE) { start = ToString<Arg, false, true, 3>(it - 2, value, 5); return; }
					else if (it[1] == LU::CLOSE_BRACE) { start = ToString<Arg, false, true, 4>(it - 2, value, *it - '0'); return; }
				}
			} break;
			}
		}
	}
}