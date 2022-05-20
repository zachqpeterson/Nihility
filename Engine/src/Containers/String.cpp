#include "String.hpp"

#include "Memory/Memory.hpp"
#include "Vector.hpp"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef _MSC_VER
#include <strings.h>
#endif

String::String(char* str)
{
    if (str == nullptr)
    {
        this->str = (char*)Memory::Allocate(1, MEMORY_TAG_ENTITY);
        this->str[0] = '\0';
        length = 0;
    }
    else
    {
        length = strlen(str);
        this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_ENTITY);
        Memory::Copy(this->str, str, length);
        this->str[length] = '\0';
    }
}

String::String(const char* str)
{
    if (str == nullptr)
    {
        this->str = (char*)Memory::Allocate(1, MEMORY_TAG_ENTITY);
        this->str[0] = '\0';
        length = 0;
    }
    else
    {
        length = strlen(str);
        this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_ENTITY);
        Memory::Copy(this->str, str, length);
        this->str[length] = '\0';
    }
}

String::String(const String& other)
{
    if (other.str == nullptr)
    {
        str = (char*)Memory::Allocate(1, MEMORY_TAG_ENTITY);
        str[0] = '\0';
        length = 0;
    }
    else
    {
        length = strlen(other.str);
        str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_ENTITY);
        Memory::Copy(str, other.str, length);
        str[length] = '\0';
    }
}

String::String(String&& other)
{
    str = other.str;
    other.str = nullptr;
    length = other.length;
    other.length = 0;
}

String::~String()
{
    if (str)
    {
        Memory::Free(str, length + 1, MEMORY_TAG_ENTITY);
        str = nullptr;
        length = 0;
    }

    str = nullptr;
}

void String::Destroy()
{
    if (str)
    {
        Memory::Free(str, length + 1, MEMORY_TAG_ENTITY);
        str = nullptr;
        length = 0;
    }

    str = nullptr;
}

String& String::operator=(char* str)
{
    if (str == this->str) { return *this; }

    if (this->str)
    {
        Memory::Free(this->str, length + 1, MEMORY_TAG_ENTITY);
    }

    if (str == nullptr)
    {
        this->str = (char*)Memory::Allocate(1, MEMORY_TAG_ENTITY);
        this->str[0] = '\0';
        length = 0;
    }
    else
    {
        length = strlen(str);
        this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_ENTITY);
        Memory::Copy(this->str, str, length);
        this->str[length] = '\0';
    }

    return *this;
}

String& String::operator=(const char* str)
{
    if (str == this->str) { return *this; }

    if (this->str)
    {
        Memory::Free(this->str, length + 1, MEMORY_TAG_ENTITY);
    }

    if (str == nullptr)
    {
        this->str = (char*)Memory::Allocate(1, MEMORY_TAG_ENTITY);
        this->str[0] = '\0';
        length = 0;
    }
    else
    {
        length = strlen(str);
        this->str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_ENTITY);
        Memory::Copy(this->str, str, length);
        this->str[length] = '\0';
    }

    return *this;
}

String& String::operator=(const String& other)
{
    if (str == other.str) { return *this; }

    if (this->str)
    {
        Memory::Free(this->str, length + 1, MEMORY_TAG_ENTITY);
    }

    if (other.str == nullptr)
    {
        str = (char*)Memory::Allocate(1, MEMORY_TAG_ENTITY);
        str[0] = '\0';
        length = 0;
    }
    else
    {
        length = strlen(other.str);
        str = (char*)Memory::Allocate(length + 1, MEMORY_TAG_ENTITY);
        Memory::Copy(str, other.str, length);
        str[length] = '\0';
    }

    return *this;
}

String& String::operator=(String&& other)
{
    if (str == other.str) { return *this; }

    if (this->str)
    {
        Memory::Free(this->str, length + 1, MEMORY_TAG_ENTITY);
    }

    str = other.str;
    other.str = nullptr;
    length = other.length;
    other.length = 0;

    return *this;
}

U64& String::Length()
{
    return length;
}

const U64& String::Length() const
{
    return length;
}

String String::Duplicate() const
{
    if (str)
    {
        char* copy = (char*)Memory::Allocate(length + 1, MEMORY_TAG_ENTITY);
        Memory::Copy(copy, str, length);
        copy[length] = '\0';
        return copy;
    }

    return {};
}

String String::NDuplicate(U64 length) const
{
    if (str && length < this->length)
    {
        char* copy = (char*)Memory::Allocate(length + 1, MEMORY_TAG_ENTITY);
        Memory::Copy(copy, str, length);
        copy[length] = '\0';
        return copy;
    }

    return {};
}

bool String::Equals(const String& str) const
{
    return strcmp(this->str, str) == 0;
}

bool String::EqualsI(const String& str) const
{
#if defined(__GNUC__)
    return strcasecmp(this->str, str) == 0;
#elif (defined _MSC_VER)
    return _strcmpi(this->str, str) == 0;
#endif
}

bool String::NEquals(const String& str, U64 length) const
{
    return strncmp(this->str, str, length);
}

bool String::NEqualsI(const String& str, U64 length) const
{
#if defined(__GNUC__)
    return strncasecmp(this->str, str, length) == 0;
#elif (defined _MSC_VER)
    return _strnicmp(this->str, str, length) == 0;
#endif
}

I32 String::Format(const char* format, ...)
{
    if (format)
    {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        I32 written = FormatV(format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }

    return -1;
}

I32 String::FormatV(const char* format, va_list vaList)
{
    if (format)
    {
        char buffer[32000];
        I32 written = vsnprintf(buffer, 32000, format, vaList);
        buffer[written] = '\0';

        char* newStr = (char*)Memory::Allocate(written + 1, MEMORY_TAG_ENTITY);
        length = written;
        Memory::Copy(newStr, buffer, length + 1);
        Memory::Free(str, length + 1, MEMORY_TAG_ENTITY);
        str = newStr;

        return written;
    }

    return -1;
}

void String::Empty()
{
    if (str)
    {
        Memory::Free(str, length + 1, MEMORY_TAG_ENTITY);
        str = nullptr;
        length = 0;
    }
}

void String::Trim()
{
    //TODO: 
}

String String::SubString(U64 start, U64 length) const
{
    //TODO:
    return {};
}

U64 String::IndexOf(char c) const
{
    if (!str)
    {
        for (U64 i = 0; i < length; ++i)
        {
            if (str[i] == c) { return i; }
        }
    }

    return -1;
}

Vector<String> String::Split(char delimiter) const
{
    //TODO:
    return {};
}

void String::Append(const String& append)
{
    if (str && append.str)
    {
        U64 newLength = length + append.length;

        char* newStr = (char*)Memory::Allocate(newLength + 1, MEMORY_TAG_ENTITY);

        Memory::Copy(newStr, str, length);
        Memory::Copy(newStr + length, append.str, append.length);
        newStr[newLength] = '\0';

        Memory::Free(str, length + 1, MEMORY_TAG_ENTITY);
        str = newStr;
        length = newLength;
    }
}

bool String::operator==(const String& other) const
{
    if (length == other.length)
    {
        for (U64 i = 0; i < length; ++i)
        {
            if (str[i] != other.str[i])
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
    if (length != other.length)
    {
        for (U64 i = 0; i < length; ++i)
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