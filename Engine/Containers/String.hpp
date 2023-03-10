#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "Memory\Memory.hpp"
#include "Vector.hpp"

struct C8Lookup
{
	static inline constexpr const C8* TRUE_STR = u8"true";
	static inline constexpr const C8* FALSE_STR = u8"false";
	static inline constexpr const C8 NULL_CHAR = u8'\0';
	static inline constexpr const C8 NEGATIVE_CHAR = u8'-';
	static inline constexpr const C8 DECIMAL_CHAR = u8'.';
	static inline constexpr const C8 ZERO_CHAR = u8'0';
	static inline constexpr const C8 SPACE = u8' ';
	static inline constexpr const C8 HTAB = u8'\t';
	static inline constexpr const C8 VTAB = u8'\v';
	static inline constexpr const C8 RETURN = u8'\r';
	static inline constexpr const C8 NEW_LINE = u8'\n';
	static inline constexpr const C8 FEED = u8'\f';
	static inline constexpr const C8 OPEN_BRACE = u8'{';
	static inline constexpr const C8 CLOSE_BRACE = u8'}';
	static inline constexpr const C8 FMT_HEX = u8'h';
	static inline constexpr const C8 FMT_DEC = u8'.';

	static inline constexpr const C8 DECIMAL_LOOKUP[] =
		u8"000001002003004005006007008009010011012013014015016017018019"
		u8"020021022023024025026027028029030031032033034035036037038039"
		u8"040041042043044045046047048049050051052053054055056057058059"
		u8"060061062063064065066067068069070071072073074075076077078079"
		u8"080081082083084085086087088089090091092093094095096097098099"
		u8"100101102103104105106107108109110111112113114115116117118119"
		u8"120121122123124125126127128129130131132133134135136137138139"
		u8"140141142143144145146147148149150151152153154155156157158159"
		u8"160161162163164165166167168169170171172173174175176177178179"
		u8"180181182183184185186187188189190191192193194195196197198199"
		u8"200201202203204205206207208209210211212213214215216217218219"
		u8"220221222223224225226227228229230231232233234235236237238239"
		u8"240241242243244245246247248249250251252253254255256257258259"
		u8"260261262263264265266267268269270271272273274275276277278279"
		u8"280281282283284285286287288289290291292293294295296297298299"
		u8"300301302303304305306307038309310311312313314315316317318319"
		u8"320321322323324325326327238329330331332333334335336337338339"
		u8"340341342343344345346347438349350351352353354355356357358359"
		u8"360361362363364365366367638369370371372373374375376377378379"
		u8"380381382383384385386387838389390391392393394395396397398399"
		u8"400401402403404405406407408409410411412413414415416417418419"
		u8"420421422423424425426427428429430431432433434435436437438439"
		u8"440441442443444445446447448449450451452453454455456457458459"
		u8"460461462463464465466467468469470471472473474475476477478479"
		u8"480481482483484485486487488489490491492493494495496497498499"
		u8"500501502503504505506507508509510511512513514515516517518519"
		u8"520521522523524525526527528529530531532533534535536537538539"
		u8"540541542543544545546547548549550551552553554555556557558559"
		u8"560561562563564565566567568569570571572573574575576577578579"
		u8"580581582583584585586587588589590591592593594595596597598599"
		u8"600601602603604605606607608609610611612613614615616617618619"
		u8"620621622623624625626627628629630631632633634635636637638639"
		u8"640641642643644645646647648649650651652653654655656657658659"
		u8"660661662663664665666667668669670671672673674675676677678679"
		u8"680681682683684685686687688689690691692693694695696697698699"
		u8"707701702703704705706707708709710711712713714715716717718719"
		u8"727721722723724725726727728729730731732733734735736737738739"
		u8"747741742743744745746747748749750751752753754755756757758759"
		u8"767761762763764765766767768769770771772773774775776777778779"
		u8"787781782783784785786787788789790791792793794795796797798799"
		u8"800801802803804805806807808809810811812813814815816817818819"
		u8"820821822823824825826827828829830831832833834835836837838839"
		u8"840841842843844845846847848849850851852853854855856857858859"
		u8"860861862863864865866867868869870871872873874875876877878879"
		u8"880881882883884885886887888889890891892893894895896897898899"
		u8"900901902903904905906907908909910911912913914915916917918919"
		u8"920921922923924925926927928929930931932933934935936937938939"
		u8"940941942943944945946947948949950951952953954955956957958959"
		u8"960961962963964965966967968969970971972973974975976977978979"
		u8"980981982983984985986987988989990991992993994995996997998999";

