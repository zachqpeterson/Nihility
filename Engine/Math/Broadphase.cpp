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
#include <type_traits>

import Containers;

I32 Broadphase::proxyCount;

Vector<I32> Broadphase::moveBuffer(16);
Vector<ProxyPair> Broadphase::pairBuffer(16);

//Tree
TreeNode* Broadphase::nodes;
I32 Broadphase::root;
I32 Broadphase::nodeCount;
U32 Broadphase::nodeCapacity;
I32 Broadphase::freeList;
I32 Broadphase::path;

void Broadphase::Initialize()
{
	proxyCount = 0;

	//Tree init
	root = NullNode;
	Memory::AllocateArray(&nodes, 16, nodeCapacity);

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
	path = 0;
}

void Broadphase::Shutdown()
{
	pairBuffer.Destroy();
	moveBuffer.Destroy();
	Memory::Free(&nodes);
}

I32 Broadphase::CreateProxy(const AABB& aabb, ColliderProxy* data)
{
	I32 proxyId = AllocateNode();

	// Fatten the aabb.
	Vector2 r{ AABBExtension, AABBExtension };
	nodes[proxyId].aabb.lowerBound = aabb.lowerBound - r;
	nodes[proxyId].aabb.upperBound = aabb.upperBound + r;
	nodes[proxyId].data = data;
	nodes[proxyId].height = 0;
	nodes[proxyId].moved = true;

	InsertLeaf(proxyId);

	++proxyCount;
	BufferMove(proxyId);
	return proxyId;
}

void Broadphase::DestroyProxy(I32 proxyId)
{
	UnBufferMove(proxyId);
	--proxyCount;
	RemoveLeaf(proxyId);
	FreeNode(proxyId);
}

void Broadphase::MoveProxy(I32 proxyId, const AABB& aabb, const Vector2& displacement)
{
	AABB fatAABB;
	Vector2 r{ AABBExtension, AABBExtension };
	fatAABB.lowerBound = fatAABB.lowerBound - r;
	fatAABB.upperBound = fatAABB.upperBound + r;

	Vector2 d = AABBMultiplier * displacement;

	if (d.x < 0.0f) { fatAABB.lowerBound.x += d.x; }
	else { fatAABB.upperBound.x += d.x; }

	if (d.y < 0.0f) { fatAABB.lowerBound.y += d.y; }
	else { fatAABB.upperBound.y += d.y; }

	const AABB& treeAABB = nodes[proxyId].aabb;

	if(treeAABB.Contains(aabb))
	{
		AABB hugeAABB;
		hugeAABB.lowerBound = fatAABB.lowerBound - 4.0f * r;
		hugeAABB.upperBound = fatAABB.upperBound + 4.0f * r;

		if (hugeAABB.Contains(treeAABB)) { return; }
	}

	RemoveLeaf(proxyId);

	nodes[proxyId].aabb = fatAABB;

	InsertLeaf(proxyId);

	nodes[proxyId].moved = true;

	BufferMove(proxyId);
}

void Broadphase::TouchProxy(I32 proxyId)
{
	BufferMove(proxyId);
}

const AABB& Broadphase::GetFatAABB(I32 proxyId)
{
	return nodes[proxyId].aabb;
}

bool TestOverlapAABB(const AABB& a, const AABB& b)
{
	Vector2 d1, d2;
	d1 = b.lowerBound - a.upperBound;
	d2 = a.lowerBound - b.upperBound;

	if (d1.x > 0.0f || d1.y > 0.0f || d2.x > 0.0f || d2.y > 0.0f) { return false; }

	return true;
}

bool Broadphase::TestOverlap(I32 proxyIdA, I32 proxyIdB)
{
	const AABB& aabbA = GetFatAABB(proxyIdA);
	const AABB& aabbB = GetFatAABB(proxyIdB);
	return TestOverlapAABB(aabbA, aabbB);
}

I32 Broadphase::GetProxyCount()
{
	return proxyCount;
}

void Broadphase::UpdatePairs()
{
	// Reset pair buffer
	pairBuffer.Clear();

	// Perform tree queries for all moving proxies.
	for (I32 id : moveBuffer)
	{
		if (id == NullNode || nodes[id].data == nullptr) { continue; }

		// We have to query the tree with the fat AABB so that
		// we don't fail to create a pair that may touch later.
		const AABB& fatAABB = GetFatAABB(id);

		// Query tree, create pairs and add them pair buffer.
		Query(fatAABB, id);
	}

	// Send the pairs back to the client.
	for (ProxyPair* pair = pairBuffer.begin(); pair != pairBuffer.end(); ++pair)
	{
		Physics::AddPair(nodes[pair->proxyIdA].data, nodes[pair->proxyIdB].data);

		//Skip duplicates
		for (ProxyPair* otherPair = pair; otherPair != pairBuffer.end(); ++otherPair)
		{
			if (otherPair->proxyIdA != pair->proxyIdA || otherPair->proxyIdB != pair->proxyIdB) { pair = otherPair; break; }
		}
	}

	for (I32 id : moveBuffer)
	{
		if (id == NullNode) { continue; }

		nodes[id].moved = false;
	}

	// Reset move buffer
	moveBuffer.Clear();
}

