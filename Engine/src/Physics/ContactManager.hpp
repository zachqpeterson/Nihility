#pragma once

#include "Defines.hpp"
#include "Broadphase.hpp"
#include "Math/Math.hpp"
#include "Containers/BoolTable.hpp"

#include <Containers/Vector2D.hpp>
#include <Containers/List.hpp>

struct Contact2D
{
	struct PhysicsObject2D* a;
	struct PhysicsObject2D* b;

	Vector2 normal;
	F32 penetration;
};

class ContactManager
{
public:
	ContactManager();
	~ContactManager();

	void* operator new(U64 size) { return Memory::Allocate(sizeof(ContactManager), MEMORY_TAG_DATA_STRUCT); }
	void operator delete(void* ptr) { Memory::Free(ptr, sizeof(ContactManager), MEMORY_TAG_DATA_STRUCT); }

	void AddObject(struct PhysicsObject2D* object);
	void MoveObject(I32 proxyID, const Box& box, const Vector2& displacement);
	void FindNewContacts();

	List<struct Contact2D>& Contacts() { return contacts; }

private:
	Broadphase broadphase;

	void PairCallback(struct PhysicsObject2D* objectA, struct PhysicsObject2D* objectB);

	BoolTable contactLookup;
	List<struct Contact2D> contacts;

	friend class Broadphase;
};