	static inline constexpr const C8 HEX_LOOKUP[] =
		u8"000102030405060708090A0B0C0D0E0F"
		u8"101112131415161718191A1B1C1D1E1F"
		u8"202122232425262728292A2B2C2D2E2F"
		u8"303132333435363738393A3B3C3D3E3F"
		u8"404142434445464748494A4B4C4D4E4F"
		u8"505152535455565758595A5B5C5D5E5F"
		u8"606162636465666768696A6B6C6D6E6F"
		u8"707172737475767778797A7B7C7D7E7F"
		u8"808182838485868788898A8B8C8D8E8F"
		u8"909192939495969798999A9B9C9D9E9F"
		u8"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
		u8"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
		u8"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
		u8"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
		u8"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
		u8"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";
};

struct C16Lookup
{
	static inline constexpr const C16* TRUE_STR = L"true";
	static inline constexpr const C16* FALSE_STR = L"false";
	static inline constexpr const C16 NULL_CHAR = L'\0';
	static inline constexpr const C16 NEGATIVE_CHAR = L'-';
	static inline constexpr const C16 DECIMAL_CHAR = L'.';
	static inline constexpr const C16 ZERO_CHAR = L'0';
	static inline constexpr const C16 SPACE = L' ';
	static inline constexpr const C16 HTAB = L'\t';
	static inline constexpr const C16 VTAB = L'\v';
	static inline constexpr const C16 RETURN = L'\r';
	static inline constexpr const C16 NEW_LINE = L'\n';
	static inline constexpr const C16 FEED = L'\f';
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
static struct NoInit {} NO_INIT;

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
	using CharType = T;

	StringBase();
	StringBase(NoInit flag);
	template<typename Arg> StringBase(const Arg& value) noexcept;
	template<typename Arg> StringBase(const Arg& value, Hex flag) noexcept;
	template<typename... Args> StringBase(const T* fmt, const Args& ... args) noexcept;
	template<typename... Args> StringBase(const Args& ... args) noexcept;

	template<typename Arg> StringBase& operator=(const Arg& value) noexcept;

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
	template<Pointer Arg> Arg ToType(U64 start = 0) const;

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
	char* CStr();
	const char* CStr() const;

	T* begin();
	T* end();
	const T* begin() const;
	const T* end() const;

	T* rbegin();
	T* rend();
	const T* rbegin() const;
	const T* rend() const;

private:
	template<Signed Arg> void ToString(T* str, const Arg& value);
	template<Unsigned Arg> void ToString(T* str, const Arg& value);
	template<Boolean Arg> void ToString(T* str, const Arg& value);
	template<FloatingPoint Arg> void ToString(T* str, const Arg& value);
	template<Pointer Arg> void ToString(T* str, const Arg& value);
	template<Character Arg> void ToString(T* str, const Arg& value);
	template<StringLiteral Arg> void ToString(T* str, const Arg& value);
	//template<StrBs Arg> void ToString(T* str, const Arg& value);

	template<Signed Arg> void HexToString(T* str, const Arg& value);
	template<Unsigned Arg> void HexToString(T* str, const Arg& value);
	template<FloatingPoint Arg> void HexToString(T* str, const Arg& value);
	template<Pointer Arg> void HexToString(T* str, const Arg& value);

	template<Signed Arg> U64 InsertType(T* str, const Arg& value, U64 rmvAmt);
	template<Unsigned Arg> U64 InsertType(T* str, const Arg& value, U64 rmvAmt);
	template<Boolean Arg> U64 InsertType(T* str, const Arg& value, U64 rmvAmt);
	template<FloatingPoint Arg> U64 InsertType(T* str, const Arg& value, U64 rmvAmt, U64 decimalCount = 5);
	template<Pointer Arg> U64 InsertType(T* str, const Arg& value, U64 rmvAmt);
	template<Character Arg> U64 InsertType(T* str, const Arg& value, U64 rmvAmt);
	template<StringLiteral Arg> U64 InsertType(T* str, const Arg& value, U64 rmvAmt);
	//template<StrBs Arg> U64 InsertType(T* str, const Arg& value, U64 rmvAmt);

	template<Signed Arg> U64 InsertHex(T* str, const Arg& value, U64 rmvAmt);
	template<Unsigned Arg> U64 InsertHex(T* str, const Arg& value, U64 rmvAmt);
	template<FloatingPoint Arg> U64 InsertHex(T* str, const Arg& value, U64 rmvAmt);
	template<Pointer Arg> U64 InsertHex(T* str, const Arg& value, U64 rmvAmt);

	template<typename Arg>
	void Format(U64& start, const Arg& value);