bool PairLessThan(const ProxyPair& pair1, const ProxyPair& pair2)
{
	if (pair1.proxyIdA < pair2.proxyIdA) { return true; }

	if (pair1.proxyIdA == pair2.proxyIdA) { return pair1.proxyIdB < pair2.proxyIdB; }

	return false;
}

void Broadphase::Query(const AABB& aabb, I32 id)
{
	Stack<I32> stack(256);
	stack.Push(root);

	I32 nodeId;
	while (stack.Pop(nodeId))
	{
		if (nodeId == NullNode) { continue; }

		const TreeNode* node = nodes + nodeId;

		if (TestOverlapAABB(node->aabb, aabb))
		{
			if (node->child1 == NullNode)
			{
				if (nodeId == id) { continue; }

				const bool moved = nodes[nodeId].moved;

				if (moved && nodeId > id || node->data == nullptr) { continue; }

				if (node->data->collider->body->colliders.Size() > 2) { BreakPoint; }

				pairBuffer.SortedInsert(PairLessThan, {Math::Min(nodeId, id), Math::Max(nodeId, id)});
			}
			else
			{
				stack.Push(node->child1);
				stack.Push(node->child2);
			}
		}
	}
}

void Broadphase::BufferMove(I32 proxyId)
{
	moveBuffer.Push(proxyId);
}

void Broadphase::UnBufferMove(I32 proxyId)
{
	for (I32& id : moveBuffer)
	{
		if (id == proxyId) { id = NullNode; }
	}
}

//Tree

I32 Broadphase::AllocateNode()
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
	nodes[nodeId].parent = NullNode;
	nodes[nodeId].child1 = NullNode;
	nodes[nodeId].child2 = NullNode;
	nodes[nodeId].height = 0;
	nodes[nodeId].moved = false;
	nodes[nodeId].data = nullptr;
	++nodeCount;
	return nodeId;
}

void Broadphase::FreeNode(I32 nodeId)
{
	nodes[nodeId].next = freeList;
	nodes[nodeId].height = -1;
	freeList = nodeId;
	--nodeCount;
}

void Broadphase::InsertLeaf(I32 leaf)
{
	if (root == NullNode)
	{
		root = leaf;
		nodes[root].parent = NullNode;
		return;
	}

	// Find the best sibling for this node
	AABB leafAABB = nodes[leaf].aabb;
	I32 index = root;
	while (nodes[index].child1 != NullNode)
	{
		I32 child1 = nodes[index].child1;
		I32 child2 = nodes[index].child2;

		F32 area = nodes[index].aabb.GetPerimeter();

		AABB combinedAABB;
		combinedAABB.Combine(nodes[index].aabb, leafAABB);
		F32 combinedArea = combinedAABB.GetPerimeter();

		// Cost of creating a new parent for this node and the new leaf
		F32 cost = 2.0f * combinedArea;

		// Minimum cost of pushing the leaf further down the tree
		F32 inheritanceCost = 2.0f * (combinedArea - area);

		// Cost of descending into child1
		F32 cost1;
		if (nodes[child1].child1 == NullNode)
		{
			AABB aabb;
			aabb.Combine(leafAABB, nodes[child1].aabb);
			cost1 = aabb.GetPerimeter() + inheritanceCost;
		}
		else
		{
			AABB aabb;
			aabb.Combine(leafAABB, nodes[child1].aabb);
			F32 oldArea = nodes[child1].aabb.GetPerimeter();
			F32 newArea = aabb.GetPerimeter();
			cost1 = (newArea - oldArea) + inheritanceCost;
		}

		// Cost of descending into child2
		F32 cost2;
		if (nodes[child2].child1 == NullNode)
		{
			AABB aabb;
			aabb.Combine(leafAABB, nodes[child2].aabb);
			cost2 = aabb.GetPerimeter() + inheritanceCost;
		}
		else
		{
			AABB aabb;
			aabb.Combine(leafAABB, nodes[child2].aabb);
			F32 oldArea = nodes[child2].aabb.GetPerimeter();
			F32 newArea = aabb.GetPerimeter();
			cost2 = newArea - oldArea + inheritanceCost;
		}

		// Descend according to the minimum cost.
		if (cost < cost1 && cost < cost2) { break; }

		// Descend
		if (cost1 < cost2) { index = child1; }
		else { index = child2; }
	}

	I32 sibling = index;

	// Create a new parent.
	I32 oldParent = nodes[sibling].parent;
	I32 newParent = AllocateNode();
	nodes[newParent].parent = oldParent;
	nodes[newParent].aabb.Combine(leafAABB, nodes[sibling].aabb);
	nodes[newParent].height = nodes[sibling].height + 1;

	if (oldParent != NullNode)
	{
		// The sibling was not the root.
		if (nodes[oldParent].child1 == sibling) { nodes[oldParent].child1 = newParent; }
		else { nodes[oldParent].child2 = newParent; }

		nodes[newParent].child1 = sibling;
		nodes[newParent].child2 = leaf;
		nodes[sibling].parent = newParent;
		nodes[leaf].parent = newParent;
	}
	else
	{
		// The sibling was the root.
		nodes[newParent].child1 = sibling;
		nodes[newParent].child2 = leaf;
		nodes[sibling].parent = newParent;
		nodes[leaf].parent = newParent;
		root = newParent;
	}

	// Walk back up the tree fixing heights and AABBs
	index = nodes[leaf].parent;
	while (index != NullNode)
	{
		index = Balance(index);

		I32 child1 = nodes[index].child1;
		I32 child2 = nodes[index].child2;

		nodes[index].height = 1 + Math::Max(nodes[child1].height, nodes[child2].height);
		nodes[index].aabb.Combine(nodes[child1].aabb, nodes[child2].aabb);

		index = nodes[index].parent;
	}
}

