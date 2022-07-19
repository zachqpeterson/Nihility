#include "Physics.hpp"

HashMap<U64, PhysicsObject*> Physics::physicsObjects;

bool Physics::Initialize()
{
	PhysicsObject* invalidPO = (PhysicsObject*)Memory::Allocate(sizeof(PhysicsObject), MEMORY_TAG_DATA_STRUCT);
	invalidPO->id = U64_MAX;
	physicsObjects = Move(HashMap<U64, PhysicsObject*>(100, invalidPO));

	return true;
}

void Physics::Shutdown()
{
	for (List<HashMap<U64, PhysicsObject*>::Node>& l : physicsObjects)
	{
		for (HashMap<U64, PhysicsObject*>::Node& n : l)
		{
			Memory::Free(n.value, sizeof(PhysicsObject), MEMORY_TAG_DATA_STRUCT);
		}

		l.Clear();
	}

	physicsObjects.Destroy();
}

PhysicsObject* Physics::CreatePhysicsObject()
{
	static U64 id = 0;
	PhysicsObject* po = (PhysicsObject*)Memory::Allocate(sizeof(PhysicsObject), MEMORY_TAG_DATA_STRUCT);
	po->id = id;
	++id;

	physicsObjects.Insert(po->id, po);

	return po;
}

void Physics::Update()
{

}