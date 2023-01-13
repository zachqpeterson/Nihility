#include "BoxTree.hpp"

#include "Memory/Memory.hpp"

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

BoxTree::~BoxTree()
{
	Destroy();
}

void BoxTree::Destroy()
{
	nodes.Destroy();
	U32 root = U32_MAX;
	U32 freeList = 0;
	U32 nodeCount = 0;
	U32 nodeCapacity = 0;
}

void* BoxTree::operator new(U64 size) { return Memory::Allocate(sizeof(BoxTree), MEMORY_TAG_DATA_STRUCT); }
void BoxTree::operator delete(void* ptr) { Memory::Free(ptr, sizeof(BoxTree), MEMORY_TAG_DATA_STRUCT); }

void BoxTree::Update(List<List<Contact2D>>& contacts)
{

}

void BoxTree::InsertObj(PhysicsObject2D* obj)
{
	U32 index = AllocateNode();
	Node& node = nodes[index];
	node.box = (obj->collider->box + obj->transform->Position()).Fattened(fat);
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
	Box box = obj->collider->box + obj->transform->Position();
	U32 index = obj->proxyID;
	Node& node = nodes[index];

	if (node.box.Contains(box)) { return; }

	RemoveLeaf(index);

	node.box = box.Fattened(fat);

	InsertLeaf(index);
}

bool BoxTree::Query(PhysicsObject2D* obj, List<Contact2D>& contacts)
{
	Stack<U32> stack;
	stack.Push(root);

	const Box& box = obj->collider->box;

	while (stack.Size())
	{
		U32 index = stack.Pop();

		Node& node = nodes[index];
		Box& nodeBox = node.box;

		if (index < U32_MAX && box.Contains(nodeBox))
		{
			if (node.Leaf() && node.obj->id != obj->id)
			{
				//result.Push(node.obj);
			}
			else
			{
				stack.Push(node.left);
				stack.Push(node.right);
			}
		}
	}

	return false;
}

bool BoxTree::Query(const Box& box, Vector<PhysicsObject2D*>& result)
{
	Stack<U32> stack;
	stack.Push(root);

	while (stack.Size())
	{
		U32 index = stack.Pop();

		Node& node = nodes[index];
		Box nodeBox = node.box;

		if (index < U32_MAX && box.Contains(nodeBox))
		{
			if (node.Leaf())
			{
				result.Push(node.obj);
			}
			else
			{
				stack.Push(node.left);
				stack.Push(node.right);
			}
		}
	}

	return false;
}

bool BoxTree::RaycastQuery(PhysicsObject2D* obj, PhysicsObject2D* result)
{
	return false;
}

bool BoxTree::RaycastQueryAll(Raycast& ray, Vector<PhysicsObject2D*>& result)
{
	return false;
}

U32 BoxTree::Height() const
{
	return nodes[root * (root == U32_MAX)].height * (root == U32_MAX);
}

U32 BoxTree::Size() const
{
	return nodeCount;
}

U32 BoxTree::MaximumBalance() const
{
	U32 maxBalance = 0;
	for (const Node& node : nodes)
	{
		if (node.height <= 1) { continue; }

		U32 balance = Math::Abs(nodes[node.left].height - nodes[node.right].height);
		maxBalance = Math::Max(maxBalance, balance);
	}

	return maxBalance;
}

F32 BoxTree::SurfaceAreaRatio() const
{
	if (root == U32_MAX) { return 0.0f; }

	F32 rootArea = nodes[root].box.Area();
	F32 totalArea = 0.0f;

	for (const Node& node : nodes)
	{
		if (node.height < 0) { continue; }

		totalArea += node.box.Area();
	}

	return totalArea / rootArea;
}

