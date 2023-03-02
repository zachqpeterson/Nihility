#include "File.hpp"

#include "Containers\String.hpp"
#include "Containers\Vector.hpp"
#include "Core\Logger.hpp"

#if defined PLATFORM_WINDOWS

#include <Windows.h>
#include <fileapi.h>

static const UL32 fileAccesses[FILE_OPEN_COUNT]{
	GENERIC_READ,
	GENERIC_READ,
	GENERIC_READ,
	GENERIC_READ | GENERIC_WRITE,
	GENERIC_WRITE,
	GENERIC_WRITE,
	GENERIC_WRITE,
};

static const UL32 fileDispositions[FILE_OPEN_COUNT]{
	OPEN_EXISTING,
	OPEN_EXISTING,
	OPEN_EXISTING,
	OPEN_ALWAYS,
	OPEN_EXISTING,
	OPEN_ALWAYS,
	OPEN_EXISTING,
};

static const UL32 fileAttributes[FILE_OPEN_COUNT]{
	FILE_FLAG_SEQUENTIAL_SCAN,
	FILE_FLAG_RANDOM_ACCESS,
	FILE_FLAG_RANDOM_ACCESS,
	FILE_FLAG_RANDOM_ACCESS,
	FILE_FLAG_SEQUENTIAL_SCAN,
	FILE_FLAG_SEQUENTIAL_SCAN,
	NULL,
};

File::File() : handle{ INVALID_HANDLE_VALUE }, pointer{ 0 }, size{ 0 } {}

File::File(const String& path, FileOpenType type) : handle{ INVALID_HANDLE_VALUE }, pointer{ 0 }, size{ 0 }
{
	Open(path, type);
}

File::~File()
{
	Close();
}

void File::Close()
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}
}

bool File::Open(const String& path, FileOpenType type)
{
	Close();
	handle = CreateFileA(path.Data(), fileAccesses[type], FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, fileDispositions[type], fileAttributes[type], nullptr);
	GetSize();

	return handle != INVALID_HANDLE_VALUE;
}

bool File::Opened() const { return handle != INVALID_HANDLE_VALUE; }

bool File::ReadAll(String& string)
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		UL32 read;
		if (!ReadFile(handle, string.Data(), (UL32)size, &read, nullptr))
		{
			Logger::Error("Failed to read from file, {}", GetLastError());
			return false;
		}

		string.Resize();

		return true;
	}

	return false;
}

bool File::Read(String& string, U64 size)
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		UL32 read;
		if (!ReadFile(handle, string.Data(), (UL32)size, &read, nullptr))
		{
			Logger::Error("Failed to read from file, {}", GetLastError());
			return false;
		}

		string.Resize();

		return true;
	}

	return false;
}

bool File::Write(const String& string)
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		UL32 wrote;
		if (!WriteFile(handle, string.Data(), (UL32)string.Size(), &wrote, nullptr))
		{
			Logger::Error("Failed to write to file, {}", GetLastError());
			return false;
		}

		return true;
	}

	return false;
}

void File::Reset()
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		pointer = SetFilePointer(handle, 0, nullptr, FILE_BEGIN);
	}
}

void File::Seek(U64 offset)
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER li;
		li.QuadPart = offset;

		pointer = SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_CURRENT);
	}
}

void File::SeekFromStart(U64 offset)
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER li;
		li.QuadPart = offset;

		pointer = SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
	}
}

void File::SeekToEnd()
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		pointer = SetFilePointer(handle, 0, nullptr, FILE_END);
	}
}

bool File::Read(void* value, U64 size)
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		UL32 read;
		if (!ReadFile(handle, value, (UL32)size, &read, nullptr))
		{
			Logger::Error("Failed to read from file, {}", GetLastError());
			return false;
		}

		return true;
	}

	return false;
}

bool File::Write(const void* value, U64 size)
{
	if (handle != INVALID_HANDLE_VALUE)
	{
		UL32 wrote;
		if (!WriteFile(handle, value, (UL32)size, &wrote, nullptr))
		{
			Logger::Error("Failed to write to file, {}", GetLastError());
			return false;
		}

		return true;
	}

	return false;
}

I64 File::Pointer() const { return pointer; }

U64 File::Size() const { return size; }

void File::GetSize()
{
	size = SetFilePointer(handle, 0, nullptr, FILE_END);

	LARGE_INTEGER li;
	li.QuadPart = pointer;

	SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
}

#endif