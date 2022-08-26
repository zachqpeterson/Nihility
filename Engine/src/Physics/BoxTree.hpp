#pragma once

#include "Defines.hpp"
#include "Physics.hpp"

template<typename> struct Vector;

struct BoxTree
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

	void InsertObj(PhysicsObject2D* obj);
	void RemoveObj(PhysicsObject2D* obj);
	void UpdateObj(PhysicsObject2D* obj);

	Vector<PhysicsObject2D*> Query(PhysicsObject2D* obj);
	Vector<PhysicsObject2D*> Query(const Box& box);

	U32 Height() const;
	U32 Size() const;
	U32 MaximumBalance() const;
	F32 SurfaceAreaRatio() const;

	void Validate();
	void Rebuild();

private:
	U32 AllocateNode();
    void FreeNode(U32 index);
    void InsertLeaf(U32 index);
    void RemoveLeaf(U32 index);

    U32 Balance(U32 index);
    U32 ComputeHeight() const;

    U32 ComputeHeight(U32 index) const;
    void ValidateStructure(U32 index) const;
    void ValidateMetrics(U32 index) const;

    void PeriodicBoundaries(Vector<double>& position);
    bool MinimumImage(Vector<double>& separation, Vector<double>& shift);

	U32 root;
	U32 freeList;
	U32 nodeCount;
	U32 nodeCapacity;
	Vector<Node> nodes;

	F32 fat;

	Vector2 minImage;
	Vector2 minImageNeg;
};