#pragma once

#include "File.hpp"

struct DataReader
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
