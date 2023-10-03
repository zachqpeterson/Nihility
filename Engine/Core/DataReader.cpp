#include "DataReader.hpp"

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
	data = dataPtr;
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