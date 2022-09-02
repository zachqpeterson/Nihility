#pragma once

#include "Defines.hpp"
#include "Physics.hpp"
#include "Broadphase.hpp"

struct PhysicsObject2D;

class BoxTree : public Broadphase
{
	struct Node
	{
		Box box;

		U32 parent;
		U32 next;
		U32 left;
		U32 right;

		I32 height;

		PhysicsObject2D* obj;

		bool Leaf() const { return left == U32_MAX; }
	};

public:
	BoxTree(U32 capacity = 16, F32 fat = 0.05f);
	~BoxTree();
	void Destroy();
	void* operator new(U64 size);
	void operator delete(void* ptr);

	void InsertObj(PhysicsObject2D* obj) final;
	void RemoveObj(PhysicsObject2D* obj) final;
	void UpdateObj(PhysicsObject2D* obj) final;

	void Query(PhysicsObject2D* obj, Vector<PhysicsObject2D*>& result) final;
	void Query(const Box& box, Vector<PhysicsObject2D*>& result) final;
	void RaycastQuery(Raycast& ray, PhysicsObject2D* result) final;
	void RaycastQueryAll(Raycast& ray, Vector<PhysicsObject2D*>& result) final;

private:
	U32 AllocateNode();
	void FreeNode(U32 index);
	void InsertLeaf(U32 leaf);
	void RemoveLeaf(U32 leaf);

	U32 Balance(U32 index);
	U32 ComputeHeight() const;
	U32 ComputeHeight(U32 index) const;

	U32 Height() const;
	U32 Size() const;
	U32 MaximumBalance() const;
	F32 SurfaceAreaRatio() const;

	void Rebuild();

	U32 root;
	U32 freeList;
	U32 nodeCount;
	U32 nodeCapacity;
	Vector<Node> nodes;

	F32 fat;
};