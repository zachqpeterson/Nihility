#include "File.hpp"

#include "Containers/String.hpp"
#include <Containers/Vector.hpp>
#include "Core/Logger.hpp"
#include "Memory/Memory.hpp"

#include <stdio.h>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

bool File::Open(const String& path, FileMode mode, bool binary)
{
	handle = nullptr;
	const char* modeStr;

	if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0)
	{
		modeStr = binary ? "w+b" : "w+";
	}
	else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0)
	{
		modeStr = binary ? "rb" : "r";
	}
	else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0)
	{
		modeStr = binary ? "wb" : "w";
	}
	else
	{
		Logger::Error("Invalid mode passed while trying to open file: {}", path);
		return false;
	}

	FILE* file;
	fopen_s(&file, path, modeStr);
	if (!file)
	{
		Logger::Error("Error opening file: {}", path);
		return false;
	}

	handle = file;

	return true;
}

void File::Close()
{
	if (handle)
	{
		fclose((FILE*)handle);
		handle = nullptr;
	}
}

U64 File::Size()
{
	if (handle)
	{
		fseek((FILE*)handle, 0, SEEK_END);
		U64 size = ftell((FILE*)handle);
		rewind((FILE*)handle);
		return size;
	}

	return -1;
}

void File::Restart()
{
	rewind((FILE*)handle);
}

bool File::ReadLine(struct String& line, U64 maxLength)
{
	if (handle && maxLength > 0)
	{
		char buf[1024];

		if (fgets(buf, (I32)maxLength, (FILE*)handle))
		{
			line = buf;
			return true;
		}
	}

	return false;
}

bool File::WriteLine(const String& str)
{
	if (handle)
	{
		I32 result = fputs(str, (FILE*)handle);
		if (result != EOF)
		{
			result = fputc('\n', (FILE*)handle);
		}

		fflush((FILE*)handle);
		return result != EOF;
	}

	return false;
}

String File::Read(U64 length)
{
	if (handle)
	{
		char* buf = (char*)Memory::Allocate(sizeof(char) * length, MEMORY_TAG_RESOURCE);
		fread(buf, 1, length, (FILE*)handle);
		return String(buf);
		Memory::Free(buf, sizeof(char) * length, MEMORY_TAG_RESOURCE);
	}

	return String();
}

U8* File::ReadBytes(U64 length, I64 tag)
{
	if (handle)
	{
		U8* buf = (U8*)Memory::Allocate(sizeof(U8) * length, (MemoryTag)tag);
		fread(buf, 1, length, (FILE*)handle);
		return buf;
	}

	return nullptr;
}

U8* File::ReadAllBytes(U64& size, I64 tag)
{
	if (handle)
	{
		size = Size();
		U8* data = (U8*)Memory::Allocate(sizeof(U8) * size, (MemoryTag)tag);

		fread(data, 1, size, (FILE*)handle);
		return data;
	}

	return nullptr;
}

I8 File::ReadI8()
{
	if (handle)
	{
		I8 buf[1];
		fread(buf, 1, 1, (FILE*)handle);
		return buf[0];
	}

	return 0;
}

I16 File::ReadI16()
{
	if (handle)
	{
		I16 buf[1];
		fread(buf, 2, 1, (FILE*)handle);
		return buf[0];
	}

	return 0;
}

I32 File::ReadI32()
{
	if (handle)
	{
		I32 buf[1];
		fread(buf, 4, 1, (FILE*)handle);
		return buf[0];
	}

	return 0;
}

I64 File::ReadI64()
{
	if (handle)
	{
		I64 buf[1];
		fread(buf, 8, 1, (FILE*)handle);
		return buf[0];
	}

	return 0;
}

U8 File::ReadU8()
{
	if (handle)
	{
		U8 buf[1];
		fread(buf, 1, 1, (FILE*)handle);
		return buf[0];
	}

	return 0;
}

U16 File::ReadU16()
{
	if (handle)
	{
		U16 buf[1];
		fread(buf, 2, 1, (FILE*)handle);
		return buf[0];
	}

	return 0;
}

U32 File::ReadU32()
{
	if (handle)
	{
		U32 buf[1];
		fread(buf, 4, 1, (FILE*)handle);
		return buf[0];
	}

	return 0;
}

U64 File::ReadU64()
{
	if (handle)
	{
		U64 buf[1];
		fread(buf, 8, 1, (FILE*)handle);
		return buf[0];
	}

	return 0;
}

String File::ReadAllText()
{
	if (handle)
	{
		U64 size = Size();
		char* buf = (char*)Memory::Allocate(sizeof(char) * size, MEMORY_TAG_RESOURCE);

		fread(buf, 1, size, (FILE*)handle);
		return String(buf);
		Memory::Free(buf, sizeof(char) * size, MEMORY_TAG_RESOURCE);
	}

	return String();
}

void File::Write(const String& str)
{
	if (handle)
	{
		fwrite((const char*)str, 1, str.Length(), (FILE*)handle);
		fflush((FILE*)handle);
	}
}

void File::SeekFromStart(U64 length)
{
	if (handle)
	{
		fseek((FILE*)handle, (I32)length, SEEK_SET);
	}
}

void File::Seek(U64 length)
{
	if (handle)
	{
		fseek((FILE*)handle, (I32)length, SEEK_CUR);
	}
}

Vector<String> File::GetAllFiles(const String& dir)
{
	//TODO:
#ifdef PLATFORM_WINDOWS
	return Vector<String>();
#endif
}