#include "Entity.hpp"

Hashmap<U64, U64> Components::ids{ 32 };
Vector<ComponentPool*> Components::pools;

void Components::Update()
{
	for (ComponentPool* pool : pools)
	{
		pool->Update();
	}
}

void Components::AddID(U64 id)
{
	ids.Insert(id, pools.Size());
}

void Components::AddPool(ComponentPool* pool)
{
	pools.Push(pool);
}

Component* Components::GetNewComponent(U64 hash)
{
	U64* id = ids[hash];

	if (id) { return pools[*id]->CreateComponent(); }
	else { Logger::Error("Can't Create A Component Type That Isn't Registered"); return nullptr; }
}