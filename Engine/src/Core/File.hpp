#pragma once

#include "Defines.hpp"

#include <stdio.h>

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
	void Restart();
	bool ReadLine(struct String& line, U64 maxLength = 512);
	bool WriteLine(const struct String& str);
	struct String Read(U64 length);
	U8* ReadBytes(U64 length, I64 tag = 5);
	U8* ReadAllBytes(U64& size, I64 tag = 5);
	I8 ReadI8();
	I16 ReadI16();
	I32 ReadI32();
	I64 ReadI64();
	U8 ReadU8();
	U16 ReadU16();
	U32 ReadU32();
	U64 ReadU64();
	struct String ReadAllText();
	void Write(const struct String& str);
	void SeekFromStart(U64 length);
	void Seek(U64 length);

	static Vector<struct String> GetAllFiles(const struct String& dir);

	template<typename T> bool ReadT(T& t)
	{
		if (handle)
		{
			fread(&t, sizeof(T), 1, (FILE*)handle);
			return true;
		}

		return false;
	}

public:
	void* handle;
};