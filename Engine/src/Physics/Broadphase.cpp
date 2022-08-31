#include "Broadphase.hpp"

Broadphase::Broadphase() : proxyCount{ 0 }, queryProxyID{ 0 }, moves{ 16 }, pairs{ 16 } {}

Broadphase::~Broadphase() {}

I32 Broadphase::CreateProxy(PhysicsObject2D* object)
{
	I32 proxyID = tree.CreateProxy(object);
	++proxyCount;
	BufferMove(proxyID);
	return proxyID;
}

void Broadphase::DestroyProxy(I32 proxyID)
{
	UnBufferMove(proxyID);
	--proxyCount;
	tree.DestroyProxy(proxyID);
}

void Broadphase::MoveProxy(I32 proxyID, const Box& box, const Vector2& displacement)
{
	if (tree.MoveProxy(proxyID, box, displacement))
	{
		BufferMove(proxyID);
	}
}

void Broadphase::TouchProxy(I32 proxyID)
{
	BufferMove(proxyID);
}

bool Broadphase::TestOverlap(I32 proxyIDA, I32 proxyIDB) const
{
	const Box& boxA = tree.GetFatBox(proxyIDA);
	const Box& boxB = tree.GetFatBox(proxyIDB);
	return boxA.Contains(boxB);
}

void Broadphase::BufferMove(I32 proxyID)
{
	moves.Push(proxyID);
}

void Broadphase::UnBufferMove(I32 proxyID)
{
	for (I32& i : moves) { if (i == proxyID) { i = NULL_PROXY; } }
}

bool Broadphase::QueryCallback(I32 proxyID)
{
	if (proxyID == queryProxyID) { return true; }

	const bool moved = tree.WasMoved(proxyID);
	if (moved && proxyID > queryProxyID) { return true; }

	pairs.Push({ Math::Min(proxyID, queryProxyID), Math::Max(proxyID, queryProxyID) });

	return true;
}