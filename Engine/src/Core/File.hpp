#pragma once

#include "Defines.hpp"

enum FileMode
{
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2,
};

template<typename> struct Vector;

struct NH_API File
{
public:
    File() : handle{ nullptr } {}

    bool Open(const struct String& path, FileMode mode, bool binary);
    void Close();
    U64 Size();
    bool ReadLine(struct String& line, U64 maxLength = 512);
    bool WriteLine(const struct String& str);
    struct String Read(U64 length);
    U8* ReadBytes(U64 length);
    U8* ReadAllBytes(U64& size);
    struct String ReadAllText();
    void Write(const struct String& str);
    void Seek(U64 length);

    static Vector<struct String> GetAllFiles(const struct String& dir);

public:
    void* handle;
};