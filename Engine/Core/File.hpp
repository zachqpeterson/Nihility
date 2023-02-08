#pragma once

#include "Defines.hpp"

#include "String.hpp"

#include <io.h>
#include <fcntl.h>
#include <stdio.h>

enum FileOpenParam
{
	FILE_OPEN_BINARY_READ_SEQ = 32800,
	FILE_OPEN_BINARY_READ_RAN = 32784,
	FILE_OPEN_BINARY_WRITE = 32769,
	FILE_OPEN_BINARY_WRITE_NEW = 33569,
	FILE_OPEN_BINARY_WRITE_APPEND = 32809,

	FILE_OPEN_TEXT_READ_SEQ = 16416,
	FILE_OPEN_TEXT_READ_RAN = 16400,
	FILE_OPEN_TEXT_WRITE = 16385,
	FILE_OPEN_TEXT_WRITE_NEW = 17185,
	FILE_OPEN_TEXT_WRITE_APPEND = 16425,
};

//TODO: https://learn.microsoft.com/en-us/cpp/c-runtime-library/low-level-i-o?view=msvc-170
//TODO: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sopen-s-wsopen-s?view=msvc-170
struct File
{
public:
	File();
	File(const String& path, FileOpenParam param);

	~File();
	void Close();

	bool Open(const String& path, FileOpenParam param);
	bool Opened() const;

	bool ReadBytes(U8* bytes) const; //TODO: Vector
	bool ReadString(String& string) const;

	bool ReadU8(U8& value) const;
	bool ReadI8(I8& value) const;
	bool ReadU16(U16& value) const;
	bool ReadI16(I16& value) const;
	bool ReadU32(U32& value) const;
	bool ReadI32(I32& value) const;
	bool ReadU64(U64& value) const;
	bool ReadI64(I64& value) const;
	template<typename T> bool ReadT(T& value) const;

	bool Write(const String& str);
	bool Write(U8* bytes, U64 count);// TODO: Vector version

	void Reset() const;
	void Seek(L32 offset) const;
	void SeekFromStart(L32 offset) const;
	void SeekToEnd() const;

	L32 Size() const;

private:

	L32 size;
	int handle;
};

inline File::File() : size{ 0 }, handle{ 0 } {}

inline File::File(const String& path, FileOpenParam param) : size{ 0 }, handle{ 0 } { _sopen_s(&handle, path.Data(), param, 64, 128); size = _lseek(handle, 0, 2); }

inline File::~File() { Close(); }

inline bool File::Open(const String& path, FileOpenParam param) { _sopen_s(&handle, path.Data(), param, 64, 128); }

inline void File::Close() { if (handle) { _close(handle); } }

inline bool File::Opened() const { return handle; }

inline bool File::ReadBytes(U8* bytes) const
{

}

inline bool File::ReadString(String& string) const
{

}

inline bool File::ReadU8(U8& value) const { return _read(handle, &value, sizeof(U8)) > 0; }

inline bool File::ReadI8(I8& value) const { return _read(handle, &value, sizeof(I8)) > 0; }

inline bool File::ReadU16(U16& value) const { return _read(handle, &value, sizeof(U16)) > 0; }

inline bool File::ReadI16(I16& value) const { return _read(handle, &value, sizeof(I16)) > 0; }

inline bool File::ReadU32(U32& value) const { return _read(handle, &value, sizeof(U32)) > 0; }

inline bool File::ReadI32(I32& value) const { return _read(handle, &value, sizeof(I32)) > 0; }

inline bool File::ReadU64(U64& value) const { return _read(handle, &value, sizeof(U64)) > 0; }

inline bool File::ReadI64(I64& value) const { return _read(handle, &value, sizeof(I64)) > 0; }

template<typename T> inline bool File::ReadT(T& value) const { return _read(handle, &value, sizeof(T)) > 0; }

inline void File::Reset() const { _lseek(handle, 0, 0); }

inline void File::Seek(L32 offset) const { _lseek(handle, offset, 1); }

inline void File::SeekFromStart(L32 offset) const { _lseek(handle, offset, 0); }

inline void File::SeekToEnd() const { _lseek(handle, 0, 2); }

inline L32 File::Size() const { return size; }