	U64 Length(const T* str) const;
	void Copy(T* dst, const T* src, U64 length);
	bool Compare(const T* a, const T* b, U64 length) const;
	bool WhiteSpace(T c) const;
	bool NotWhiteSpace(T c) const;

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
inline StringBase<T, LU>::StringBase(const Arg& value) noexcept { ToString(string, value); }

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>::StringBase(const Arg& value, Hex flag) noexcept { HexToString(string, value); }

template<typename T, typename LU>
template<typename... Args>
inline StringBase<T, LU>::StringBase(const T* fmt, const Args& ... args) noexcept : size{ Length(fmt) }, capacity{ size }
{
	Memory::AllocateArray(&string, capacity);

	Copy(string, (T*)fmt, size + 1);
	U64 start = 0;
	(Format(start, args), ...);
}

template<typename T, typename LU>
template<typename... Args>
inline StringBase<T, LU>::StringBase(const Args& ... args) noexcept
{
	Memory::AllocateArray(&string, capacity);
	(ToString(string + size, args), ...);
}

template<typename T, typename LU>
template<typename Arg>
inline StringBase<T, LU>& StringBase<T, LU>::operator=(const Arg& value) noexcept
{
	ToString(string, value);
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
		Memory::Reallocate(&string, capacity = size);
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
	if (capacity < size + addLength + 1) { Memory::Reallocate(&string, capacity = size + addLength + 1); }
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
	if (capacity < size + addLength + 1) { Memory::Reallocate(&string, capacity = size + addLength + 1); }
	Copy(string + size, other, addLength);
	size += addLength;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::operator+=(const StringBase<T, LU>& other)
{
	hashed = false;
	if (capacity < size + other.size + 1) { Memory::Reallocate(&string, capacity = size + other.size + 1); }
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
char* StringBase<T, LU>::CStr() { return (char*)string; }

template<typename T, typename LU>
const char* StringBase<T, LU>::CStr() const { return (const char*)string; }

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
inline StringBase<T, LU>& StringBase<T, LU>::Append(const StringBase<T, LU>& append)
{
	if (capacity < size + append.size + 1) { Memory::Reallocate(&string, capacity = size + append.size + 1); }
	hashed = false;
	Copy(string + size, append.string, append.size);
	size += append.size;
	string[size] = LU::NULL_CHAR;

	return *this;
}

template<typename T, typename LU>
inline StringBase<T, LU>& StringBase<T, LU>::Prepend(const StringBase<T, LU>& prepend)
{
	if (capacity < size + prepend.size + 1) { Memory::Reallocate(&string, capacity = size + prepend.size + 1); }
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
	if (capacity < size + append.size + prepend.size + 1) { Memory::Reallocate(&string, capacity = size + append.size + prepend.size + 1); }
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
	if (capacity < size + other.size + 1) { Memory::Reallocate(&string, capacity = size + other.size + 1); }
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
	if (capacity < i + other.size + 2) { Memory::Reallocate(&string, capacity = i + other.size + 2); }
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
			if (capacity < size - find.size + replace.size + 1) { Memory::Reallocate(&string, capacity = size - find.size + replace.size + 1); }

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
	if (capacity < size + replace.size * count - find.size * count + 1) { Memory::Reallocate(&string, capacity = size + replace.size * count - find.size * count + 1); }

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
	if (capacity < size + replace.size - find.size + 1) { Memory::Reallocate(&string, capacity = size + replace.size - find.size + 1); }

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
	if (nLength < U64_MAX) { newStr.Resize(nLength); }
	else { newStr.Resize(size - start); }

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
template<Signed Arg>
inline void StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 21) { Memory::Reallocate(&string, capacity = size + 21); }

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
template<Unsigned Arg>
inline void StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 21) { Memory::Reallocate(&string, capacity = size + 21); }

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
template<Boolean Arg>
inline void StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (value)
	{
		if (!string || capacity < size + 5) { Memory::Reallocate(&string, capacity = size + 5); }
		Copy(str + size, LU::TRUE_STR, 4);
		size += 4;
		str[size] = LU::NULL_CHAR;
	}
	else
	{
		if (!string || capacity < size + 6) { Memory::Reallocate(&string, capacity = size + 6); }
		Copy(str + size, LU::FALSE_STR, 5);
		size += 5;
		str[size] = LU::NULL_CHAR;
	}
}

template<typename T, typename LU>
template<FloatingPoint Arg>
inline void StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 28) { Memory::Reallocate(&string, capacity = size + 28); }

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
template<Pointer Arg>
inline void StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	ToString(str, (U64)value);
}

template<typename T, typename LU>
template<Character Arg>
inline void StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 2) { Memory::Reallocate(&string, capacity = size + 2); }

	*str = value;
	str[++size] = LU::NULL_CHAR;
}

