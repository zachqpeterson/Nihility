#include "File.hpp"

#include "Containers/String.hpp"
#include "Core/Logger.hpp"

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
        char* buf = nullptr;
        if (fgets(buf, maxLength, (FILE*)handle) != 0) {
            return String(buf);
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
        char* buf = nullptr;
        fread(buf, 1, length, (FILE*)handle);
        return String(buf);
    }

    return String();
}

Vector<U8> File::ReadAllBytes()
{
    Vector<U8> data;

    if (handle)
    {
        U64 size = 0;
        if (!Size())
        {
            return data;
        }

        U8* buf = nullptr;
        fread(buf, 1, size, (FILE*)handle);
        data.SetArray(buf, size);
        return data;
    }

    return data;
}

String File::ReadAllText()
{
    if (handle) 
    {
        U64 size = 0;
        if (!Size()) {
            return String();
        }

        char* buf = nullptr;
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
