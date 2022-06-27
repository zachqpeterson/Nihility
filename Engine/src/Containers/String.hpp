#pragma once

#include "Defines.hpp"

template<typename> struct Vector;

struct String
{
    //TODO: Constructor that formats
    NH_API String() : str{ nullptr }, length{ 0 } {}
    NH_API String(char* str);
    NH_API String(const char* str);
    NH_API String(const String& other);
    NH_API String(String&& other);

    NH_API ~String();
    NH_API void Destroy();

    NH_API void* operator new(U64 size);
    NH_API void operator delete(void* ptr);

    NH_API String(I32 value);
    NH_API String(I64 value);
    NH_API String(U32 value);
    NH_API String(U64 value);
    NH_API String(F32 value);
    NH_API String(F64 value);
    NH_API String(bool value);

    NH_API String& operator=(char* str);
    NH_API String& operator=(const char* str);
    NH_API String& operator=(const String& other);
    NH_API String& operator=(String&& other);

    NH_API String& operator=(I32 value);
    NH_API String& operator=(I64 value);
    NH_API String& operator=(U32 value);
    NH_API String& operator=(U64 value);
    NH_API String& operator=(F32 value);
    NH_API String& operator=(F64 value);
    NH_API String& operator=(bool value);

    NH_API U64& Length();
    NH_API const U64& Length() const;
    NH_API String Duplicate() const;
    NH_API String NDuplicate(U64 length) const;
    NH_API bool Equals(const String& str) const;
    NH_API bool EqualsI(const String& str) const;
    NH_API bool NEquals(const String& str, U64 length) const;
    NH_API bool NEqualsI(const String& str, U64 length) const;
    NH_API bool Blank() const;
    NH_API void Empty();
    NH_API String& Trim();
    NH_API String SubString(U64 start, U64 length = I64_MAX) const;
    NH_API I64 IndexOf(char c, U64 start = 0) const;
    NH_API Vector<String> Split(char delimiter, bool trimEntries) const;
    NH_API String& Append(const String& append);
    NH_API String& Prepend(const String& prepend);
    NH_API String& Surround(const String& prepend, const String& append);
    NH_API String& Insert(const String& string, U64 i);
    NH_API String& Replace(const String& find, const String& replace, U64 start = 0);
    NH_API String& ReplaceFirst(const String& find, const String& replace, U64 start = 0);

    NH_API I8  ToI8 ();
    NH_API I16 ToI16();
    NH_API I32 ToI32();
    NH_API I64 ToI64();
    NH_API U8  ToU8 ();
    NH_API U16 ToU16();
    NH_API U32 ToU32();
    NH_API U64 ToU64();
    NH_API F32 ToF32();
    NH_API F64 ToF64();
    NH_API bool ToBool();

    NH_API operator const char* () const { return str; }
    NH_API operator char* () { return str; }
    NH_API const char* operator*() const { return str; }
    NH_API char* operator*() { return str; }
    NH_API char& operator[](U64 i) { return str[i]; }
    NH_API const char& operator[](U64 i) const { return str[i]; }

    NH_API bool operator==(const String& other) const;
    NH_API bool operator==(const char* other) const;
    NH_API bool operator==(char* other) const;
    NH_API bool operator!=(const String& other) const;
    NH_API bool operator!=(const char* other) const;
    NH_API bool operator!=(char* other) const;
    
    template<typename... Types>
    NH_API void Format(const Types& ... args)
    {
        (ReplaceFirst("{}", args), ...);
    }

private:
    char* str;
    U64 length;
};