#include "ContactManager.hpp"

#include "Broadphase.hpp"
#include "Physics.hpp"

ContactManager::ContactManager() {}

ContactManager::~ContactManager()
{
	contacts.Destroy();
}

void ContactManager::AddObject(PhysicsObject2D* object)
{
	contactLookup.Expand();

	object->proxyID = broadphase.CreateProxy(object);
}

void ContactManager::MoveObject(I32 proxyID, const Box& box, const Vector2& displacement)
{
	broadphase.MoveProxy(proxyID, box, displacement);
}

void ContactManager::FindNewContacts()
{
	broadphase.UpdatePairs(this);
}

void ContactManager::PairCallback(struct PhysicsObject2D* objectA, struct PhysicsObject2D* objectB)
{
	U64 indexA = objectA->id;
	U64 indexB = objectB->id;

	if (indexA > indexB) { Math::Swap(indexA, indexB); }

	bool b = contactLookup.GetSet(indexA, indexB);

	if (objectA == objectB || (objectA->kinematic && objectB->kinematic) || !(objectA->layerMask & objectB->layerMask) || b) { return; }

	Contact2D contact{ objectA, objectB };
	contacts.PushBack(contact);
}