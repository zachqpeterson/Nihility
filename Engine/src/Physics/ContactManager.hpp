#pragma once

#include "Defines.hpp"
#include "Broadphase.hpp"
#include "Math/Math.hpp"

#include <Containers/Vector2D.hpp>
#include <Containers/List.hpp>

struct Lookup
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
			bool b = data[i >> 3];
			data[i >> 3] |= mask;
			return b;
		}

		bool operator[](U64 i) const { return data[i >> 3] & (1 << (i % 8)); }

		Column* next;
		U8* data;
		U32 size;
	};

	Lookup() : table{ nullptr }, size{ 0 } {}
	~Lookup()
	{
		for (U64 i = 0; i < size; ++i)
		{
			table[i].Destroy();
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

struct Contact2D
{
	struct PhysicsObject2D* a;
	struct PhysicsObject2D* b;

	Vector2 normal;
	F64 penetration;
};

class ContactManager
{
public:
	ContactManager();
	~ContactManager();

	void* operator new(U64 size) { return Memory::Allocate(sizeof(ContactManager), MEMORY_TAG_DATA_STRUCT); }
	void operator delete(void* ptr) { Memory::Free(ptr, sizeof(ContactManager), MEMORY_TAG_DATA_STRUCT); }

	void AddObject(struct PhysicsObject2D* object);
	void MoveObject(I32 proxyID, const Vector2& displacement);
	void FindNewContacts();

	List<struct Contact2D>& Contacts() { return contacts; }

private:
	Broadphase broadphase;

	void PairCallback(struct PhysicsObject2D* objectA, struct PhysicsObject2D* objectB);

	Lookup contactLookup;
	List<struct Contact2D> contacts;

	friend class Broadphase;
};