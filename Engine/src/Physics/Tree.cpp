#include "Tree.hpp"

#include "Broadphase.hpp"
#include "Memory/Memory.hpp"

Tree::Tree()
{
	root = NULL_NODE;

	nodeCapacity = 16;
	nodeCount = 0;
	nodes = (Node*)Memory::Allocate(nodeCapacity * sizeof(Node), MEMORY_TAG_DATA_STRUCT);

	for (U32 i = 0; i < nodeCapacity - 1; ++i)
	{
		nodes[i].parent = i + 1;
		nodes[i].height = -1;
	}

	nodes[nodeCapacity - 1].parent = NULL_NODE;
	nodes[nodeCapacity - 1].height = -1;
	freeList = 0;

	insertionCount = 0;
}

Tree::~Tree()
{
	Memory::Free(nodes, nodeCapacity * sizeof(Node), MEMORY_TAG_DATA_STRUCT);
}

I32 Tree::AllocateNode()
{
	if (freeList == NULL_NODE)
	{
		ASSERT(nodeCount == nodeCapacity);

		Node* oldNodes = nodes;
		nodeCapacity *= 2;
		nodes = (Node*)Memory::Allocate(nodeCapacity * sizeof(Node), MEMORY_TAG_DATA_STRUCT);
		Memory::Copy(nodes, oldNodes, nodeCount * sizeof(Node));
		Memory::Free(oldNodes, nodeCount * sizeof(Node), MEMORY_TAG_DATA_STRUCT);

		for (U32 i = nodeCount; i < nodeCapacity - 1; ++i)
		{
			nodes[i].parent = i + 1;
			nodes[i].height = -1;
		}

		nodes[nodeCapacity - 1].parent = NULL_NODE;
		nodes[nodeCapacity - 1].height = -1;
		freeList = nodeCount;
	}

	I32 nodeID = freeList;
	freeList = nodes[nodeID].parent;
	nodes[nodeID].parent = NULL_NODE;
	nodes[nodeID].left = NULL_NODE;
	nodes[nodeID].right = NULL_NODE;
	nodes[nodeID].height = 0;
	nodes[nodeID].moved = false;
	++nodeCount;
	return nodeID;
}

void Tree::FreeNode(I32 nodeID)
{
	ASSERT(0 <= nodeID && nodeID < (I32)nodeCapacity && 0 < nodeCount);
	nodes[nodeID].parent = freeList;
	nodes[nodeID].height = -1;
	freeList = nodeID;
	--nodeCount;
}

I32 Tree::CreateProxy(PhysicsObject2D* object)
{
	I32 proxyID = AllocateNode();
	Node& node = nodes[proxyID];

	Vector2 r(-boxExtension, boxExtension);
	node.box.xBounds = object->collider->xBounds + r;
	node.box.yBounds = object->collider->yBounds + r;
	node.height = 0;
	node.moved = true;
	node.object = object;

	InsertLeaf(proxyID);

	return proxyID;
}

void Tree::DestroyProxy(I32 proxyID)
{
	ASSERT(0 <= proxyID && proxyID < (I32)nodeCapacity && nodes[proxyID].IsLeaf());

	RemoveLeaf(proxyID);
	FreeNode(proxyID);
}

bool Tree::MoveProxy(I32 proxyID, const Vector2& displacement)
{
	ASSERT(0 <= proxyID && proxyID < (I32)nodeCapacity && nodes[proxyID].IsLeaf());

	Box& box = nodes[proxyID].box;
	Box fatBox;
	Vector2 r(-boxExtension, boxExtension);
	fatBox.xBounds = box.xBounds - r;
	fatBox.yBounds = box.yBounds + r;

	Vector2 d = displacement * boxMultiplier;

	fatBox.xBounds += Vector2(-(F32)(d.x < 0.0f), d.x > 0.0f) * d.x;
	fatBox.yBounds += Vector2(-(F32)(d.y < 0.0f), d.y > 0.0f) * d.y;

	if (box.Contains(box))
	{
		Box hugeBox;
		hugeBox.xBounds = box.xBounds - r * 4.0f;
		hugeBox.yBounds = box.yBounds + r * 4.0f;

		if (hugeBox.Contains(box)) { return false; }
	}

	RemoveLeaf(proxyID);
	box = fatBox;

	InsertLeaf(proxyID);
	nodes[proxyID].moved = true;

	return true;
}

