#pragma once

#include "Physics.hpp"

#include <Containers/Stack.hpp>

#define boxExtension 0.1f
#define boxMultiplier 4.0f
#define NULL_NODE -1

struct Tree
{
	struct Node
	{
		bool IsLeaf() const { return left == NULL_NODE; }

		Box box;

		I32 parent;

		I32 left;
		I32 right;

		I32 height;

		bool moved;

		PhysicsObject2D* object;
	};

public:
	Tree();
	~Tree();

	I32 CreateProxy(PhysicsObject2D* object);
	void DestroyProxy(I32 proxyID);
	bool MoveProxy(I32 proxyID, const Box& box, const Vector2& displacement);

	const Box& GetFatBox(I32 proxyID) const;

	template<class T>
	void Query(T* callback, const Box& box) const;
	template<class T>
	void RayCast(T* callback, const RayCastInput& input) const;

	I32 GetHeight() const { return nodes[root].height * root != NULL_NODE; }
	bool WasMoved(I32 proxyID) const { return nodes[proxyID].moved; }
	void ClearMoved(I32 proxyID) { nodes[proxyID].moved = false; }
	PhysicsObject2D* GetObject(I32 proxyID) { return nodes[proxyID].object; }

	I32 GetMaxBalance() const;
	F32 GetAreaRatio() const;
	
private:
	I32 AllocateNode();
	void FreeNode(I32 nodeID);

	void InsertLeaf(I32 leaf);
	void RemoveLeaf(I32 leaf);

	I32 Balance(I32 iA);

	I32 ComputeHeight(I32 nodeID) const;
	I32 ComputeHeight() const;

	I32 root;
	Node* nodes;
	U32 nodeCount;
	U32 nodeCapacity;
	I32 freeList;
	U32 insertionCount;
};

NH_INLINE const Box& Tree::GetFatBox(I32 proxyID) const
{
	return nodes[proxyID].box;
}

template<class T>
NH_INLINE void Tree::Query(T* callback, const Box& box) const
{
	Stack<I32> stack;
	stack.Push(root);

	while (stack.Size())
	{
		I32 nodeID = stack.Pop();
		if (nodeID == NULL_NODE) { continue; }

		const Node* node = nodes + nodeID;

		if (Physics::TestOverlap(node->box, box))
		{
			if (node->IsLeaf())
			{
				if (!callback->QueryCallback(nodeID)) { return; }
			}
			else
			{
				stack.Push(node->left);
				stack.Push(node->right);
			}
		}
	}
}

template<class T>
NH_INLINE void Tree::RayCast(T* callback, const RayCastInput& input) const
{
	Vector2 p1 = input.p1;
	Vector2 p2 = input.p2;
	Vector2 r = p2 - p1;
	ASSERT(r.x > 0.0f && r.y > 0.0f);
	r.Normalize();

	Vector2 v = Math::Cross(1.0f, r);
	Vector2 abs_v = Math::Abs(v);

	F32 maxFraction = input.maxFraction;

	Box segmentBox;
	{
		Vector2 t = p1 + (p2 - p1) * maxFraction;
		segmentBox.xBounds.x = Math::Min(p1.x, t.x);
		segmentBox.xBounds.y = Math::Max(p1.x, t.x);
		segmentBox.yBounds.x = Math::Min(p1.y, t.y);
		segmentBox.yBounds.y = Math::Max(p1.y, t.y);
	}

	Stack<I32> stack;
	stack.Push(root);

	while (stack.Size())
	{
		I32 nodeID = stack.Pop();

		const Node* node = nodes + nodeID;

		if (nodeID == NULL_NODE || !Physics::TestOverlap(node->box, segmentBox)) { continue; }

		Vector2 c = node->box.Center();
		Vector2 h = node->box.Extents();
		F32 separation = Math::Abs(v.Dot(p1 - c)) - abs_v.Dot(h);
		if (separation > 0.0f) { continue; }

		if (node->IsLeaf())
		{
			RayCastInput subInput;
			subInput.p1 = input.p1;
			subInput.p2 = input.p2;
			subInput.maxFraction = maxFraction;

			F32 value = callback->RaycastCallback(subInput, nodeID);

			if (value == 0.0f) { return; }

			if (value > 0.0f)
			{
				maxFraction = value;
				Vector2 t = p1 + (p2 - p1) * maxFraction;
				segmentBox.xBounds.x = Math::Min(p1.x, t.x);
				segmentBox.xBounds.y = Math::Max(p1.x, t.x);
				segmentBox.yBounds.x = Math::Min(p1.y, t.y);
				segmentBox.yBounds.y = Math::Max(p1.y, t.y);
			}
		}
		else
		{
			stack.Push(node->left);
			stack.Push(node->right);
		}
	}
}