#pragma once

#include "Defines.hpp"

#include <stdarg.h>

struct String
{
    NH_API String();
    NH_API String(char* str);
    NH_API String(const char* str);
    NH_API String(const String& other);
    NH_API String(String&& other);
    NH_API ~String();

    NH_API String& operator=(char* str);
    NH_API String& operator=(const char* str);
    NH_API String& operator=(const String& other);
    NH_API String& operator=(String&& other);

    NH_API U64 Length();
    NH_API const U64 Length() const;
    NH_API String Duplicate();
    NH_API String NDuplicate(U64 length);
    NH_API bool Equals(const String& str);
    NH_API bool EqualsI(const String& str);
    NH_API bool NEquals(const String& str, U64 length);
    NH_API bool NEqualsI(const String& str, U64 length);
    NH_API I32 Format(const char* format, ...);
    NH_API I32 FormatV(const char* format, va_list vaList);
    NH_API void Empty();
    NH_API void Trim();
    NH_API String SubString(U64 start, U64 length);
    NH_API U64 IndexOf(char c);
    NH_API U32 Split(char delimiter, char*** str_darray);
    NH_API void Append(const String& append);

    NH_API operator const char* () const { return str; }
    NH_API operator char* () { return str; }
    NH_API const char* operator*() const { return str; }
    NH_API char& operator[](U64 i) { return str[i]; }
    NH_API const char& operator[](U64 i) const { return str[i]; }

    NH_API bool operator==(const String& other) const;
    NH_API bool operator!=(const String& other) const;

private:
    char* str;
};