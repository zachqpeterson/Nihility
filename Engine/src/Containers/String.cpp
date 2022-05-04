#include "String.hpp"


U64 String::Length()
{
    return 0;
}

String String::Duplicate()
{
    return String();
}

bool String::Equals(const String& str)
{
    return false;
}

bool String::EqualsI(const String& str)
{
    return false;
}

bool String::NEquals(const String& str, U64 length)
{
    return false;
}

bool String::NEqualsI(const String& str, U64 length)
{
    return false;
}

String String::Format(const char* format, ...)
{
    return String();
}

String String::FormatV(const char* format, void* vaList)
{
    return String();
}

void String::Empty()
{
    
}

String String::Copy()
{
    return String();
}

String String::NCopy(U64 length)
{
    return String();
}

void String::Trim()
{
    
}

String String::SubString(U64 start, U64 length)
{
    return String();
}

I32 String::IndexOf(char c)
{
    return -1;
}

U32 String::Split(char delimiter, char*** str_darray)
{
    return -1;
}

void String::Append(const String& source)
{
    
}