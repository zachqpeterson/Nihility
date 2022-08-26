#include "BoxTree.hpp"

#include "Physics.hpp"

#include <Containers/Stack.hpp>

BoxTree::BoxTree(U32 capacity, F32 fat) : root{ U32_MAX }, freeList{ 0 }, nodeCount{ 0 }, nodeCapacity{ capacity }, nodes{ capacity, {} }, fat{ fat }
{
	U32 i = 0;
	for (Node& node : nodes)
	{
		node.next = i + 1;
		node.height = -1;
		++i;
	}

	nodes.Back().next = U32_MAX;
}

void BoxTree::InsertObj(PhysicsObject2D* obj)
{
	U32 index = AllocateNode();
	Node& node = nodes[index];
	node.box = obj->collider->box.Fattened(fat);
	node.height = 0;
	node.obj = obj;
	obj->proxyID = index;

	InsertLeaf(index);
}

void BoxTree::RemoveObj(PhysicsObject2D* obj)
{
	RemoveLeaf(obj->proxyID);
	FreeNode(obj->proxyID);
}

void BoxTree::UpdateObj(PhysicsObject2D* obj)
{

}

Vector<PhysicsObject2D*> BoxTree::Query(PhysicsObject2D* obj)
{

}

Vector<PhysicsObject2D*> BoxTree::Query(const Box& box)
{

}

U32 BoxTree::Height() const
{

}

U32 BoxTree::Size() const
{

}

U32 BoxTree::MaximumBalance() const
{

}

F32 BoxTree::SurfaceAreaRatio() const
{

}

void BoxTree::Validate()
{

}

void BoxTree::Rebuild()
{

}

U32 BoxTree::AllocateNode()
{
	if (freeList == U32_MAX)
	{
		nodeCapacity *= 2;

		nodes.Resize(nodeCapacity);

		U32 i = nodeCount;
		for (auto it = nodes.begin() + nodeCount; it != nodes.end(); ++it)
		{
			Node& node = *it;
			node.next = i + 1;
			node.height = -1;
			++i;
		}

		nodes.Back().next = U32_MAX;

		freeList = nodeCount;
	}

	U32 index = freeList;
	Node& node = nodes[index];
	freeList = node.next;
	node.parent = U32_MAX;
	node.left = U32_MAX;
	node.right = U32_MAX;
	node.height = 0;
	++nodeCount;

	return index;
}

void BoxTree::FreeNode(U32 index)
{

}

void BoxTree::InsertLeaf(U32 index)
{

}

void BoxTree::RemoveLeaf(U32 index)
{

}

U32 BoxTree::Balance(U32 index)
{

}

U32 BoxTree::ComputeHeight() const
{

}

U32 BoxTree::ComputeHeight(U32 index) const
{

}

void BoxTree::ValidateStructure(U32 index) const
{

}

void BoxTree::ValidateMetrics(U32 index) const
{

}

void BoxTree::PeriodicBoundaries(Vector<double>& position)
{

}

bool BoxTree::MinimumImage(Vector<double>& separation, Vector<double>& shift)
{

}
