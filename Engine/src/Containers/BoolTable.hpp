#pragma once

#include "Defines.hpp"
#include "Memory/Memory.hpp"

struct BoolTable
{
	struct Column
	{
		void Destroy()
		{
			Memory::Free(data, size * sizeof(U8), MEMORY_TAG_DATA_STRUCT);
		}

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

	void Expand()
	{
		Column* column = (Column*)Memory::Allocate(sizeof(Column), MEMORY_TAG_DATA_STRUCT);
		column->size = (U32)((size / 8) + 1) * (size > 0);
		column->data = (U8*)Memory::Allocate(sizeof(U8) * column->size, MEMORY_TAG_DATA_STRUCT);
		column->next = table;
		table = column;
		++size;
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