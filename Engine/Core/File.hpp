#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"

enum NH_API FileOpenMode
{
	FILE_OPEN_BINARY = 0x8000,
	FILE_OPEN_TEXT = 0x4000,
	FILE_OPEN_UTF_16 = 0x20000,
	FILE_OPEN_UTF_8 = 0x40000,

	FILE_OPEN_READ = 0x0000,
	FILE_OPEN_WRITE = 0x0001,
	FILE_OPEN_READ_WRITE = 0x0002,

	FILE_OPEN_RANDOM = 0x0010,
	FILE_OPEN_SEQUENTIAL = 0x0020,

	FILE_OPEN_NEW = 0x0100 | 0x0200,
	FILE_OPEN_APPEND = 0x0008,

	FILE_OPEN_FLUSH_IMMEDIATE = 0x100000,

	FILE_OPEN_TEMPORARY = 0x0040,

	FILE_OPEN_LOG = FILE_OPEN_WRITE | FILE_OPEN_NEW | FILE_OPEN_SEQUENTIAL | FILE_OPEN_TEXT,
	FILE_OPEN_CONSOLE = FILE_OPEN_WRITE | FILE_OPEN_SEQUENTIAL | FILE_OPEN_TEXT | FILE_OPEN_FLUSH_IMMEDIATE,
	FILE_OPEN_RESOURCE_READ = FILE_OPEN_READ | FILE_OPEN_SEQUENTIAL | FILE_OPEN_BINARY,
	FILE_OPEN_RESOURCE_WRITE = FILE_OPEN_WRITE | FILE_OPEN_NEW | FILE_OPEN_SEQUENTIAL | FILE_OPEN_BINARY,
	FILE_OPEN_TEMP_RESOURCE = FILE_OPEN_WRITE | FILE_OPEN_SEQUENTIAL | FILE_OPEN_BINARY | FILE_OPEN_NEW | FILE_OPEN_FLUSH_IMMEDIATE | FILE_OPEN_TEMPORARY,
};

struct NH_API File
{
private:
	struct FileStats
	{
		U32 driveNumber;
		U16 informationNode;
		U16 mode;
		I16 nlink;
		I16 userId;
		I16 groupId;
		U32 driveNumber2;
		I64 size;
		I64 lastAccessed;
		I64 lastModified;
		I64 creationTime;
	} stats = {};

public:
	File();
	File(const String& path, I32 mode);
	~File();
	void Destroy();

	bool Open(const String& path, I32 mode);
	bool Opened() const;
	void Close();

	template<class Type> U64 Read(Type& value);
	U64 Read(void* buffer, U64 size);
	String ReadString();
	String ReadLine();
	String ReadAll();

	template<class Type> U64 Write(const Type& value);
	template<typename... Args> U64 FormatedWrite(const Args... args);
	U64 Write(const String& data);
	U64 Write(const void* buffer, U64 size);

	void Reset();
	void Seek(I64 offset);
	void SeekFromStart(I64 offset);
	void SeekToEnd();

	I64 Pointer() const;
	I64 Size();

	static const String& WorkingDirectory();
	static bool Delete(const String& path);
	static bool Exists(const String& path);

private:
	template<class Type> U64 FormatedWrite(C8* buffer, Type type);

	bool Flush();
	bool FillBuffer();
	bool EmptyBuffer();

	I32 handle = -1;
	I64 pointer = 0;
	bool opened = false;

	U8* streamPtr = nullptr;
	U8* streamBuffer = nullptr;
	U64 bufferSize = 0;
	U64 bufferRemaining = 0;
	U64 streamFlag = 0;
};

template<class Type>
U64 File::Read(Type& value)
{
	return Read(&value, sizeof(Type));
}

template<class Type>
U64 File::Write(const Type& value)
{
	return Write(&value, sizeof(Type));
}

template<typename... Args>
U64 File::FormatedWrite(const Args... args)
{
	static C8 buffer[1024] = {};

	U64 count = 0;

	((count += FormatedWrite(buffer + count, args)), ...);

	return count;
}

template<class Type>
U64 File::FormatedWrite(C8* buffer, Type type)
{
	if constexpr (IsStringType<Type>) { return Write(type.Data(), type.Size()); }
	else if constexpr (IsSame<Type, StringView>) { return Write(type.Data(), type.Size()); }
	else if constexpr (IsStringLiteral<Type>) { return Write(type, Length(type)); }
	else { return Write(buffer, String::Format(buffer, type)); }
}