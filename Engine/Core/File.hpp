#pragma once

#include "Defines.hpp"

#include "Containers\String.hpp"
#include "Containers\Vector.hpp"

#include <io.h>

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

//https://learn.microsoft.com/en-us/cpp/c-runtime-library/low-level-i-o?view=msvc-170
//https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sopen-s-wsopen-s?view=msvc-170
//TODO: Make thread safe, use mutex
struct NH_API File
{
public:
	File();
	File(const String& path, FileOpenParam param);

	~File();
	void Close();

	bool Open(const String& path, FileOpenParam param);
	bool Opened() const;

	bool ReadAllBytes(U8* bytes) const;
	bool ReadAllBytes(Vector<U8>& bytes) const;
	bool ReadAllString(String& string) const;
	//TODO: Read line

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
	bool Write(const Vector<U8>& bytes);
	bool Write(U8* bytes, U64 size);

	void Reset() const;
	void Seek(L32 offset) const;
	void SeekFromStart(L32 offset) const;
	void SeekToEnd() const;

	L32 Size() const;

private:
	void OpenInput();
	void OpenOutput();
	void OpenError();

	L32 size;
	I32 handle;

	friend class Logger;
};

inline File::File() : size{ 0 }, handle{ -1 } {}

inline File::File(const String& path, FileOpenParam param) : size{ 0 }, handle{ -1 } { _sopen_s(&handle, path.Data(), param, 64, 128); size = _lseek(handle, 0, 2); }

inline File::~File() { Close(); }

inline void File::Close() { if (handle > -1) { _close(handle); handle = -1; } }

inline bool File::Open(const String& path, FileOpenParam param) { if (handle > -1) { Close(); } _sopen_s(&handle, path.Data(), param, 64, 128); return handle > -1; }

inline void File::OpenInput() { handle = 0; }

inline void File::OpenOutput() { handle = 1; }

inline void File::OpenError() { handle = 2; }

inline bool File::Opened() const { return handle > -1; }

inline bool File::ReadAllBytes(U8* bytes) const
{
	bytes = (U8*)Memory::Allocate(size);
	return _read(handle, bytes, size) > 0;
}

inline bool File::ReadAllBytes(Vector<U8>& bytes) const
{
	bytes.Resize(size);
	return _read(handle, bytes.Data(), size) > 0;
}

inline bool File::ReadAllString(String& string) const
{
	string.Resize(size);
	return _read(handle, string.Data(), size) > 0;
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

inline bool File::Write(const String& str) { return _write(handle, str.Data(), (U32)str.Size()); }

inline bool File::Write(U8* bytes, U64 size) { return _write(handle, bytes, (U32)size); }

inline bool File::Write(const Vector<U8>& bytes) { return _write(handle, bytes.Data(), (U32)bytes.Size()); }

inline void File::Reset() const { _lseek(handle, 0, 0); }

inline void File::Seek(L32 offset) const { _lseek(handle, offset, 1); }

inline void File::SeekFromStart(L32 offset) const { _lseek(handle, offset, 0); }

inline void File::SeekToEnd() const { _lseek(handle, 0, 2); }

inline L32 File::Size() const { return size; }