void Tree::InsertLeaf(I32 leaf)
{
	++insertionCount;

	if (root == NULL_NODE)
	{
		root = leaf;
		nodes[root].parent = NULL_NODE;
		return;
	}

	Box leafBox = nodes[leaf].box;
	I32 index = root;
	while (!nodes[index].IsLeaf())
	{
		I32 left = nodes[index].left;
		I32 right = nodes[index].right;

		F32 area = nodes[index].box.GetPerimeter();

		Box combinedAABB;
		combinedAABB.Combine(nodes[index].box, leafBox);
		F32 combinedArea = combinedAABB.GetPerimeter();

		F32 cost = 2.0f * combinedArea;

		F32 inheritanceCost = 2.0f * (combinedArea - area);

		F32 leftCost;
		if (nodes[left].IsLeaf())
		{
			Box box;
			box.Combine(leafBox, nodes[left].box);
			leftCost = box.GetPerimeter() + inheritanceCost;
		}
		else
		{
			Box box;
			box.Combine(leafBox, nodes[left].box);
			F32 oldArea = nodes[left].box.GetPerimeter();
			F32 newArea = box.GetPerimeter();
			leftCost = (newArea - oldArea) + inheritanceCost;
		}

		F32 rightCost;
		if (nodes[right].IsLeaf())
		{
			Box box;
			box.Combine(leafBox, nodes[right].box);
			rightCost = box.GetPerimeter() + inheritanceCost;
		}
		else
		{
			Box box;
			box.Combine(leafBox, nodes[right].box);
			F32 oldArea = nodes[right].box.GetPerimeter();
			F32 newArea = box.GetPerimeter();
			rightCost = newArea - oldArea + inheritanceCost;
		}

		if (cost < leftCost && cost < rightCost) { break; }
		if (leftCost < rightCost) { index = left; }
		else { index = right; }
	}

	I32 sibling = index;

	I32 oldParent = nodes[sibling].parent;
	I32 newParent = AllocateNode();
	nodes[newParent].parent = oldParent;
	nodes[newParent].box.Combine(leafBox, nodes[sibling].box);
	nodes[newParent].height = nodes[sibling].height + 1;

	if (oldParent != NULL_NODE)
	{
		if (nodes[oldParent].left == sibling) { nodes[oldParent].left = newParent; }
		else { nodes[oldParent].right = newParent; }

		nodes[newParent].left = sibling;
		nodes[newParent].right = leaf;
		nodes[sibling].parent = newParent;
		nodes[leaf].parent = newParent;
	}
	else
	{
		nodes[newParent].left = sibling;
		nodes[newParent].right = leaf;
		nodes[sibling].parent = newParent;
		nodes[leaf].parent = newParent;
		root = newParent;
	}

	index = nodes[leaf].parent;
	while (index != NULL_NODE)
	{
		index = Balance(index);

		I32 left = nodes[index].left;
		I32 right = nodes[index].right;

		ASSERT(left != NULL_NODE && right != NULL_NODE);

		nodes[index].height = 1 + Math::Max(nodes[left].height, nodes[right].height);
		nodes[index].box.Combine(nodes[left].box, nodes[right].box);

		index = nodes[index].parent;
	}
}

void Tree::RemoveLeaf(I32 leaf)
{
	if (leaf == root)
	{
		root = NULL_NODE;
		return;
	}

	I32 parent = nodes[leaf].parent;
	I32 grandParent = nodes[parent].parent;
	I32 sibling;
	if (nodes[parent].left == leaf) { sibling = nodes[parent].right; }
	else { sibling = nodes[parent].left; }

	if (grandParent != NULL_NODE)
	{
		if (nodes[grandParent].left == parent) { nodes[grandParent].left = sibling; }
		else { nodes[grandParent].right = sibling; }
		nodes[sibling].parent = grandParent;
		FreeNode(parent);

		I32 index = grandParent;
		while (index != NULL_NODE)
		{
			index = Balance(index);

			I32 left = nodes[index].left;
			I32 right = nodes[index].right;

			nodes[index].box.Combine(nodes[left].box, nodes[right].box);
			nodes[index].height = 1 + Math::Max(nodes[left].height, nodes[right].height);

			index = nodes[index].parent;
		}
	}
	else
	{
		root = sibling;
		nodes[sibling].parent = NULL_NODE;
		FreeNode(parent);
	}
}

