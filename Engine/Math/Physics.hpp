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

struct Scene;

class NH_API Physics
{
public:
	static Collider2D CreateBoxCollider2D(const ColliderInfo& info, F32 halfWidth, F32 halfHeight);
	static Collider2D CreateCircleCollider2D(const ColliderInfo& info, F32 radius);
	static Collider2D CreatePolygonCollider2D(const ColliderInfo& info, U8 vertexCount, Vector2* vertices);

	static const Vector3& Gravity();

	static bool TestOverlap(const Collider2D* shapeA, const Collider2D* shapeB, const Transform2D& xfA, const Transform2D& xfB);

private:
	enum Flags
	{
		FLAG_NEW_FIXTURE = 0x0001,
		FLAG_LOCKED = 0x0002,
		FLAG_CLEAR_FORCES = 0x0004
	};

	static bool Initialize();
	static void Shutdown();

	static void SetScene(Scene* scene);
	static void Update(F32 step);
	static void Solve(F32 step);
	static void SolveTOI(F32 step);
	static void TimeOfImpact(TOIOutput* output, const TOIInput* input);
	static void DetectCollision();

	static void Synchronize(Collider2D& collider, const Transform2D& transformA, const Transform2D& transformB);
	static void AddPair(ColliderProxy* proxyA, ColliderProxy* proxyB);

	static void Distance(DistanceOutput* output, SimplexCache* cache, const DistanceInput* input);

	static Vector<RigidBody2D>* bodies;
	static Contact2D* contacts;
	static U64 contactCapacity;
	static Freelist contactFreelist;
	static Vector3 gravity;
	static U32 flags;
	static bool stepComplete;

	STATIC_CLASS(Physics);
	friend class Engine;
	friend class Broadphase;
	friend struct Scene;
	friend struct RigidBody2D;
};