#include "File.hpp"

#include "Containers\String.hpp"
#include "Containers\Vector.hpp"
#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <stdio.h>

static const C8* openMode[FILE_OPEN_COUNT]{
	"rbS",
	"rbR",
	"r+R",
	"w+R",
	"aS",
	"wS",
};

File::File() : handle{ nullptr }, pointer{ 0 }, size{ 0 }, updateSize{ false } {}

File::File(const String& path, FileOpenType type) : handle{ nullptr }, pointer{ 0 }, size{ 0 }
{
	Open(path, type);
}

File::~File()
{
	Close();
}

void File::Close()
{
	if (handle)
	{
		fclose(handle);
		handle = nullptr;
	}
}

bool File::Open(const String& path, FileOpenType type)
{
	Close();
	fopen_s(&handle, path.Data(), openMode[type]);
	updateSize = true;

	return handle;
}

bool File::Opened() const { return handle; }

bool File::ReadAll(String& string)
{
	GetSize();
	return fread_s(string.Data(), size, size, 1, handle);
}

bool File::Read(String& string, U64 size)
{
	return fread_s(string.Data(), size, size, 1, handle);
}

bool File::Write(const String& string)
{
	updateSize = true;
	return fwrite(string.Data(), string.Size(), 1, handle);
}

void File::Reset()
{
	_fseeki64(handle, 0, SEEK_SET);
	pointer = _ftelli64(handle);
}

void File::Seek(I64 offset)
{
	_fseeki64(handle, offset, SEEK_CUR);
	pointer = _ftelli64(handle);
}

void File::SeekFromStart(I64 offset)
{
	_fseeki64(handle, offset, SEEK_SET);
	pointer = _ftelli64(handle);
}

void File::SeekToEnd()
{
	_fseeki64(handle, 0, SEEK_END);
	pointer = _ftelli64(handle);
}

bool File::Read(void* value, U64 size)
{
	return fread_s(value, size, size, 1, handle);
}

bool File::Write(const void* value, U64 size)
{
	updateSize = true;
	return fwrite(value, size, 1, handle);
}

I64 File::Pointer() const { return pointer; }

I64 File::Size() { GetSize(); return size; }

void File::GetSize()
{
	if (updateSize)
	{
		_fseeki64(handle, 0, SEEK_END);
		size = _ftelli64(handle);
		_fseeki64(handle, pointer, SEEK_SET);
	}
}

#endif