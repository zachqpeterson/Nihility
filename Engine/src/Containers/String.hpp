#pragma once

#include "Defines.hpp"

#include <stdarg.h>

struct String
{
    String() : str{ nullptr } {}
    String(char* str) : str{ str } {}
    String(const char* str) : str{ (char*)str } {}
    String(const String& other) : str{ other.str } {}

    //TODO: Test these
    NH_API U64 Length();
    NH_API String Duplicate();
    NH_API String NDuplicate(U64 length);
    NH_API bool Equals(const String& str);
    NH_API bool EqualsI(const String& str);
    NH_API bool NEquals(const String& str, U64 length);
    NH_API bool NEqualsI(const String& str, U64 length);
    NH_API I32 Format(const char* format, ...);
    NH_API I32 FormatV(const char* format, va_list vaList);
    NH_API bool Empty();
    NH_API void Trim();
    NH_API String SubString(U64 start, U64 length);
    NH_API I32 IndexOf(char c);
    NH_API U32 Split(char delimiter, char*** str_darray);
    NH_API void Append(const String& append);

    operator const char*() const { return str; }
    operator char*() { return str; }
    const char* operator*() const { return str; }

private:
    char* str;
};