void BoxTree::Rebuild() //TODO: optimise
{
	Vector<U32> nodeIndices(nodeCount);
	U32 count = 0;

	U32 i = 0;
	for (Node& node : nodes)
	{
		if (node.height < 0)
		{
			if (node.Leaf())
			{
				node.parent = U32_MAX;
				nodeIndices[count] = i;
				++count;
			}
			else { FreeNode(i); }
		}

		++i;
	}

	while (count > 1)
	{
		F32 minCost = F32_MAX;
		I32 iMin = -1;
		I32	jMin = -1;

		for (i = 0; i < count; ++i)
		{
			Box& boxi = nodes[nodeIndices[i]].box;

			for (U32 j = i + 1; j < count; j++)
			{
				Box& boxj = nodes[nodeIndices[j]].box;
				Box box;
				box.Merge(boxi, boxj);
				F32 cost = box.Area();

				if (cost < minCost)
				{
					iMin = i;
					jMin = j;
					minCost = cost;
				}
			}
		}

		U32 index1 = nodeIndices[iMin];
		U32 index2 = nodeIndices[jMin];

		U32 parent = AllocateNode();
		Node& parentNode = nodes[parent];
		parentNode.left = index1;
		parentNode.right = index2;
		parentNode.height = 1 + Math::Max(nodes[index1].height, nodes[index2].height);
		parentNode.box.Merge(nodes[index1].box, nodes[index2].box);
		parentNode.parent = U32_MAX;

		nodes[index1].parent = parent;
		nodes[index2].parent = parent;

		nodeIndices[jMin] = nodeIndices[count - 1];
		nodeIndices[iMin] = parent;
		--count;
	}

	root = nodeIndices[0];
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
	nodes[index].next = freeList;
	nodes[index].height = -1;
	freeList = index;
	--nodeCount;
}

void BoxTree::InsertLeaf(U32 leaf)
{
	if (root == U32_MAX)
	{
		root = leaf;
		nodes[root].parent = U32_MAX;
		return;
	}

	Node& leafNode = nodes[leaf];
	Box& leafBox = leafNode.box;
	U32 index = root;

	while (!nodes[index].Leaf())
	{
		Node& node = nodes[index];

		U32 left = node.left;
		Node& leftNode = nodes[left];
		U32 right = node.right;
		Node& rightNode = nodes[right];

		F32 surfaceArea = node.box.Area();

		Box combinedBox;
		combinedBox.Merge(node.box, leafBox);
		F32 combinedSurfaceArea = combinedBox.Area();

		F32 cost = 2.0f * combinedSurfaceArea;

		F32 inheritanceCost = 2.0f * (combinedSurfaceArea - surfaceArea);

		F32 costLeft;
		if (leftNode.Leaf())
		{
			Box box;
			box.Merge(leafBox, leftNode.box);
			costLeft = box.Area() + inheritanceCost;
		}
		else
		{
			Box box;
			box.Merge(leafBox, leftNode.box);
			F32 oldArea = leftNode.box.Area();
			F32 newArea = box.Area();
			costLeft = (newArea - oldArea) + inheritanceCost;
		}

		F32 costRight;
		if (rightNode.Leaf())
		{
			Box box;
			box.Merge(leafBox, rightNode.box);
			costRight = box.Area() + inheritanceCost;
		}
		else
		{
			Box box;
			box.Merge(leafBox, rightNode.box);
			F32 oldArea = rightNode.box.Area();
			F32 newArea = box.Area();
			costRight = (newArea - oldArea) + inheritanceCost;
		}

		if ((cost < costLeft) && (cost < costRight)) { break; }

		if (costLeft < costRight) { index = left; }
		else { index = right; }
	}

	U32 sibling = index;
	Node& siblingNode = nodes[sibling];
	U32 oldParent = siblingNode.parent;
	Node& oldParentNode = nodes[oldParent];
	U32 newParent = AllocateNode();
	Node& parentNode = nodes[newParent];

	parentNode.parent = oldParent;
	parentNode.box.Merge(leafBox, siblingNode.box);
	parentNode.height = siblingNode.height + 1;

	if (oldParent != U32_MAX)
	{
		if (oldParentNode.left == sibling) { oldParentNode.left = newParent; }
		else { oldParentNode.right = newParent; }

		parentNode.left = sibling;
		parentNode.right = leaf;
		siblingNode.parent = newParent;
		leafNode.parent = newParent;
	}
	else
	{
		parentNode.left = sibling;
		parentNode.right = leaf;
		siblingNode.parent = newParent;
		leafNode.parent = newParent;
		root = newParent;
	}

	index = leafNode.parent;
	while (index != U32_MAX)
	{
		index = Balance(index);
		Node& node = nodes[index];

		U32 left = node.left;
		U32 right = node.right;

		node.height = 1 + Math::Max(nodes[left].height, nodes[right].height);
		node.box.Merge(nodes[left].box, nodes[right].box);

		index = node.parent;
	}
}

