#include "File.hpp"

#include "Containers/String.hpp"
#include "Core/Logger.hpp"
#include "Memory/Memory.hpp"

#include <stdio.h>

bool File::Open(const String& path, FileMode mode, bool binary)
{
    handle = nullptr;
    const char* modeStr;

    if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
        modeStr = binary ? "w+b" : "w+";
    }
    else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
        modeStr = binary ? "rb" : "r";
    }
    else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
        modeStr = binary ? "wb" : "w";
    }
    else {
        LOG_ERROR("Invalid mode passed while trying to open file: '%s'", (const char*)path);
        return false;
    }

    FILE* file = fopen(path, modeStr);
    if (!file) {
        LOG_ERROR("Error opening file: '%s'", (const char*)path);
        return false;
    }

    handle = file;

    return true;
}

void File::Close()
{
    if (handle) {
        fclose((FILE*)handle);
        handle = nullptr;
    }
}

U64 File::Size()
{
    if (handle) {
        fseek((FILE*)handle, 0, SEEK_END);
        U64 size = ftell((FILE*)handle);
        rewind((FILE*)handle);
        return size;
    }

    return -1;
}

String File::ReadLine(U64 maxLength)
{
    if (handle && maxLength > 0) {
        char* buf = (char*)Memory::Allocate(sizeof(char) * maxLength, MEMORY_TAG_DATA_STRUCT);
        if (fgets(buf, maxLength, (FILE*)handle) != 0) {
            String s;
            s.Append(buf);
            Memory::Free(buf, maxLength, MEMORY_TAG_DATA_STRUCT);
            return s;
        }
    }

    return String();
}

bool File::WriteLine(const String& str)
{
    if (handle) {
        I32 result = fputs(str, (FILE*)handle);
        if (result != EOF) {
            result = fputc('\n', (FILE*)handle);
        }

        fflush((FILE*)handle);
        return result != EOF;
    }

    return false;
}

String File::Read(U64 length)
{
    if (handle) {
        char* buf = (char*)Memory::Allocate(sizeof(char) * length, MEMORY_TAG_DATA_STRUCT);
        fread(buf, 1, length, (FILE*)handle);
        return String(buf);
    }

    return String();
}

U8* File::ReadBytes(U64 length)
{
    if (handle)
    {
        U8* buf = (U8*)Memory::Allocate(sizeof(U8) * length, MEMORY_TAG_DATA_STRUCT);
        fread(buf, 1, length, (FILE*)handle);
        return buf;
    }

    return nullptr;
}

U8* File::ReadAllBytes(U64& size)
{
    if (handle)
    {
        size = Size();
        U8* data = (U8*)Memory::Allocate(sizeof(U8) * size, MEMORY_TAG_DATA_STRUCT);

        fread(data, 1, size, (FILE*)handle);
        return data;
    }

    return nullptr;
}

String File::ReadAllText()
{
    if (handle) 
    {
        U64 size = Size();
        char* buf = (char*)Memory::Allocate(sizeof(char) * size, MEMORY_TAG_DATA_STRUCT);

        fread(buf, 1, size, (FILE*)handle);
        return String(buf);
    }

    return String();
}

void File::Write(const String& str)
{
    if (handle) {
        fwrite(str, 1, str.Length(), (FILE*)handle);
        fflush((FILE*)handle);
    }
}

void File::Seek(U64 length)
{
    if(handle)
    {
        fseek((FILE*)handle, length, SEEK_SET);
    }
}