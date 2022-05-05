#pragma once

#include "Defines.hpp"

enum FileMode
{
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2,
};

struct File
{
public:
    NH_API File() : handle{ nullptr } {}

    NH_API bool Open(const struct String& path, FileMode mode, bool binary);
    NH_API void Close();
    NH_API U64 Size();
    NH_API struct String ReadLine(U64 maxLength = U64_MAX);
    NH_API bool WriteLine(const struct String& str);
    NH_API struct String Read(U64 length);
    //NH_API Vector<U8> ReadAllBytes();
    NH_API struct String ReadAllText();
    NH_API void Write(const struct String& str);

public:
    void* handle;
};