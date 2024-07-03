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

static constexpr inline I32 NullNode = -1;

struct TreeNode
{
	AABB aabb;

	I32 child1;
	I32 child2;
	I32 height;
	bool moved;

	ColliderProxy* data;

	union
	{
		I32 parent;
		I32 next;
	};
};

struct ProxyPair
{
	I32 proxyIdA;
	I32 proxyIdB;
};

class Broadphase
{
private:
	static void Initialize();
	static void Shutdown();

	static I32 CreateProxy(const AABB& aabb, ColliderProxy* data);
	static void DestroyProxy(I32 proxyId);
	static void MoveProxy(I32 proxyId, const AABB& aabb, const Vector2& displacement);
	static void TouchProxy(I32 proxyId);
	static const AABB& GetFatAABB(I32 proxyId);
	static bool TestOverlap(I32 proxyIdA, I32 proxyIdB);
	static I32 GetProxyCount();
	static void UpdatePairs();
	static void Query(const AABB& aabb, I32 id);

	static void BufferMove(I32 proxyId);
	static void UnBufferMove(I32 proxyId);

	//Tree
	static I32 AllocateNode();
	static void FreeNode(I32 nodeId);

	static void InsertLeaf(I32 leaf);
	static void RemoveLeaf(I32 leaf);

	static I32 Balance(I32 index);

	static I32 ComputeHeight();
	static I32 ComputeHeight(I32 nodeId);

	static I32 proxyCount;

	static Vector<I32> moveBuffer;
	static Vector<ProxyPair> pairBuffer;

	//Tree
	static TreeNode* nodes;
	static I32 root;
	static I32 nodeCount;
	static U32 nodeCapacity;
	static I32 freeList;
	static I32 path;

	STATIC_CLASS(Broadphase);
	friend class Physics;
	friend struct RigidBody2D;
};