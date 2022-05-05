#include "String.hpp"

#include "Memory/Memory.hpp"

#include <string.h>
#include <stdio.h>
#include <ctype.h>  // isspace

#ifndef _MSC_VER
#include <strings.h>
#endif

U64 String::Length()
{
    return strlen(str);
}

const U64 String::Length() const
{
    return strlen(str);
}

String String::Duplicate()
{
    U64 length = Length();
    char* copy = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
    Memory::CopyMemory(copy, str, length);
    copy[length] = 0;
    return copy;
}

String String::NDuplicate(U64 length)
{
    char* copy = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
    Memory::CopyMemory(copy, str, length);
    copy[length] = 0;
    return copy;
}

bool String::Equals(const String& str)
{
    return strcmp(this->str, str) == 0;
}

bool String::EqualsI(const String& str)
{
#if defined(__GNUC__)
    return strcasecmp(this->str, str) == 0;
#elif (defined _MSC_VER)
    return _strcmpi(this->str, str) == 0;
#endif
}

bool String::NEquals(const String& str, U64 length)
{
    return strncmp(this->str, str, length);
}

bool String::NEqualsI(const String& str, U64 length)
{
#if defined(__GNUC__)
    return strncasecmp(this->str, str, length) == 0;
#elif (defined _MSC_VER)
    return _strnicmp(this->str, str, length) == 0;
#endif
}

I32 String::Format(const char* format, ...)
{
    if (str) {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        //NOTE: Ignore this error
        I32 written = FormatV(format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }

    return -1;
}

I32 String::FormatV(const char* format, va_list vaList)
{
    if (str) {
        // Big, but can fit on the stack.
        char buffer[32000];
        I32 written = vsnprintf(buffer, 32000, format, vaList);
        buffer[written] = 0;
        Memory::CopyMemory(str, buffer, written + 1);
        return written;
    }

    return -1;
}

bool String::Empty()
{
    if (str) {
        str[0] = 0;
    }

    return str;
}

void String::Trim()
{
    //TODO: Write helper for this
    while (isspace((unsigned char)*str)) { str++; }
    if (*str) {
        char* p = str;
        while (*p) {
            p++;
        }
        while (isspace((unsigned char)*(--p)));

        p[1] = '\0';
    }
}

String String::SubString(U64 start, U64 length)
{
    if (length == 0) { return String(); }

    char* dest = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);

    U64 srcLength = Length();
    if (start >= srcLength) {
        dest[0] = 0;
        return dest;
    }
    if (length > 0) {
        for (U64 i = start, j = 0; j < length && str[i]; ++i, ++j) {
            dest[j] = str[i];
        }
        dest[start + length] = 0;
    }
    else {
        // If a negative value is passed, proceed to the end of the string.
        U64 j = 0;
        for (U64 i = start; str[i]; ++i, ++j) {
            dest[j] = str[i];
        }
        dest[start + j] = 0;
    }

    return dest;
}

I32 String::IndexOf(char c)
{
    if (!str) {
        return -1;
    }
    U32 length = Length();
    if (length > 0) {
        for (U32 i = 0; i < length; ++i) {
            if (str[i] == c) {
                return i;
            }
        }
    }

    return -1;
}

U32 String::Split(char delimiter, char*** str_darray)
{
    //TODO:
    return -1;
}

void String::Append(const String& append)
{
    sprintf(str, "%s%s", str, (const char*)append);
}