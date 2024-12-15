/*
* This is a modified version of Box2D for C++: https://github.com/eXpl0it3r/Box2D
*
* Copyright (c) 2011 Erin Catto http://box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "Broadphase.hpp"

#include "Physics.hpp"

import Containers;

static constexpr TreeNode DefaultTreeNode = { { { 0.0f, 0.0f }, { 0.0f, 0.0f } }, 0, { NullNode }, NullNode, NullNode, -1, -2, false };
static constexpr U64 TreeStackSize = 1024;

//Tree

void DynamicTree::Create()
{
	proxyCount = 0;

	//Tree init
	root = NullNode;
	Memory::AllocateArray(&nodes, 16, nodeCapacity);
	nodeCount = 0;

	// Build a linked list for the free list.
	TreeNode* node = nodes;

	for (U32 i = 0; i < nodeCapacity - 1; ++i, ++node)
	{
		node->next = i + 1;
		node->height = -1;
	}

	node->next = NullNode;
	node->height = -1;

	freeList = 0;
	proxyCount = 0;
	leafIndices = nullptr;
	leafBoxes = nullptr;
	leafCenters = nullptr;
	binIndices = nullptr;
	rebuildCapacity = 0;
}

void DynamicTree::Destroy()
{
	Memory::Free(&nodes);
	Memory::Free(&leafIndices);
	Memory::Free(&leafBoxes);
	Memory::Free(&leafCenters);
	Memory::Free(&binIndices);
}

I32 DynamicTree::AllocateNode()
{
	// Expand the node pool as needed.
	if (freeList == NullNode)
	{
		// The free list is empty. Rebuild a bigger pool.
		Memory::Reallocate(&nodes, nodeCapacity + 1, nodeCapacity);

		// Build a linked list for the free list. The parent
		// pointer becomes the "next" pointer.
		TreeNode* node = nodes;

		for (U32 i = 0; i < nodeCapacity - 1; ++i, ++node)
		{
			node->next = i + 1;
			node->height = -1;
		}

		node->next = NullNode;
		node->height = -1;

		freeList = nodeCount;
	}

	// Peel a node off the free list.
	I32 nodeId = freeList;
	freeList = nodes[nodeId].next;
	nodes[nodeId] = DefaultTreeNode;
	++nodeCount;
	return nodeId;
}

void DynamicTree::FreeNode(I32 nodeId)
{
	nodes[nodeId].next = freeList;
	nodes[nodeId].height = -1;
	freeList = nodeId;
	--nodeCount;
}

I32 DynamicTree::FindBestSibling(AABB box)
{
	Vector2 center = box.Center();
	F32 area = box.Perimeter();

	AABB rootBox = nodes[root].aabb;

	F32 areaBase = rootBox.Perimeter();

	F32 directCost = AABB::Combine(rootBox, box).Perimeter();
	F32 inheritedCost = 0.0f;

	I32 bestSibling = root;
	F32 bestCost = directCost;

	I32 index = root;
	while (nodes[index].height > 0)
	{
		I32 child1 = nodes[index].child1;
		I32 child2 = nodes[index].child2;

		F32 cost = directCost + inheritedCost;

		if (cost < bestCost)
		{
			bestSibling = index;
			bestCost = cost;
		}

		inheritedCost += directCost - areaBase;

		bool leaf1 = nodes[child1].height == 0;
		bool leaf2 = nodes[child2].height == 0;

		F32 lowerCost1 = F32_MAX;
		AABB box1 = nodes[child1].aabb;
		F32 directCost1 = AABB::Combine(box1, box).Perimeter();
		F32 area1 = 0.0f;
		if (leaf1)
		{
			F32 cost1 = directCost1 + inheritedCost;

			if (cost1 < bestCost)
			{
				bestSibling = child1;
				bestCost = cost1;
			}
		}
		else
		{
			area1 = box1.Perimeter();
			lowerCost1 = inheritedCost + directCost1 + Math::Min(area - area1, 0.0f);
		}

		F32 lowerCost2 = F32_MAX;
		AABB box2 = nodes[child2].aabb;
		F32 directCost2 = AABB::Combine(box2, box).Perimeter();
		F32 area2 = 0.0f;
		if (leaf2)
		{
			F32 cost2 = directCost2 + inheritedCost;

			if (cost2 < bestCost)
			{
				bestSibling = child2;
				bestCost = cost2;
			}
		}
		else
		{
			area2 = box2.Perimeter();
			lowerCost2 = inheritedCost + directCost2 + Math::Min(area - area2, 0.0f);
		}

		if (bestCost <= lowerCost1 && bestCost <= lowerCost2)
		{
			break;
		}

		if (lowerCost1 == lowerCost2 && leaf1 == false)
		{
			Vector2 d1 = box1.Center() - center;
			Vector2 d2 = box2.Center() - center;
			lowerCost1 = d1.SqrMagnitude();
			lowerCost2 = d2.SqrMagnitude();
		}

		if (lowerCost1 < lowerCost2 && leaf1 == false)
		{
			index = child1;
			areaBase = area1;
			directCost = directCost1;
		}
		else
		{
			index = child2;
			areaBase = area2;
			directCost = directCost2;
		}
	}

	return bestSibling;
}

void DynamicTree::RotateNodes(I32 iA)
{
	TreeNode* A = nodes + iA;
	if (A->height < 2) { return; }

	I32 iB = A->child1;
	I32 iC = A->child2;

	TreeNode* B = nodes + iB;
	TreeNode* C = nodes + iC;

	if (B->height == 0)
	{
		I32 iF = C->child1;
		I32 iG = C->child2;
		TreeNode* F = nodes + iF;
		TreeNode* G = nodes + iG;

		F32 costBase = C->aabb.Perimeter();

		AABB aabbBG = AABB::Combine(B->aabb, G->aabb);
		F32 costBF = aabbBG.Perimeter();

		AABB aabbBF = AABB::Combine(B->aabb, F->aabb);
		F32 costBG = aabbBF.Perimeter();

		if (costBase < costBF && costBase < costBG) { return; }

		if (costBF < costBG)
		{
			A->child1 = iF;
			C->child1 = iB;

			B->parent = iC;
			F->parent = iA;

			C->aabb = aabbBG;

			C->height = 1 + Math::Max(B->height, G->height);
			A->height = 1 + Math::Max(C->height, F->height);
			C->layers = B->layers | G->layers;
			A->layers = C->layers | F->layers;
			C->enlarged = B->enlarged || G->enlarged;
			A->enlarged = C->enlarged || F->enlarged;
		}
		else
		{
			A->child1 = iG;
			C->child2 = iB;

			B->parent = iC;
			G->parent = iA;

			C->aabb = aabbBF;

			C->height = 1 + Math::Max(B->height, F->height);
			A->height = 1 + Math::Max(C->height, G->height);
			C->layers = B->layers | F->layers;
			A->layers = C->layers | G->layers;
			C->enlarged = B->enlarged || F->enlarged;
			A->enlarged = C->enlarged || G->enlarged;
		}
	}
	else if (C->height == 0)
	{
		I32 iD = B->child1;
		I32 iE = B->child2;
		TreeNode* D = nodes + iD;
		TreeNode* E = nodes + iE;

		F32 costBase = B->aabb.Perimeter();

		AABB aabbCE = AABB::Combine(C->aabb, E->aabb);
		F32 costCD = aabbCE.Perimeter();

		AABB aabbCD = AABB::Combine(C->aabb, D->aabb);
		F32 costCE = aabbCD.Perimeter();

		if (costBase < costCD && costBase < costCE) { return; }

		if (costCD < costCE)
		{
			A->child2 = iD;
			B->child1 = iC;

			C->parent = iB;
			D->parent = iA;

			B->aabb = aabbCE;

			B->height = 1 + Math::Max(C->height, E->height);
			A->height = 1 + Math::Max(B->height, D->height);
			B->layers = C->layers | E->layers;
			A->layers = B->layers | D->layers;
			B->enlarged = C->enlarged || E->enlarged;
			A->enlarged = B->enlarged || D->enlarged;
		}
		else
		{
			A->child2 = iE;
			B->child2 = iC;

			C->parent = iB;
			E->parent = iA;

			B->aabb = aabbCD;
			B->height = 1 + Math::Max(C->height, D->height);
			A->height = 1 + Math::Max(B->height, E->height);
			B->layers = C->layers | D->layers;
			A->layers = B->layers | E->layers;
			B->enlarged = C->enlarged || D->enlarged;
			A->enlarged = B->enlarged || E->enlarged;
		}
	}
	else
	{
		I32 iD = B->child1;
		I32 iE = B->child2;
		I32 iF = C->child1;
		I32 iG = C->child2;

		TreeNode* D = nodes + iD;
		TreeNode* E = nodes + iE;
		TreeNode* F = nodes + iF;
		TreeNode* G = nodes + iG;

		F32 areaB = B->aabb.Perimeter();
		F32 areaC = C->aabb.Perimeter();
		F32 costBase = areaB + areaC;
		RotateType bestRotation = ROTATE_TYPE_NONE;
		F32 bestCost = costBase;

		AABB aabbBG = AABB::Combine(B->aabb, G->aabb);
		F32 costBF = areaB + aabbBG.Perimeter();
		if (costBF < bestCost)
		{
			bestRotation = ROTATE_TYPE_BF;
			bestCost = costBF;
		}

		AABB aabbBF = AABB::Combine(B->aabb, F->aabb);
		F32 costBG = areaB + aabbBF.Perimeter();
		if (costBG < bestCost)
		{
			bestRotation = ROTATE_TYPE_BG;
			bestCost = costBG;
		}

		AABB aabbCE = AABB::Combine(C->aabb, E->aabb);
		F32 costCD = areaC + aabbCE.Perimeter();
		if (costCD < bestCost)
		{
			bestRotation = ROTATE_TYPE_CD;
			bestCost = costCD;
		}

		AABB aabbCD = AABB::Combine(C->aabb, D->aabb);
		F32 costCE = areaC + aabbCD.Perimeter();
		if (costCE < bestCost)
		{
			bestRotation = ROTATE_TYPE_CE;
		}

		switch (bestRotation)
		{
		case ROTATE_TYPE_BF: {
			A->child1 = iF;
			C->child1 = iB;

			B->parent = iC;
			F->parent = iA;

			C->aabb = aabbBG;
			C->height = 1 + Math::Max(B->height, G->height);
			A->height = 1 + Math::Max(C->height, F->height);
			C->layers = B->layers | G->layers;
			A->layers = C->layers | F->layers;
			C->enlarged = B->enlarged || G->enlarged;
			A->enlarged = C->enlarged || F->enlarged;
		} break;
		case ROTATE_TYPE_BG: {
			A->child1 = iG;
			C->child2 = iB;

			B->parent = iC;
			G->parent = iA;

			C->aabb = aabbBF;
			C->height = 1 + Math::Max(B->height, F->height);
			A->height = 1 + Math::Max(C->height, G->height);
			C->layers = B->layers | F->layers;
			A->layers = C->layers | G->layers;
			C->enlarged = B->enlarged || F->enlarged;
			A->enlarged = C->enlarged || G->enlarged;
		} break;
		case ROTATE_TYPE_CD: {
			A->child2 = iD;
			B->child1 = iC;

			C->parent = iB;
			D->parent = iA;

			B->aabb = aabbCE;
			B->height = 1 + Math::Max(C->height, E->height);
			A->height = 1 + Math::Max(B->height, D->height);
			B->layers = C->layers | E->layers;
			A->layers = B->layers | D->layers;
			B->enlarged = C->enlarged || E->enlarged;
			A->enlarged = B->enlarged || D->enlarged;
		} break;
		case ROTATE_TYPE_CE: {
			A->child2 = iE;
			B->child2 = iC;

			C->parent = iB;
			E->parent = iA;

			B->aabb = aabbCD;
			B->height = 1 + Math::Max(C->height, D->height);
			A->height = 1 + Math::Max(B->height, E->height);
			B->layers = C->layers | D->layers;
			A->layers = B->layers | E->layers;
			B->enlarged = C->enlarged || D->enlarged;
			A->enlarged = B->enlarged || E->enlarged;
		} break;
		case ROTATE_TYPE_NONE:
		default: break;
		}
	}
}

void DynamicTree::InsertLeaf(I32 leaf, bool shouldRotate)
{
	if (root == NullNode)
	{
		root = leaf;
		nodes[root].parent = NullNode;
		return;
	}

	AABB leafAABB = nodes[leaf].aabb;
	I32 sibling = FindBestSibling(leafAABB);

	I32 oldParent = nodes[sibling].parent;
	I32 newParent = AllocateNode();

	nodes[newParent].parent = oldParent;
	nodes[newParent].userData = -1;
	nodes[newParent].aabb = AABB::Combine(leafAABB, nodes[sibling].aabb);
	nodes[newParent].layers = nodes[leaf].layers | nodes[sibling].layers;
	nodes[newParent].height = nodes[sibling].height + 1;

	if (oldParent != NullNode)
	{
		if (nodes[oldParent].child1 == sibling) { nodes[oldParent].child1 = newParent; }
		else { nodes[oldParent].child2 = newParent; }

		nodes[newParent].child1 = sibling;
		nodes[newParent].child1 = leaf;
		nodes[sibling].child1 = newParent;
		nodes[leaf].child1 = newParent;
	}
	else
	{
		nodes[newParent].child1 = sibling;
		nodes[newParent].child1 = leaf;
		nodes[sibling].child1 = newParent;
		nodes[leaf].child1 = newParent;
		root = newParent;
	}

	I32 index = nodes[leaf].parent;
	while (index != NullNode)
	{
		I32 child1 = nodes[index].child1;
		I32 child2 = nodes[index].child2;

		nodes[index].aabb = AABB::Combine(nodes[child1].aabb, nodes[child2].aabb);
		nodes[index].layers = nodes[child1].layers | nodes[child2].layers;
		nodes[index].height = 1 + Math::Max(nodes[child1].height, nodes[child2].height);
		nodes[index].enlarged = nodes[child1].enlarged || nodes[child2].enlarged;

		if (shouldRotate) { RotateNodes(index); }

		index = nodes[index].parent;
	}
}

void DynamicTree::RemoveLeaf(I32 leaf)
{
	if (leaf == root)
	{
		root = NullNode;
		return;
	}

	I32 parent = nodes[leaf].parent;
	I32 grandParent = nodes[parent].parent;
	I32 sibling;
	if (nodes[parent].child1 == leaf) { sibling = nodes[parent].child2; }
	else { sibling = nodes[parent].child1; }

	if (grandParent != NullNode)
	{
		if (nodes[grandParent].child1 == parent) { nodes[grandParent].child1 = sibling; }
		else { nodes[grandParent].child2 = sibling; }
		nodes[sibling].parent = grandParent;
		FreeNode(parent);

		I32 index = grandParent;
		while (index != NullNode)
		{
			I32 child1 = nodes[index].child1;
			I32 child2 = nodes[index].child2;

			nodes[index].aabb = AABB::Combine(nodes[child1].aabb, nodes[child2].aabb);
			nodes[index].layers = nodes[child1].layers | nodes[child2].layers;
			nodes[index].height = 1 + Math::Max(nodes[child1].height, nodes[child2].height);

			index = nodes[index].parent;
		}
	}
	else
	{
		root = sibling;
		nodes[sibling].parent = NullNode;
		FreeNode(parent);
	}
}

I32 DynamicTree::CreateProxy(const AABB& aabb, U64 layers, I32 userData)
{
	I32 proxyId = AllocateNode();
	TreeNode& node = nodes[proxyId];

	node.aabb = aabb;
	node.userData = userData;
	node.layers = layers;
	node.height = 0;

	InsertLeaf(proxyId, true);

	++proxyCount;

	return proxyId;
}

void DynamicTree::DestroyProxy(I32 proxyId)
{
	RemoveLeaf(proxyId);
	FreeNode(proxyId);
	--proxyCount;
}

void DynamicTree::MoveProxy(I32 proxyId, const AABB& aabb)
{
	RemoveLeaf(proxyId);

	nodes[proxyId].aabb = aabb;

	InsertLeaf(proxyId, false);
}

void DynamicTree::EnlargeProxy(I32 proxyId, const AABB& aabb)
{
	nodes[proxyId].aabb = aabb;

	I32 parentIndex = nodes[proxyId].parent;

	bool changed = true;

	while(parentIndex != NullNode && changed)
	{
		changed = AABB::Enlarge(nodes[parentIndex].aabb, aabb);
		nodes[parentIndex].enlarged = true;
		parentIndex = nodes[parentIndex].parent;
	}

	while (parentIndex != NullNode && !nodes[parentIndex].enlarged)
	{
		nodes[parentIndex].enlarged = true;
		parentIndex = nodes[parentIndex].parent;
	}
}

bool DynamicTree::AABBOverlaps(AABB a, AABB b)
{
	Vector2 d1 = { b.lowerBound.x - a.upperBound.x, b.lowerBound.y - a.upperBound.y };
	Vector2 d2 = { a.lowerBound.x - b.upperBound.x, a.lowerBound.y - b.upperBound.y };

	if (d1.x > 0.0f || d1.y > 0.0f) { return false; }

	if (d2.x > 0.0f || d2.y > 0.0f) { return false; }

	return true;
}

void DynamicTree::Query(const AABB& aabb, U64 layerMask, QueryPairContext& context)
{
	Stack<I32> stack(256);
	stack.Push(root);

	I32 nodeId;
	while (stack.Pop(nodeId))
	{
		if (nodeId == NullNode) { continue; }

		const TreeNode* node = nodes + nodeId;

		if (AABBOverlaps(node->aabb, aabb) && (node->layers & layerMask))
		{
			if (node->child1 == NullNode)
			{
				// callback to user code with proxy id
				bool proceed = Broadphase::PairQueryCallback(nodeId, node->userData, context);
				if (proceed == false) { return; }
			}
			else
			{
				stack.Push(node->child1);
				stack.Push(node->child2);
			}
		}
	}
}

void DynamicTree::QueryContinuous(const AABB& aabb, U64 layerMask, ContinuousContext& context)
{
	Stack<I32> stack(256);
	stack.Push(root);

	I32 nodeId;
	while (stack.Pop(nodeId))
	{
		if (nodeId == NullNode) { continue; }

		const TreeNode* node = nodes + nodeId;

		if (AABBOverlaps(node->aabb, aabb) && (node->layers & layerMask))
		{
			if (node->child1 == NullNode)
			{
				// callback to user code with proxy id
				bool proceed = Broadphase::ContinuousQueryCallback(nodeId, context);
				if (proceed == false) { return; }
			}
			else
			{
				stack.Push(node->child1);
				stack.Push(node->child2);
			}
		}
	}
}

//TODO: Raycast
//TODO: Shapecast

I32 DynamicTree::GetHeight()
{
	if (root == NullNode) { return 0; }
	return nodes[root].height;
}

I32 DynamicTree::ComputeHeight(I32 nodeId)
{
	TreeNode* node = nodes + nodeId;

	if (node->child1 == NullNode) { return 0; }

	I32 height1 = ComputeHeight(node->child1);
	I32 height2 = ComputeHeight(node->child2);
	return 1 + Math::Max(height1, height2);
}

I32 DynamicTree::ComputeHeight()
{
	return ComputeHeight(root);
}

I32 DynamicTree::GetMaxBalance()
{
	I32 maxBalance = 0;
	for (U32 i = 0; i < nodeCapacity; ++i)
	{
		const TreeNode* node = nodes + i;
		if (node->height <= 1) { continue; }

		I32 child1 = node->child1;
		I32 child2 = node->child2;
		I32 balance = Math::Abs(nodes[child2].height - nodes[child1].height);
		maxBalance = Math::Max(maxBalance, balance);
	}

	return maxBalance;
}

F32 DynamicTree::GetAreaRatio()
{
	if (root == NullNode) { return 0.0f; }

	F32 rootArea = nodes[root].aabb.Perimeter();

	F32 totalArea = 0.0f;
	for (U32 i = 0; i < nodeCapacity; ++i)
	{
		const TreeNode* node = nodes + i;
		if (node->height < 0 || node->height == 0 || i == root) { continue; }

		totalArea += node->aabb.Perimeter();
	}

	return totalArea / rootArea;
}


I32 DynamicTree::GetProxyCount()
{
	return proxyCount;
}

I32 DynamicTree::Rebuild(bool fullBuild)
{
	if (proxyCount == 0) { return 0; }

	// Ensure capacity for rebuild space
	if (proxyCount > rebuildCapacity)
	{
		I32 newCapacity = proxyCount + proxyCount / 2;

		Memory::Free(&leafIndices);
		Memory::AllocateArray(&leafIndices, newCapacity);


		Memory::Free(&leafCenters);
		Memory::AllocateArray(&leafCenters, newCapacity);
		rebuildCapacity = newCapacity;
	}

	I32 leafCount = 0;
	I32 stack[TreeStackSize];
	I32 stackCount = 0;

	I32 nodeIndex = root;
	TreeNode* node = nodes + nodeIndex;

	// Gather all proxy nodes that have grown and all internal nodes that haven't grown. Both are
	// considered leaves in the tree rebuild.
	// Free all internal nodes that have grown.
	// todo use a node growth metric instead of simply enlarged to reduce rebuild size and frequency
	// this should be weighed against b2_aabbMargin
	while (true)
	{
		if (node->height == 0 || (node->enlarged == false && fullBuild == false))
		{
			leafIndices[leafCount] = nodeIndex;
			leafCenters[leafCount] = node->aabb.Center();
			leafCount += 1;

			// Detach
			node->parent = NullNode;
		}
		else
		{
			I32 doomedNodeIndex = nodeIndex;

			// Handle children
			nodeIndex = node->child1;

			if (stackCount < TreeStackSize)
			{
				stack[stackCount++] = node->child2;
			}

			node = nodes + nodeIndex;

			// Remove doomed node
			FreeNode(doomedNodeIndex);

			continue;
		}

		if (stackCount == 0) { break; }

		nodeIndex = stack[--stackCount];
		node = nodes + nodeIndex;
	}

	root = BuildTree(leafCount);

	return leafCount;
}

struct RebuildItem
{
	I32 nodeIndex;
	I32 childCount;

	// Leaf indices
	I32 startIndex;
	I32 splitIndex;
	I32 endIndex;
};

I32 DynamicTree::BuildTree(I32 leafCount)
{
	if (leafCount == 1)
	{
		nodes[leafIndices[0]].parent = NullNode;
		return leafIndices[0];
	}

	// todo large stack item
	RebuildItem stack[TreeStackSize];
	I32 top = 0;

	stack[0].nodeIndex = AllocateNode();
	stack[0].childCount = -1;
	stack[0].startIndex = 0;
	stack[0].endIndex = leafCount;
	stack[0].splitIndex = PartitionMid(leafIndices, leafCenters, leafCount);

	while (true)
	{
		RebuildItem* item = stack + top;

		item->childCount += 1;

		if (item->childCount == 2)
		{
			// This internal node has both children established

			if (top == 0) { break; }

			RebuildItem* parentItem = stack + (top - 1);
			TreeNode* parentNode = nodes + parentItem->nodeIndex;

			if (parentItem->childCount == 0)
			{
				parentNode->child1 = item->nodeIndex;
			}
			else
			{
				parentNode->child2 = item->nodeIndex;
			}

			TreeNode* node = nodes + item->nodeIndex;

			node->parent = parentItem->nodeIndex;

			TreeNode* child1 = nodes + node->child1;
			TreeNode* child2 = nodes + node->child2;

			node->aabb = AABB::Combine(child1->aabb, child2->aabb);
			node->height = 1 + Math::Max(child1->height, child2->height);
			node->layers = child1->layers | child2->layers;

			// Pop stack
			top -= 1;
		}
		else
		{
			I32 startIndex, endIndex;
			if (item->childCount == 0)
			{
				startIndex = item->startIndex;
				endIndex = item->splitIndex;
			}
			else
			{
				startIndex = item->splitIndex;
				endIndex = item->endIndex;
			}

			I32 count = endIndex - startIndex;

			if (count == 1)
			{
				I32 childIndex = leafIndices[startIndex];
				TreeNode* node = nodes + item->nodeIndex;

				if (item->childCount == 0)
				{
					node->child1 = childIndex;
				}
				else
				{
					node->child2 = childIndex;
				}

				TreeNode* childNode = nodes + childIndex;
				childNode->parent = item->nodeIndex;
			}
			else
			{
				top += 1;
				RebuildItem* newItem = stack + top;
				newItem->nodeIndex = AllocateNode();
				newItem->childCount = -1;
				newItem->startIndex = startIndex;
				newItem->endIndex = endIndex;
				newItem->splitIndex = PartitionMid(leafIndices + startIndex, leafCenters + startIndex, count);
				newItem->splitIndex += startIndex;
			}
		}
	}

	TreeNode* rootNode = nodes + stack[0].nodeIndex;

	TreeNode* child1 = nodes + rootNode->child1;
	TreeNode* child2 = nodes + rootNode->child2;

	rootNode->aabb = AABB::Combine(child1->aabb, child2->aabb);
	rootNode->height = 1 + Math::Max(child1->height, child2->height);
	rootNode->layers = child1->layers | child2->layers;

	return stack[0].nodeIndex;
}

I32 DynamicTree::PartitionMid(I32* indices, Vector2* centers, I32 count)
{
	if (count <= 2) { return count / 2; }

	Vector2 lowerBound = centers[0];
	Vector2 upperBound = centers[0];

	for (I32 i = 1; i < count; ++i)
	{
		lowerBound = Math::Min(lowerBound, centers[i]);
		upperBound = Math::Max(upperBound, centers[i]);
	}

	Vector2 d = upperBound - lowerBound;
	Vector2 c = { 0.5f * (lowerBound.x + upperBound.x), 0.5f * (lowerBound.y + upperBound.y) };

	// Partition longest axis using the Hoare partition scheme
	// https://en.wikipedia.org/wiki/Quicksort
	// https://nicholasvadivelu.com/2021/01/11/array-partition/
	I32 i1 = 0, i2 = count;
	if (d.x > d.y)
	{
		F32 pivot = c.x;

		while (i1 < i2)
		{
			while (i1 < i2 && centers[i1].x < pivot)
			{
				i1 += 1;
			};

			while (i1 < i2 && centers[i2 - 1].x >= pivot)
			{
				i2 -= 1;
			};

			if (i1 < i2)
			{
				// Swap indices
				{
					I32 temp = indices[i1];
					indices[i1] = indices[i2 - 1];
					indices[i2 - 1] = temp;
				}

				// Swap centers
				{
					Vector2 temp = centers[i1];
					centers[i1] = centers[i2 - 1];
					centers[i2 - 1] = temp;
				}

				i1 += 1;
				i2 -= 1;
			}
		}
	}
	else
	{
		F32 pivot = c.y;

		while (i1 < i2)
		{
			while (i1 < i2 && centers[i1].y < pivot)
			{
				i1 += 1;
			};

			while (i1 < i2 && centers[i2 - 1].y >= pivot)
			{
				i2 -= 1;
			};

			if (i1 < i2)
			{
				// Swap indices
				{
					I32 temp = indices[i1];
					indices[i1] = indices[i2 - 1];
					indices[i2 - 1] = temp;
				}

				// Swap centers
				{
					Vector2 temp = centers[i1];
					centers[i1] = centers[i2 - 1];
					centers[i2 - 1] = temp;
				}

				i1 += 1;
				i2 -= 1;
			}
		}
	}

	if (i1 > 0 && i1 < count) { return i1; }
	else { return count / 2; }
}

void DynamicTree::ShiftOrigin(Vector2 newOrigin)
{
	for (U32 i = 0; i < nodeCapacity; ++i)
	{
		TreeNode* n = nodes + i;
		n->aabb.lowerBound.x -= newOrigin.x;
		n->aabb.lowerBound.y -= newOrigin.y;
		n->aabb.upperBound.x -= newOrigin.x;
		n->aabb.upperBound.y -= newOrigin.y;
	}
}

I32 DynamicTree::GetUserData(I32 proxyId) const
{
	return nodes[proxyId].userData;
}

AABB DynamicTree::GetAABB(I32 proxyId) const
{
	return nodes[proxyId].aabb;
}



//Broadphase

DynamicTree Broadphase::trees[BODY_TYPE_COUNT];
I32 Broadphase::proxyCount;

Hashset<I32> Broadphase::moveSet(16);
Vector<I32> Broadphase::moveArray(16);

MoveResult* Broadphase::moveResults;
MovePair* Broadphase::movePairs;
I32 Broadphase::movePairCapacity;
I32_Atomic Broadphase::movePairIndex;

Hashset<U64> Broadphase::pairSet(32);

void Broadphase::Initialize()
{
	proxyCount = 0;
	moveResults = nullptr;
	movePairs = nullptr;
	movePairCapacity = 0;
	movePairIndex = 0;

	for (I32 i = 0; i < BODY_TYPE_COUNT; ++i) { trees[i].Create(); }
}

void Broadphase::Shutdown()
{
	for (I32 i = 0; i < BODY_TYPE_COUNT; ++i) { trees[i].Destroy(); }

	moveSet.Destroy();
	moveArray.Destroy();
	pairSet.Destroy();
}

I32 Broadphase::CreateProxy(BodyType proxyType, AABB aabb, U64 categoryBits, I32 shapeIndex, bool forcePairCreation)
{
	I32 proxyId = trees[proxyType].CreateProxy(aabb, categoryBits, shapeIndex);
	I32 proxyKey = PROXY_KEY(proxyId, proxyType);
	if (proxyType != BODY_TYPE_STATIC || forcePairCreation) { BufferMove(proxyKey); }
	return proxyKey;
}

void Broadphase::DestroyProxy(I32 proxyKey)
{
	UnBufferMove(proxyKey);

	--proxyCount;

	BodyType proxyType = PROXY_TYPE(proxyKey);
	I32 proxyId = PROXY_ID(proxyKey);

	trees[proxyType].DestroyProxy(proxyId);
}

void Broadphase::MoveProxy(I32 proxyKey, AABB aabb)
{
	BodyType proxyType = PROXY_TYPE(proxyKey);
	I32 proxyId = PROXY_ID(proxyKey);

	trees[proxyType].MoveProxy(proxyId, aabb);
	BufferMove(proxyKey);
}

void Broadphase::UnBufferMove(I32 proxyKey)
{
	moveSet.Remove(proxyKey + 1);
}

void Broadphase::EnlargeProxy(I32 proxyKey, AABB aabb)
{
	I32 typeIndex = PROXY_TYPE(proxyKey);
	I32 proxyId = PROXY_ID(proxyKey);

	trees[typeIndex].EnlargeProxy(proxyId, aabb);
	BufferMove(proxyKey);
}

void Broadphase::RebuildTrees()
{
	trees[BODY_TYPE_DYNAMIC].Rebuild(false);
	trees[BODY_TYPE_KINEMATIC].Rebuild(false);
}

I32 Broadphase::GetShapeIndex(I32 proxyKey)
{
	I32 typeIndex = PROXY_TYPE(proxyKey);
	I32 proxyId = PROXY_ID(proxyKey);

	return trees[typeIndex].GetUserData(proxyId);
}

void Broadphase::Update()
{
	I32 moveCount = (I32)moveSet.Size();

	if (moveCount == 0) { return; }

	// todo these could be in the step context
	Memory::AllocateArray(&moveResults, moveCount);
	movePairCapacity = 16 * moveCount;

	Memory::AllocateArray(&movePairs, movePairCapacity);
	movePairIndex = 0;

	I32 minRange = 64;
	FindPairs(0, moveCount);

	// Single-threaded work
	// - Clear move flags
	// - Create contacts in deterministic order
	for (I32 i = 0; i < moveCount; ++i)
	{
		MoveResult* result = moveResults + i;
		MovePair* pair = result->pairList;
		while (pair != nullptr)
		{
			I32 shapeIdA = pair->shapeIndexA;
			I32 shapeIdB = pair->shapeIndexB;

			Shape& shapeA = Physics::shapes[shapeIdA];
			Shape& shapeB = Physics::shapes[shapeIdB];

			Physics::CreateContact(shapeA, shapeB);

			if (pair->heap)
			{
				MovePair* temp = pair;
				pair = pair->next;
				Memory::Free(&temp);
			}
			else
			{
				pair = pair->next;
			}
		}
	}

	// Reset move buffer
	moveSet.Clear();

	Memory::Free(&movePairs);
	Memory::Free(&moveResults);
}

void Broadphase::FindPairs(I32 startIndex, I32 endIndex)
{
	QueryPairContext queryContext;

	for (I32 i = startIndex; i < endIndex; ++i)
	{
		// Initialize move result for this moved proxy
		queryContext.moveResult = moveResults + i;
		queryContext.moveResult->pairList = nullptr;

		I32 proxyKey = moveArray[i];
		if (proxyKey == NullIndex) { continue; }

		BodyType proxyType = PROXY_TYPE(proxyKey);

		I32 proxyId = PROXY_ID(proxyKey);
		queryContext.queryProxyKey = proxyKey;

		const DynamicTree* baseTree = trees + proxyType;

		// We have to query the tree with the fat AABB so that
		// we don't fail to create a contact that may touch later.
		AABB fatAABB = baseTree->GetAABB(proxyId);
		queryContext.queryShapeIndex = baseTree->GetUserData(proxyId);

		// Query trees. Only dynamic proxies collide with kinematic and static proxies.
		// Using b2_defaultMaskBits so that b2Filter::groupIndex works.
		if (proxyType == BODY_TYPE_DYNAMIC)
		{
			// consider using bits = groupIndex > 0 ? b2_defaultMaskBits : maskBits
			queryContext.queryTreeType = BODY_TYPE_KINEMATIC;
			trees[BODY_TYPE_KINEMATIC].Query(fatAABB, U64_MAX, queryContext);

			queryContext.queryTreeType = BODY_TYPE_STATIC;
			trees[BODY_TYPE_STATIC].Query(fatAABB, U64_MAX, queryContext);
		}

		// All proxies collide with dynamic proxies
		// Using b2_defaultMaskBits so that b2Filter::groupIndex works.
		queryContext.queryTreeType = BODY_TYPE_DYNAMIC;
		trees[BODY_TYPE_DYNAMIC].Query(fatAABB, U64_MAX, queryContext);
	}
}

bool Broadphase::PairQueryCallback(I32 proxyId, I32 shapeId, QueryPairContext& context)
{
	I32 proxyKey = PROXY_KEY(proxyId, context.queryTreeType);

	// A proxy cannot form a pair with itself.
	if (proxyKey == context.queryProxyKey)
	{
		return true;
	}

	// Is this proxy also moving?
	if (context.queryTreeType != BODY_TYPE_STATIC && proxyKey < context.queryProxyKey)
	{
		bool moved = moveSet.Contains(proxyKey + 1);
		if (moved) { return true; }
	}

	U64 pairKey = SHAPE_PAIR_KEY(shapeId, context.queryShapeIndex);
	if (pairSet.Contains(pairKey)) { return true; }

	I32 shapeIdA, shapeIdB;
	if (proxyKey < context.queryProxyKey)
	{
		shapeIdA = shapeId;
		shapeIdB = context.queryShapeIndex;
	}
	else
	{
		shapeIdA = context.queryShapeIndex;
		shapeIdB = shapeId;
	}

	Shape& shapeA = Physics::shapes[shapeIdA];
	Shape& shapeB = Physics::shapes[shapeIdB];

	I32 bodyIdA = shapeA.bodyId;
	I32 bodyIdB = shapeB.bodyId;

	// Are the shapes on the same body?
	if (bodyIdA == bodyIdB) { return true; }

	if (!shapeA.filter.ShouldShapesCollide(shapeB.filter)) { return true; }

	// Sensors don't collide with other sensors
	if (shapeA.isSensor && shapeB.isSensor) { return true; }

	// Does a joint override collision?
	RigidBody2D& bodyA = Physics::rigidBodies[bodyIdA];
	RigidBody2D& bodyB = Physics::rigidBodies[bodyIdB];
	if (!Physics::ShouldBodiesCollide(bodyA, bodyB)) { return true; }

	//TODO: Custom filtering
	//if (customFilterFn)
	//{
	//	bool shouldCollide = customFilterFn(shapeIdA + 1, shapeIdB + 1, customFilterContext);
	//	if (!shouldCollide) { return true; }
	//}

	// #todo per thread to eliminate atomic?
	I32 pairIndex = movePairIndex.fetch_add(1);

	MovePair* pair;
	if (pairIndex < movePairCapacity)
	{
		pair = movePairs + pairIndex;
		pair->heap = false;
	}
	else
	{
		Memory::Allocate(&pair);
		pair->heap = true;
	}

	pair->shapeIndexA = shapeIdA;
	pair->shapeIndexB = shapeIdB;
	pair->next = context.moveResult->pairList;
	context.moveResult->pairList = pair;

	// continue the query
	return true;
}

bool Broadphase::ContinuousQueryCallback(int shapeId, ContinuousContext& context)
{
	Shape* fastShape = context.fastShape;
	BodySim* fastBodySim = context.fastBodySim;

	// Skip same shape
	if (shapeId == fastShape->id) { return true; }

	Shape& shape = Physics::shapes[shapeId];

	// Skip same body
	if (shape.bodyId == fastShape->bodyId) { return true; }

	// Skip filtered shapes
	bool canCollide = fastShape->filter.ShouldShapesCollide(shape.filter);
	if (canCollide == false) { return true; }

	// Skip sensors
	if (shape.isSensor) { return true; }

	RigidBody2D& body = Physics::rigidBodies[shape.bodyId];
	BodySim& bodySim = Physics::GetBodySim(body);

	// Skip bullets
	if (bodySim.isBullet) { return true; }

	// Skip filtered bodies
	RigidBody2D& fastBody = Physics::rigidBodies[fastBodySim->bodyId];
	canCollide = Physics::ShouldBodiesCollide(fastBody, body);
	if (canCollide == false) { return true; }

	//TODO: Custom filtering
	//if (customFilterFcn)
	//{
	//	canCollide = customFilterFcn(shape.id + 1, fastShape->id + 1, world->customFilterContext);
	//	if (canCollide == false) { return true; }
	//}

	// Prevent pausing on chain segment junctions
	if (shape.type == SHAPE_TYPE_CHAIN_SEGMENT)
	{
		Transform2D transform = bodySim.transform;
		Vector2 p1 = shape.chainSegment.segment.point1 * transform;
		Vector2 p2 = shape.chainSegment.segment.point2 * transform;
		Vector2 e = p2 - p1;
		Vector2 c1 = context.centroid1;
		Vector2 c2 = context.centroid2;
		F32 offset1 = (c1 - p1).Cross(e);
		F32 offset2 = (c2 - p1).Cross(e);

		if (offset1 < 0.0f || offset2 > 0.0f) { return true; }
	}

	TOIInput input;
	input.proxyA = Physics::MakeShapeDistanceProxy(shape);
	input.proxyB = Physics::MakeShapeDistanceProxy(*fastShape);
	input.sweepA.Create(bodySim);
	input.sweepB = context.sweep;
	input.tMax = context.fraction;

	TOIOutput output = Physics::TimeOfImpact(input);
	if (0.0f < output.t && output.t < context.fraction)
	{
		context.fraction = output.t;
	}
	else if (0.0f == output.t)
	{
		// fallback to TOI of a small circle around the fast shape centroid
		Vector2 centroid = fastShape->GetShapeCentroid();
		input.proxyB = MakeProxy(&centroid, 1, SpeculativeDistance);
		output = Physics::TimeOfImpact(input);
		if (0.0f < output.t && output.t < context.fraction)
		{
			context.fraction = output.t;
		}
	}

	return true;
}

DistanceProxy Broadphase::MakeProxy(const Vector2* vertices, I32 count, F32 radius)
{
	count = Math::Min(count, (I32)MaxPolygonVertices);
	DistanceProxy proxy;
	for (int i = 0; i < count; ++i)
	{
		proxy.points[i] = vertices[i];
	}
	proxy.count = count;
	proxy.radius = radius;
	return proxy;
}

bool Broadphase::TestOverlap(I32 proxyKeyA, I32 proxyKeyB)
{
	I32 typeIndexA = PROXY_TYPE(proxyKeyA);
	I32 proxyIdA = PROXY_ID(proxyKeyA);
	I32 typeIndexB = PROXY_TYPE(proxyKeyB);
	I32 proxyIdB = PROXY_ID(proxyKeyB);

	AABB aabbA = trees[typeIndexA].GetAABB(proxyIdA);
	AABB aabbB = trees[typeIndexB].GetAABB(proxyIdB);
	return aabbA.Contains(aabbB);
}

void Broadphase::BufferMove(I32 queryProxy)
{
	moveSet.Insert(queryProxy);
}
