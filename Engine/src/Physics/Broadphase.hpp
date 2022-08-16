#pragma once

#include "Physics.hpp"

#include "Tree.hpp"

#define NULL_PROXY -1

class Broadphase
{
public:
	Broadphase();
	~Broadphase();

	I32 CreateProxy(PhysicsObject2D* object);
	void DestroyProxy(I32 proxyID);
	void MoveProxy(I32 proxyID, const Box& box, const Vector2& displacement);
	void TouchProxy(I32 proxyID);

	const Box& GetFatBox(I32 proxyID) const { return tree.GetFatBox(proxyID); }
	PhysicsObject2D* GetFixture(I32 proxyID) { return tree.GetObject(proxyID); }
	bool TestOverlap(I32 proxyIDA, I32 proxyIDB) const;

	U32 GetProxyCount() const { return proxyCount; }

	template<class T>
	void UpdatePairs(T* callback);
	template<class T>
	void Query(T* callback, const Box& box) const;
	template<class T>
	void RayCast(T* callback, const RayCastInput& input) const;

	I32 GetTreeHeight() const { return tree.GetHeight(); }
	I32 GetTreeBalance() const { return tree.GetMaxBalance(); }
	F32 GetTreeQuality() const { return tree.GetAreaRatio(); }

private:
	void BufferMove(I32 proxyID);
	void UnBufferMove(I32 proxyID);

	bool QueryCallback(I32 proxyID);

	Tree tree;

	U32 proxyCount;
	I32 queryProxyID;

	Vector<I32> moves;
	Vector<Pair> pairs;

	friend struct Tree;
};

template<class T>
NH_INLINE void Broadphase::Query(T* callback, const Box& box) const
{
	tree.Query(callback, box);
}

template<class T>
NH_INLINE void Broadphase::RayCast(T* callback, const RayCastInput& input) const
{
	tree.RayCast(callback, input);
}

template<class T>
NH_INLINE void Broadphase::UpdatePairs(T* callback)
{
	for (I32& id : moves)
	{
		queryProxyID = id;
		if (queryProxyID == NULL_PROXY) { continue; }

		const Box& fatBox = tree.GetFatBox(queryProxyID);

		tree.Query(this, fatBox);
	}

	for (Pair& pair : pairs)
	{
		callback->PairCallback(tree.GetObject(pair.proxyIDA), tree.GetObject(pair.proxyIDB));
	}

	pairs.Clear();

	for (I32& id : moves)
	{
		I32 proxyID = id;
		if (proxyID == NULL_PROXY) { continue; }

		tree.ClearMoved(proxyID);
	}

	moves.Clear();
}