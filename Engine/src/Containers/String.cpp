#include "String.hpp"

#include "Memory/Memory.hpp"

#include <string.h>
#include <stdio.h>
#include <ctype.h>  // isspace

#ifndef _MSC_VER
#include <strings.h>
#endif

String::String() : str{ nullptr }
{
    str = (char*)Memory::Allocate(1, MEMORY_TAG_DATA_STRUCT);
    str[0] = '\0';
}

String::String(char* str)
{
    if (str == nullptr)
    {
        this->str = (char*)Memory::Allocate(1, MEMORY_TAG_DATA_STRUCT);
        this->str[0] = '\0';
    }
    else
    {
        U64 length = strlen(str);
        this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(this->str, str, length);
        this->str[length] = '\0';
    }
}

String::String(const char* str)
{
    if (str == nullptr)
    {
        this->str = (char*)Memory::Allocate(1, MEMORY_TAG_DATA_STRUCT);
        this->str[0] = '\0';
    }
    else
    {
        U64 length = strlen(str);
        this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(this->str, str, length);
        this->str[length] = '\0';
    }
}

String::String(const String& other)
{
    if (other.str == nullptr)
    {
        str = (char*)Memory::Allocate(1, MEMORY_TAG_DATA_STRUCT);
        str[0] = '\0';
    }
    else
    {
        U64 length = strlen(other.str);
        str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(str, other.str, length);
        str[length] = '\0';
    }
}

String::String(String&& other)
{
    str = other.str;
    other.str = nullptr;
}

String::~String()
{
    if (str)
    {
        Memory::Free(str, Length() + 1, MEMORY_TAG_DATA_STRUCT);
        str = nullptr;
    }
}

String& String::operator=(char* str)
{
    if(str == this->str)
    {
        return *this;
    }

    if(this->str)
    {
        Memory::Free(this->str, Length() + 1, MEMORY_TAG_DATA_STRUCT);
    }

    if (str == nullptr)
    {
        this->str = (char*)Memory::Allocate(1, MEMORY_TAG_DATA_STRUCT);
        this->str[0] = '\0';
    }
    else
    {
        U64 length = strlen(str);
        this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(this->str, str, length);
        this->str[length] = '\0';
    }

    return *this;
}

String& String::operator=(const char* str)
{
    if (str == this->str)
    {
        return *this;
    }

    if (this->str)
    {
        Memory::Free(this->str, Length() + 1, MEMORY_TAG_DATA_STRUCT);
    }

    if (str == nullptr)
    {
        this->str = (char*)Memory::Allocate(1, MEMORY_TAG_DATA_STRUCT);
        this->str[0] = '\0';
    }
    else
    {
        U64 length = strlen(str);
        this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(this->str, str, length);
        this->str[length] = '\0';
    }

    return *this;
}

String& String::operator=(const String& other)
{
    if (str == other.str)
    {
        return *this;
    }

    if (this->str)
    {
        Memory::Free(this->str, Length() + 1, MEMORY_TAG_DATA_STRUCT);
    }

    if (other.str == nullptr)
    {
        str = (char*)Memory::Allocate(1, MEMORY_TAG_DATA_STRUCT);
        str[0] = '\0';
    }
    else
    {
        U64 length = strlen(other.str);
        str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(str, other.str, length);
        str[length] = '\0';
    }

    return *this;
}

String& String::operator=(String&& other)
{
    if (str == other.str)
    {
        return *this;
    }

    if (this->str)
    {
        Memory::Free(this->str, Length() + 1, MEMORY_TAG_DATA_STRUCT);
    }

    str = other.str;
    other.str = nullptr;

    return *this;
}

U64 String::Length()
{
    if(str)
    {
        return strlen(str);
    }

    return 0;
}

const U64 String::Length() const
{
    if(str)
    {
        return strlen(str);
    }

    return 0;
}

String String::Duplicate()
{
    if (str)
    {
        U64 length = Length();
        char* copy = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(copy, str, length);
        copy[length] = 0;
        return copy;
    }

    return {};
}

String String::NDuplicate(U64 length)
{
    if (str && length < Length())
    {
        char* copy = (char*)Memory::Allocate(length + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(copy, str, length);
        copy[length] = 0;
        return copy;
    }

    return {};
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

        char* newStr = (char*)Memory::Allocate(written + 1, MEMORY_TAG_DATA_STRUCT);
        Memory::Copy(newStr, buffer, written + 1);
        Memory::Free(str, Length() + 1, MEMORY_TAG_DATA_STRUCT);
        str = newStr;
        
        return written;
    }

    return -1;
}

void String::Empty()
{
    if (str) {
        str[0] = 0;
    }
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

U64 String::IndexOf(char c)
{
    if (!str) {
        return -1;
    }
    U64 length = Length();
    if (length > 0) {
        for (U64 i = 0; i < length; ++i) {
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
    //TODO: Check for null strings

    U64 length0 = Length();
    U64 length1 = append.Length();
    U64 newLength = length0 + length1;

    char* newStr = (char*)Memory::Allocate(newLength + 1, MEMORY_TAG_DATA_STRUCT);

    Memory::Copy(newStr, str, length0);
    Memory::Copy(newStr + length0, append.str, length1);
    newStr[newLength] = 0;

    Memory::Free(str, length0 + 1, MEMORY_TAG_DATA_STRUCT);
    str = newStr;
}

bool String::operator==(const String& other) const
{
    U64 l0 = Length();
    U64 l1 = other.Length();

    if(l0 == l1)
    {
        for(U64 i = 0; i < l0; ++i)
        {
            if(str[i] != other.str[i])
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool String::operator!=(const String& other) const
{
    U64 l0 = Length();
    U64 l1 = other.Length();

    if (l0 != l1)
    {
        for (U64 i = 0; i < l0; ++i)
        {
            if (str[i] == other.str[i])
            {
                return false;
            }
        }

        return true;
    }

    return false;
}