#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include "File.hpp"

struct NH_API DataReader
{
	DataReader();
	DataReader(void* data, U32 size);
	DataReader(File& file);

	~DataReader();
	void Destroy();

	template<class Type> void Read(Type& value);
	template<class Type> Type Read();
	template<class Type> void ReadSize(Type* data, U32 size);
	template<class Type> void ReadCount(Type* data, U32 count);
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
	bool allocated = false;
	U64 size = 0;
	U64 remaining = 0;
	U8* data = nullptr;
	U8* dataPtr = nullptr;
};

template<class Type>
inline void DataReader::Read(Type& value)
{
	value = *reinterpret_cast<Type*>(dataPtr);
	dataPtr += sizeof(Type);
	remaining -= sizeof(Type);
}

template<class Type>
inline Type DataReader::Read()
{
	Type value = *reinterpret_cast<Type*>(dataPtr);
	dataPtr += sizeof(Type);
	remaining -= sizeof(Type);
	return Move(value);
}

template<class Type>
inline void DataReader::ReadSize(Type* data, U32 size)
{
	Copy((U8*)data, dataPtr, size);
	dataPtr += size;
	remaining -= size;
}

template<class Type>
inline void DataReader::ReadCount(Type* data, U32 count)
{
	U64 size = sizeof(Type) * count;
	Copy(data, (Type*)dataPtr, count);
	dataPtr += size;
	remaining -= size;
}

template<Character C, U64 Count>
inline bool DataReader::Compare(const C(&str)[Count])
{
	bool result = CompareString((C*)dataPtr, str, Count - 1);
	dataPtr += Count;
	remaining -= Count;

	return result;
}