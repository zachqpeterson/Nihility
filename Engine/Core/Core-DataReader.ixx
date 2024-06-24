module;

#include "Defines.hpp"
#include "Memory\Memory.hpp"

export module Core:DataReader;

import :File;
import Containers;

export struct NH_API DataReader
{
	DataReader();
	DataReader(void* data, U32 size);
	DataReader(File& file);

	~DataReader();
	void Destroy();

	template<typename Type> void Read(Type& value);
	template<typename Type> Type Read();
	template<typename Type> void ReadSize(Type& value, U32 size);
	void ReadSize(void* data, U32 size);
	String ReadString();
	String ReadLine();
	template<Character C, U64 Count> bool Compare(const C(&str)[Count]);

	void Seek(U32 length);

	U64 Size() const;
	U64 Position() const;
	U8* Data();
	U8* Data() const;
	U8* Pointer();
	U8* Pointer() const;

private:
	bool allocated{ false };
	U64 size{ 0 };
	U64 remaining{ 0 };
	U8* data{ nullptr };
	U8* dataPtr{ nullptr };
};

template<typename Type>
inline void DataReader::Read(Type& value)
{
	value = *reinterpret_cast<Type*>(dataPtr);
	dataPtr += sizeof(Type);
	remaining -= sizeof(Type);
}

template<typename Type>
inline Type DataReader::Read()
{
	Type value = *reinterpret_cast<Type*>(dataPtr);
	dataPtr += sizeof(Type);
	remaining -= sizeof(Type);
	return Move(value);
}

template<typename Type>
inline void DataReader::ReadSize(Type& value, U32 size)
{
	value = *reinterpret_cast<Type*>(dataPtr);
	dataPtr += size;
	remaining -= size;
}

template<Character C, U64 Count>
inline bool DataReader::Compare(const C(&str)[Count])
{
	bool result = Memory::Compare((C*)dataPtr, str, Count - 1);
	dataPtr += Count;
	remaining -= Count;

	return result;
}

DataReader::DataReader() {}

DataReader::DataReader(void* data, U32 size) : size{ size }, remaining{ size }, data{ (U8*)data }, dataPtr{ (U8*)data } {}

DataReader::DataReader(File& file) : allocated{ true }, size{ (U64)file.Size() }, remaining{ size }
{
	Memory::AllocateSize(&data, size);
	I64 ptr = file.Pointer();
	file.Read(data, (U32)size);
	file.SeekFromStart(ptr);

	dataPtr = data;
}

DataReader::~DataReader()
{
	Destroy();
}

void DataReader::Destroy()
{
	if (allocated) { Memory::Free(&data); }
}

void DataReader::ReadSize(void* data, U32 size)
{
	Memory::Copy(data, dataPtr, size);
	dataPtr += size;
	remaining -= size;
}

String DataReader::ReadString()
{
	String string = (C8*)dataPtr;

	dataPtr += string.Size() + 1;
	remaining -= string.Size() + 1;

	return Move(string);
}

String DataReader::ReadLine()
{
	String string;

	C8* it = (C8*)dataPtr;

	while (remaining-- && *it != '\n' && *it != '\r') { ++it; }

	//TODO:

	return Move(string);
}

void DataReader::Seek(U32 length)
{
	dataPtr += length;
	remaining -= length;
}

U64 DataReader::Size() const
{
	return size;
}

U64 DataReader::Position() const
{
	return dataPtr - data;
}

U8* DataReader::Data()
{
	return data;
}

U8* DataReader::Data() const
{
	return data;
}

U8* DataReader::Pointer()
{
	return dataPtr;
}

U8* DataReader::Pointer() const
{
	return dataPtr;
}