#pragma once

#include "Defines.hpp"
#include "Memory/Memory.hpp"

struct BoolTable
{
	struct Column
	{
		Column(Column* next, U32 size) : next{ next }, data{ (U8*)Memory::Allocate(size, MEMORY_TAG_DATA_STRUCT) }, size{ size } {}
		~Column() { Destroy(); }

		void Destroy()
		{
			if (data)
			{
				Memory::Free(data, size * sizeof(U8), MEMORY_TAG_DATA_STRUCT);
			}
		}

		void* operator new(U64 size) { return Memory::Allocate(sizeof(Column), MEMORY_TAG_DATA_STRUCT); }
		void operator delete(void* ptr) { Memory::Free(ptr, sizeof(Column), MEMORY_TAG_DATA_STRUCT); }

		void Reset()
		{
			for (U32 i = 0; i < size; ++i)
			{
				data[i] = 0;
			}
		}

		bool GetSet(U64 i) const
		{
			U8 mask = 1 << (i % 8);
			U8& bit = data[i >> 3];
			bool b = bit & mask;
			bit |= mask;
			return b;
		}

		bool operator[](U64 i) const { return data[i >> 3] & (1 << (i % 8)); }

		Column* next;
		U8* data;
		U32 size;
	};

	BoolTable() : table{ nullptr }, size{ 0 } {}
	~BoolTable()
	{
		Column* c = table;
		while (c)
		{
			c->Destroy();
			c = c->next;
		}
	}

	void* operator new(U64 size) { return Memory::Allocate(sizeof(BoolTable), MEMORY_TAG_DATA_STRUCT); }
	void operator delete(void* ptr) { Memory::Free(ptr, sizeof(BoolTable), MEMORY_TAG_DATA_STRUCT); }

	void Expand()
	{
		table = new Column(table, (U32)((size / 8) + 1) * (size > 0));;
		++size;

		//Column* prev = table;
		//Column* c = table->next;
		//while (c)
		//{
		//	for (U32 i = 0; i < c->size; ++i) { prev->data[i] = c->data[i]; }
		//
		//	prev = c;
		//	c = c->next;
		//}
	}

	void Reset()
	{
		Column* c = table;
		while (c)
		{
			c->Reset();
			c = c->next;
		}
	}

	bool Get(U64 i, U64 j) const
	{
		Column* c = table;
		for (U64 index = 0; index < i; ++index)
		{
			c = c->next;
		}

		return c->operator[](j - i - 1);
	}
	bool GetSet(U64 i, U64 j) const
	{
		Column* c = table;
		for (U64 index = 0; index < i; ++index)
		{
			c = c->next;
		}

		return c->GetSet(j - i - 1);
	}

private:
	Column* table;
	U64 size;
};