template<typename T, typename LU>
template<StringLiteral Arg>
inline void StringBase<T, LU>::ToString(T* str, const Arg& value)
{
	//TODO
}

template<typename T, typename LU>
template<Signed Arg>
inline void StringBase<T, LU>::HexToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 17) { Memory::Reallocate(&string, capacity = size + 17); }

	U8 pairs;
	U8 digits;
	U64 max;

	if constexpr (IsSame<RemovedQuals<Arg>, I8>) { if (!string || capacity < size + 3) { Memory::Reallocate(&string, capacity = size + 3); } pairs = 1; digits = 2; max = U8_MAX; }
	else if constexpr (IsSame<RemovedQuals<Arg>, I16>) { if (!string || capacity < size + 5) { Memory::Reallocate(&string, capacity = size + 5); } pairs = 2; digits = 4; max = U16_MAX; }
	else if constexpr (IsSame<RemovedQuals<Arg>, I32>) { if (!string || capacity < size + 9) { Memory::Reallocate(&string, capacity = size + 9); } pairs = 4; digits = 8; max = U32_MAX; }
	else if constexpr (IsSame<RemovedQuals<Arg>, L32>) { if (!string || capacity < size + 9) { Memory::Reallocate(&string, capacity = size + 9); } pairs = 4; digits = 8; max = U32_MAX; }
	else { if (!string || capacity < size + 17) { Memory::Reallocate(&string, capacity = size + 17); } pairs = 8; digits = 16; max = U64_MAX; }

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
template<Unsigned Arg>
inline void StringBase<T, LU>::HexToString(T* str, const Arg& value)
{
	hashed = false;

	U8 pairs;
	U8 digits;
	if constexpr (IsSame<RemovedQuals<Arg>, U8>) { if (!string || capacity < size + 3) { Memory::Reallocate(&string, capacity = size + 3); } pairs = 1; digits = 2; }
	else if constexpr (IsSame<RemovedQuals<Arg>, U16>) { if (!string || capacity < size + 5) { Memory::Reallocate(&string, capacity = size + 5); } pairs = 2; digits = 4; }
	else if constexpr (IsSame<RemovedQuals<Arg>, U32>) { if (!string || capacity < size + 9) { Memory::Reallocate(&string, capacity = size + 9); } pairs = 4; digits = 8; }
	else if constexpr (IsSame<RemovedQuals<Arg>, UL32>) { if (!string || capacity < size + 9) { Memory::Reallocate(&string, capacity = size + 9); } pairs = 4; digits = 8; }
	else { if (!string || capacity < size + 17) { Memory::Reallocate(&string, capacity = size + 17); } pairs = 8; digits = 16; }

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
template<FloatingPoint Arg>
inline void StringBase<T, LU>::HexToString(T* str, const Arg& value)
{
	hashed = false;
	if (!string || capacity < size + 17) { Memory::Reallocate(&string, capacity = size + 17); }

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
template<Signed Arg>
inline Arg StringBase<T, LU>::ToType(U64 start) const
{
	T* it = string + start;
	T c;
	Arg value = 0;

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
inline Arg StringBase<T, LU>::ToType(U64 start) const
{
	T* it = string + start;
	T c;
	Arg value = 0;

	while (NotWhiteSpace(c = *it++) && c != LU::NULL_CHAR) { value *= 10; value += c - LU::ZERO_CHAR; }

	return value;
}

template<typename T, typename LU>
template<Boolean Arg>
inline Arg StringBase<T, LU>::ToType(U64 start) const
{
	return Compare(string + start, LU::TRUE_STR, 4);
}

template<typename T, typename LU>
template<FloatingPoint Arg>
inline Arg StringBase<T, LU>::ToType(U64 start) const
{
	T* it = string + start;
	T c;
	Arg value = 0.0f;
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
template<Pointer Arg>
inline Arg StringBase<T, LU>::ToType(U64 start) const
{
	return (Arg)ToType<U64>(start);
}

template<typename T, typename LU>
inline U64 StringBase<T, LU>::Length(const T* str) const
{
	T* ptr = str;
	while (*ptr++);
	return ptr - str;
}

template<typename T, typename LU>
inline void StringBase<T, LU>::Copy(T* dst, const T* src, U64 length)
{
	memcpy(dst, src, length * sizeof(T));
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::Compare(const T* a, const T* b, U64 length) const
{
	T* c0 = a;
	T* c1 = b;

	T c;

	while ((c = *c0++) == *c1++ && --length) { if (c == LU::NULL_CHAR) { return true; } }

	return false;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::WhiteSpace(T c) const
{
	return c == LU::SPACE || c == LU::HTAB || c == LU::VTAB || c == LU::NEW_LINE || c == LU::RETURN || c == LU::FEED;
}

template<typename T, typename LU>
inline bool StringBase<T, LU>::NotWhiteSpace(T c) const
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
			case LU::CLOSE_BRACE: {
				start = InsertType(it - 2, value, 2);
			} break;
			case LU::FMT_HEX: {
				if (*it == LU::CLOSE_BRACE) { start = InsertHex(it - 2, value, 3); }
				else { /*Logger::Error("Invalid hex token, must be '{h}'!");*/ start = it - string; }
			} break;
			case LU::FMT_DEC: {
				if constexpr (IsSameNoQuals<Arg, F32> || IsSameNoQuals<Arg, F64>)
				{
					if (*it == LU::CLOSE_BRACE) { start = InsertType(it - 3, value, 4); }
					else if (it[1] == LU::CLOSE_BRACE) { start = InsertType(it - 3, value, 4, *it++); }
					else { /*Logger::Error("Invalid decimal token, must be '{.n}' where n is one digit!");*/ start = it - string; }
				}
				else { /*Logger::Error("Decimal token can only be used with float types!");*/ start = it - string; }
			} break;
			default: {
				//Logger::Error("Unknown format option '{}'!", c);
				start = it - string;
			}
			}
		}
	}
}

template<typename T, typename LU>
template<Signed Arg>
inline U64 StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	if (capacity < size + 21 - rmvAmt) { Memory::Reallocate(&string, capacity = size + 21 - rmvAmt); }

	Copy(str + 20, str + rmvAmt, size - rmvAmt - (str - string));

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

	return (str - string) + addLength + 1;
}

template<typename T, typename LU>
template<Unsigned Arg>
inline U64 StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	if (capacity < size + 21 - rmvAmt) { Memory::Reallocate(&string, capacity = size + 21 - rmvAmt); }

	Copy(str + 20, str + rmvAmt, size - rmvAmt - (str - string));

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

	return (str - string) + addLength + 1;
}