I32 Tree::Balance(I32 iA)
{
	ASSERT(iA != NULL_NODE);

	Node* A = nodes + iA;
	if (A->IsLeaf() || A->height < 2) { return iA; }

	I32 iB = A->left;
	I32 iC = A->right;
	ASSERT(0 <= iB && iB < (I32)nodeCapacity && 0 <= iC && iC < (I32)nodeCapacity);

	Node* B = nodes + iB;
	Node* C = nodes + iC;

	I32 balance = C->height - B->height;

	if (balance > 1)
	{
		I32 iF = C->left;
		I32 iG = C->right;
		Node* F = nodes + iF;
		Node* G = nodes + iG;
		ASSERT(0 <= iF && iF < (I32)nodeCapacity && 0 <= iG && iG < (I32)nodeCapacity);

		C->left = iA;
		C->parent = A->parent;
		A->parent = iC;

		if (C->parent != NULL_NODE)
		{
			if (nodes[C->parent].left == iA) { nodes[C->parent].left = iC; }
			else
			{
				ASSERT(nodes[C->parent].right == iA);
				nodes[C->parent].right = iC;
			}
		}
		else { root = iC; }

		if (F->height > G->height)
		{
			C->right = iF;
			A->right = iG;
			G->parent = iA;
			A->box.Combine(B->box, G->box);
			C->box.Combine(A->box, F->box);

			A->height = 1 + Math::Max(B->height, G->height);
			C->height = 1 + Math::Max(A->height, F->height);
		}
		else
		{
			C->right = iG;
			A->right = iF;
			F->parent = iA;
			A->box.Combine(B->box, F->box);
			C->box.Combine(A->box, G->box);

			A->height = 1 + Math::Max(B->height, F->height);
			C->height = 1 + Math::Max(A->height, G->height);
		}

		return iC;
	}

	if (balance < -1)
	{
		I32 iD = B->left;
		I32 iE = B->right;
		Node* D = nodes + iD;
		Node* E = nodes + iE;
		ASSERT(0 <= iD && iD < (I32)nodeCapacity && 0 <= iE && iE < (I32)nodeCapacity);

		B->left = iA;
		B->parent = A->parent;
		A->parent = iB;

		if (B->parent != NULL_NODE)
		{
			if (nodes[B->parent].left == iA) { nodes[B->parent].left = iB; }
			else
			{
				ASSERT(nodes[B->parent].right == iA);
				nodes[B->parent].right = iB;
			}
		}
		else { root = iB; }

		if (D->height > E->height)
		{
			B->right = iD;
			A->left = iE;
			E->parent = iA;
			A->box.Combine(C->box, E->box);
			B->box.Combine(A->box, D->box);

			A->height = 1 + Math::Max(C->height, E->height);
			B->height = 1 + Math::Max(A->height, D->height);
		}
		else
		{
			B->right = iE;
			A->left = iD;
			D->parent = iA;
			A->box.Combine(C->box, D->box);
			B->box.Combine(A->box, E->box);

			A->height = 1 + Math::Max(C->height, D->height);
			B->height = 1 + Math::Max(A->height, E->height);
		}

		return iB;
	}

	return iA;
}

I32 Tree::ComputeHeight(I32 nodeID) const
{
	ASSERT(0 <= nodeID && nodeID < (I32)nodeCapacity);
	Node* node = nodes + nodeID;

	if (node->IsLeaf())
	{
		return 0;
	}

	I32 leftHeight = ComputeHeight(node->left);
	I32 rightHeight = ComputeHeight(node->right);
	return 1 + Math::Max(leftHeight, rightHeight);
}

I32 Tree::ComputeHeight() const
{
	return ComputeHeight(root);
}

I32 Tree::GetMaxBalance() const
{
	I32 maxBalance = 0;
	for (U32 i = 0; i < nodeCapacity; ++i)
	{
		const Node* node = nodes + i;
		if (node->height <= 1) { continue; }

		ASSERT(node->IsLeaf() == false);

		I32 balance = Math::Abs(nodes[node->right].height - nodes[node->left].height);
		maxBalance = Math::Max(maxBalance, balance);
	}

	return maxBalance;
}

F32 Tree::GetAreaRatio() const
{
	if (root == NULL_NODE) { return 0.0f; }

	const Node* r = nodes + root;
	F32 rootArea = r->box.GetPerimeter();

	F32 totalArea = 0.0f;
	for (U32 i = 0; i < nodeCapacity; ++i)
	{
		const Node* node = nodes + i;
		if (node->height < 0) { continue; }
		totalArea += node->box.GetPerimeter();
	}

	return totalArea / rootArea;
}