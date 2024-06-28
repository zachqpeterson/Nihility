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

import Containers;
import Memory;

static constexpr inline U32 NullNode = U32_MAX;

struct TreeNode
{
	AABB aabb;

	U32 child1;
	U32 child2;
	U32 height;

	ColliderProxy* data;

	union
	{
		U32 parent;
		U32 next;
	};
};

struct ProxyPair
{
	U32 proxyIdA;
	U32 proxyIdB;
};

class Broadphase
{
private:
	static void Initialize();
	static void Shutdown();

	static U32 CreateProxy(const AABB& aabb, ColliderProxy* data);
	static void DestroyProxy(U32 proxyId);
	static void MoveProxy(U32 proxyId, const AABB& aabb, const Vector2& displacement);
	static void TouchProxy(U32 proxyId);
	static const AABB& GetFatAABB(U32 proxyId);
	static bool TestOverlap(U32 proxyIdA, U32 proxyIdB);
	static U32 GetProxyCount();
	static void UpdatePairs();
	static void Query(const AABB& aabb, U32 id);

	static void BufferMove(U32 proxyId);
	static void UnBufferMove(U32 proxyId);

	//Tree
	static U32 AllocateNode();
	static void FreeNode(U32 nodeId);

	static void InsertLeaf(U32 leaf);
	static void RemoveLeaf(U32 leaf);

	static U32 Balance(U32 index);

	static U32 ComputeHeight();
	static U32 ComputeHeight(U32 nodeId);

	static U32 proxyCount;

	static Vector<U32> moveBuffer;
	static Vector<ProxyPair> pairBuffer;

	//Tree
	static TreeNode* nodes;
	static U32 root;
	static U32 nodeCount;
	static U32 nodeCapacity;
	static U32 freeList;
	static U32 path;

	STATIC_CLASS(Broadphase);
	friend class Physics;
	friend struct RigidBody2D;
};