void BoxTree::RemoveLeaf(U32 leaf)
{
	if (leaf == root)
	{
		root = U32_MAX;
		return;
	}

	U32 parent = nodes[leaf].parent;
	U32 grandParent = nodes[parent].parent;
	U32 sibling = (&nodes[parent].left)[nodes[parent].left == leaf]; //TODO: find other uses

	if (grandParent != U32_MAX)
	{
		if (nodes[grandParent].left == parent) { nodes[grandParent].left = sibling; }
		else { nodes[grandParent].right = sibling; }

		nodes[sibling].parent = grandParent;
		FreeNode(parent);

		U32 index = grandParent;
		while (index != U32_MAX)
		{
			index = Balance(index);

			U32 left = nodes[index].left;
			U32 right = nodes[index].right;

			nodes[index].box.Merge(nodes[left].box, nodes[right].box);
			nodes[index].height = 1 + Math::Max(nodes[left].height, nodes[right].height);

			index = nodes[index].parent;
		}
	}
	else
	{
		root = sibling;
		nodes[sibling].parent = U32_MAX;
		FreeNode(parent);
	}
}

U32 BoxTree::Balance(U32 index)
{
	Node& node = nodes[index];

	if (node.Leaf() || (node.height < 2)) { return index; }

	U32 left = node.left;
	Node& leftNode = nodes[left];
	U32 right = node.right;
	Node& rightNode = nodes[right];

	I32 currentBalance = rightNode.height - leftNode.height;

	if (currentBalance > 1)
	{
		U32 rightLeft = rightNode.left;
		Node& rightLeftNode = nodes[rightLeft];
		U32 rightRight = rightNode.right;
		Node& rightRightNode = nodes[rightRight];

		rightNode.left = index;
		rightNode.parent = node.parent;
		node.parent = right;

		if (rightNode.parent != U32_MAX)
		{
			if (nodes[rightNode.parent].left == index) nodes[rightNode.parent].left = right;
			else { nodes[rightNode.parent].right = right; }
		}
		else { root = right; }

		if (rightLeftNode.height > rightRightNode.height)
		{
			rightNode.right = rightLeft;
			node.right = rightRight;
			rightRightNode.parent = index;
			node.box.Merge(leftNode.box, rightRightNode.box);
			rightNode.box.Merge(node.box, rightLeftNode.box);

			node.height = 1 + Math::Max(leftNode.height, rightRightNode.height);
			rightNode.height = 1 + Math::Max(node.height, rightLeftNode.height);
		}
		else
		{
			rightNode.right = rightRight;
			node.right = rightLeft;
			rightLeftNode.parent = index;
			node.box.Merge(leftNode.box, rightLeftNode.box);
			rightNode.box.Merge(node.box, rightRightNode.box);

			node.height = 1 + Math::Max(leftNode.height, rightLeftNode.height);
			rightNode.height = 1 + Math::Max(node.height, rightRightNode.height);
		}

		return right;
	}

	if (currentBalance < -1)
	{
		U32 leftLeft = leftNode.left;
		Node& leftLeftNode = nodes[leftLeft];
		U32 leftRight = leftNode.right;
		Node& leftRightNode = nodes[leftRight];

		leftNode.left = index;
		leftNode.parent = node.parent;
		node.parent = left;

		if (leftNode.parent != U32_MAX)
		{
			if (nodes[leftNode.parent].left == index) { nodes[leftNode.parent].left = left; }
			else { nodes[leftNode.parent].right = left; }
		}
		else { root = left; }

		if (leftLeftNode.height > leftRightNode.height)
		{
			leftNode.right = leftLeft;
			node.left = leftRight;
			leftRightNode.parent = index;
			node.box.Merge(rightNode.box, leftRightNode.box);
			leftNode.box.Merge(node.box, leftLeftNode.box);

			node.height = 1 + Math::Max(rightNode.height, leftRightNode.height);
			leftNode.height = 1 + Math::Max(node.height, leftLeftNode.height);
		}
		else
		{
			leftNode.right = leftRight;
			node.left = leftLeft;
			leftLeftNode.parent = index;
			node.box.Merge(rightNode.box, leftLeftNode.box);
			leftNode.box.Merge(node.box, leftRightNode.box);

			node.height = 1 + Math::Max(rightNode.height, leftLeftNode.height);
			leftNode.height = 1 + Math::Max(node.height, leftRightNode.height);
		}

		return left;
	}

	return index;
}

U32 BoxTree::ComputeHeight() const
{
	return ComputeHeight(root);
}

U32 BoxTree::ComputeHeight(U32 index) const
{
	if (index == U32_MAX) { return 0; }

	Stack<U32> s;
	s.Push(index);

	I32 height = 0;

	while (s.Size())
	{
		const Node& node = nodes[s.Pop()];

		if (node.left != U32_MAX) { s.Push(node.left); }
		if (node.right != U32_MAX) { s.Push(node.right); }

		height = Math::Max(height, node.height);
	}

	return height;
}