template<typename T, typename LU>
template<Boolean Arg>
inline U64 StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	if (value)
	{
		if (capacity < size + 5 - rmvAmt) { Memory::Reallocate(&string, capacity = size + 5 - rmvAmt); }

		Copy(str + 4, str + rmvAmt, size - rmvAmt - (str - string));
		Copy(str + size, LU::TRUE_STR, 4);
		size += 4;
		str[size] = LU::NULL_CHAR;

		return (str - string) + 5;
	}
	else
	{
		if (capacity < size + 6 - rmvAmt) { Memory::Reallocate(&string, capacity = size + 6 - rmvAmt); }

		Copy(str + 5, str + rmvAmt, size - rmvAmt - (str - string));
		Copy(str + size, LU::FALSE_STR, 5);
		size += 5;
		str[size] = LU::NULL_CHAR;

		return (str - string) + 6;
	}
}

template<typename T, typename LU>
template<FloatingPoint Arg>
inline U64 StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt, U64 decimalCount)
{
	if (capacity < size + 22 + decimalCount) { Memory::Reallocate(&string, capacity = size + 22 + decimalCount); }

	Copy(str + 21 + decimalCount, str + rmvAmt, size - rmvAmt - (str - string));
	T* c = str + 21 + decimalCount;
	const T* threeDigits;
	U8 neg = 0;

	F64 abs = value;

	if (value < 0)
	{
		str[0] = LU::NEGATIVE_CHAR;
		abs = -value;
		neg = 1;
	}

	//TODO: use decimalCount
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

	U64 addLength = 21 + decimalCount + neg - (c - str);
	size += addLength;

	Copy(str + neg, c, addLength);
	str[size] = LU::NULL_CHAR;

	return (str - string) + addLength + 1;
}

template<typename T, typename LU>
template<Pointer Arg>
inline U64 StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	return InsertType(str, (U64)value, rmvAmt);
}

template<typename T, typename LU>
template<Character Arg>
inline U64 StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<StringLiteral Arg>
inline U64 StringBase<T, LU>::InsertType(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<Signed Arg>
inline U64 StringBase<T, LU>::InsertHex(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<Unsigned Arg>
inline U64 StringBase<T, LU>::InsertHex(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<FloatingPoint Arg>
inline U64 StringBase<T, LU>::InsertHex(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}

template<typename T, typename LU>
template<Pointer Arg>
inline U64 StringBase<T, LU>::InsertHex(T* str, const Arg& value, U64 rmvAmt)
{
	//TODO
}