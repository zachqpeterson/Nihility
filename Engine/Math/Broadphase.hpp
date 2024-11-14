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

#pragma once

#include "PhysicsDefines.hpp"

import ThreadSafety;
import Containers;
import Memory;

static constexpr inline I32 NullNode = -1;
static constexpr inline U64 DefaultLayerMask = U64_MAX;

enum RotateType
{
	ROTATE_TYPE_NONE,
	ROTATE_TYPE_BF,
	ROTATE_TYPE_BG,
	ROTATE_TYPE_CD,
	ROTATE_TYPE_CE
};

struct TreeNode
{
	AABB aabb;

	U64 layers;

	union
	{
		I32 parent;
		I32 next;
	};

	I32 child1;
	I32 child2;
	I32 userData;
	I32 height;
	bool enlarged;
};

struct DynamicTree
{
public:
	void Create();
	void Destroy();

	I32 AllocateNode();
	void FreeNode(I32 nodeId);
	I32 FindBestSibling(AABB box);
	void RotateNodes(I32 iA);
	void InsertLeaf(I32 leaf, bool shouldRotate);
	void RemoveLeaf(I32 leaf);

	I32 CreateProxy(const AABB& aabb, U64 layers, I32 userData);
	void DestroyProxy(I32 proxyId);
	void MoveProxy(I32 proxyId, const AABB& aabb);
	void EnlargeProxy(I32 proxyId, const AABB& aabb);
	bool AABBOverlaps(AABB a, AABB b);
	void Query(const AABB& aabb, U64 layerMask, QueryPairContext& context);
	void QueryContinuous(const AABB& aabb, U64 layerMask, ContinuousContext& context);
	void Raycast(RaycastInput input, U64 layerMask);
	void Shapecast(ShapeCastInput input, U64 layerMask);
	I32 GetHeight();
	I32 ComputeHeight(I32 nodeId);
	I32 ComputeHeight();
	I32 GetMaxBalance();
	F32 GetAreaRatio();
	I32 GetProxyCount();
	I32 Rebuild(bool fullBuild);
	I32 BuildTree(I32 leafCount);
	I32 PartitionMid(I32* indices, Vector2* centers, I32 count);
	void ShiftOrigin(Vector2 newOrigin);
	I32 GetUserData(I32 proxyId) const;
	AABB GetAABB(I32 proxyId) const;

public:
	TreeNode* nodes;
	I32 root;
	I32 nodeCount;
	U32 nodeCapacity;
	I32 freeList;
	I32 proxyCount;
	I32* leafIndices;
	AABB* leafBoxes;
	Vector2* leafCenters;
	I32* binIndices;
	I32 rebuildCapacity;
};

class Broadphase
{
private:
	static void Initialize();
	static void Shutdown();

	static void Update();
	static void FindPairs(I32 startIndex, I32 endIndex);
	static bool PairQueryCallback(I32 proxyId, I32 shapeId, QueryPairContext& context);
	static bool ContinuousQueryCallback(I32 shapeId, ContinuousContext& context);
	static DistanceProxy MakeProxy(const Vector2* vertices, I32 count, F32 radius);

	static I32 CreateProxy(BodyType proxyType, AABB aabb, U64 categoryBits, I32 shapeIndex, bool forcePairCreation);
	static void DestroyProxy(I32 proxyKey);
	static void MoveProxy(I32 proxyKey, AABB aabb);
	static void UnBufferMove(I32 proxyKey);
	static void EnlargeProxy(I32 proxyKey, AABB aabb);
	static void RebuildTrees();
	static I32 GetShapeIndex(I32 proxyKey);
	static bool TestOverlap(I32 proxyKeyA, I32 proxyKeyB);
	static void BufferMove(I32 queryProxy);

	static DynamicTree trees[BODY_TYPE_COUNT];
	static I32 proxyCount;

	static Hashset<I32> moveSet;
	static Vector<I32> moveArray;

	static MoveResult* moveResults;
	static MovePair* movePairs;
	static I32 movePairCapacity;
	static I32_Atomic movePairIndex;

	static Hashset<U64> pairSet;

	STATIC_CLASS(Broadphase);
	friend class Physics;
	friend struct DynamicTree;
};