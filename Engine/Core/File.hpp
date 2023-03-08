#pragma once

#include "Defines.hpp"

enum FileOpenType
{
	FILE_OPEN_READ_SEQ,
	FILE_OPEN_READ_RAN,
	FILE_OPEN_READ_WRITE,
	FILE_OPEN_READ_WRITE_NEW,
	FILE_OPEN_WRITE_APPEND,
	FILE_OPEN_WRITE_NEW,

	FILE_OPEN_COUNT
};

template<typename T>
struct Vector;
struct _iobuf;

//TODO: Make thread safe, use mutex
struct NH_API File
{
public:
	File();
	File(const String& path, FileOpenType type);
	~File();
	void Close();

	bool Open(const String& path, FileOpenType type);
	bool Opened() const;

	bool ReadAll(String& string);
	template<typename T> bool ReadAll(Vector<T>& data);

	bool Read(String& string, U64 size);
	template<typename T> bool Read(Vector<T>& data, U64 size);
	template<typename T> bool Read(T& value);

	bool Write(const String& string);
	template<typename T> bool Write(const Vector<T>& data);
	template<typename T> bool Write(const T& value);

	void Reset();
	void Seek(I64 offset);
	void SeekFromStart(I64 offset);
	void SeekToEnd();

	I64 Pointer() const;
	I64 Size();

private:
	bool Read(void* value, U64 size);
	bool Write(const void* value, U64 size);

	void GetSize();

	_iobuf* handle;
	I64 pointer;
	I64 size;
	bool updateSize;
};

template<typename T> inline bool File::ReadAll(Vector<T>& data)
{
	GetSize();
	data.Resize(size);
	return Read(data.Data(), sizeof(T) * data);
}

template<typename T> inline bool File::Read(Vector<T>& data, U64 size)
{
	data.Resize(size);
	return Read(data.Data(), sizeof(T) * size);
}

template<typename T> inline bool File::Read(T& value)
{
	constexpr U64 size = sizeof(T);
	return Read(&value, size);
}

template<typename T> inline bool File::Write(const Vector<T>& data)
{
	return Write(data.Data(), data.Size());
}

template<typename T> inline bool File::Write(const T& value)
{
	constexpr U64 size = sizeof(T);
	return Write(&value, size);
}