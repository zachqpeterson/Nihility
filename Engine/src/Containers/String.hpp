#pragma once

#include "Defines.hpp"

struct String
{
    String() : str{ nullptr } {}
    String(char* str) : str{ str } {}
    String(const char* str) : str{ (char*)str } {}
    String(const String& other) : str{ other.str } {}

    NH_API U64 Length();
    NH_API String Duplicate();
    NH_API bool Equals(const String& str);
    NH_API bool EqualsI(const String& str);
    NH_API bool NEquals(const String& str, U64 length);
    NH_API bool NEqualsI(const String& str, U64 length);
    NH_API String Format(const char* format, ...);
    NH_API String FormatV(const char* format, void* vaList);
    NH_API void Empty();
    NH_API String Copy();
    NH_API String NCopy(U64 length);
    NH_API void Trim();
    NH_API String SubString(U64 start, U64 length);
    NH_API I32 IndexOf(char c);
    NH_API U32 Split(char delimiter, char*** str_darray);
    NH_API void Append(const String& source);

    operator const char*() const { return str; }
    operator char*() { return str; }
    const char* operator*() const { return str; }

private:
    char* str;
};