void Broadphase::RemoveLeaf(I32 leaf)
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
		// Destroy parent and connect sibling to grandParent.
		if (nodes[grandParent].child1 == parent) { nodes[grandParent].child1 = sibling; }
		else { nodes[grandParent].child2 = sibling; }
		nodes[sibling].parent = grandParent;
		FreeNode(parent);

		// Adjust ancestor bounds.
		I32 index = grandParent;
		while (index != NullNode)
		{
			index = Balance(index);

			I32 child1 = nodes[index].child1;
			I32 child2 = nodes[index].child2;

			nodes[index].aabb.Combine(nodes[child1].aabb, nodes[child2].aabb);
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

I32 Broadphase::Balance(I32 iA)
{
	TreeNode* A = nodes + iA;
	if (A->child1 == NullNode || A->height < 2) { return iA; }

	I32 iB = A->child1;
	I32 iC = A->child2;

	TreeNode* B = nodes + iB;
	TreeNode* C = nodes + iC;

	I32 balance = C->height - B->height;

	// Rotate C up
	if (balance > 1)
	{
		I32 iF = C->child1;
		I32 iG = C->child2;
		TreeNode* F = nodes + iF;
		TreeNode* G = nodes + iG;

		// Swap A and C
		C->child1 = iA;
		C->parent = A->parent;
		A->parent = iC;

		// A's old parent should point to C
		if (C->parent != NullNode)
		{
			if (nodes[C->parent].child1 == iA) { nodes[C->parent].child1 = iC; }
			else { nodes[C->parent].child2 = iC; }
		}
		else { root = iC; }

		// Rotate
		if (F->height > G->height)
		{
			C->child2 = iF;
			A->child2 = iG;
			G->parent = iA;
			A->aabb.Combine(B->aabb, G->aabb);
			C->aabb.Combine(A->aabb, F->aabb);

			A->height = 1 + Math::Max(B->height, G->height);
			C->height = 1 + Math::Max(A->height, F->height);
		}
		else
		{
			C->child2 = iG;
			A->child2 = iF;
			F->parent = iA;
			A->aabb.Combine(B->aabb, F->aabb);
			C->aabb.Combine(A->aabb, G->aabb);

			A->height = 1 + Math::Max(B->height, F->height);
			C->height = 1 + Math::Max(A->height, G->height);
		}

		return iC;
	}

	// Rotate B up
	if (balance < -1)
	{
		I32 iD = B->child1;
		I32 iE = B->child2;
		TreeNode* D = nodes + iD;
		TreeNode* E = nodes + iE;

		// Swap A and B
		B->child1 = iA;
		B->parent = A->parent;
		A->parent = iB;

		// A's old parent should point to B
		if (B->parent != NullNode)
		{
			if (nodes[B->parent].child1 == iA)
			{
				nodes[B->parent].child1 = iB;
			}
			else
			{
				nodes[B->parent].child2 = iB;
			}
		}
		else
		{
			root = iB;
		}

		// Rotate
		if (D->height > E->height)
		{
			B->child2 = iD;
			A->child1 = iE;
			E->parent = iA;
			A->aabb.Combine(C->aabb, E->aabb);
			B->aabb.Combine(A->aabb, D->aabb);

			A->height = 1 + Math::Max(C->height, E->height);
			B->height = 1 + Math::Max(A->height, D->height);
		}
		else
		{
			B->child2 = iE;
			A->child1 = iD;
			D->parent = iA;
			A->aabb.Combine(C->aabb, D->aabb);
			B->aabb.Combine(A->aabb, E->aabb);

			A->height = 1 + Math::Max(C->height, D->height);
			B->height = 1 + Math::Max(A->height, E->height);
		}

		return iB;
	}

	return iA;
}

I32 Broadphase::ComputeHeight()
{
	I32 height = ComputeHeight(root);
	return height;
}

I32 Broadphase::ComputeHeight(I32 nodeId)
{
	TreeNode* node = nodes + nodeId;

	if (node->child1 == NullNode) { return 0; }

	I32 height1 = ComputeHeight(node->child1);
	I32 height2 = ComputeHeight(node->child2);
	return 1 + Math::Max(height1, height2);
}