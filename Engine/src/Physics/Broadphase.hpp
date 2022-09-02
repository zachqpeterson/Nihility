#pragma once

template<typename> struct Vector;
struct PhysicsObject2D;
struct Box;
struct Raycast;

class Broadphase
{
public:
	virtual ~Broadphase() {}

	virtual void InsertObj(PhysicsObject2D* obj) = 0;
	virtual void RemoveObj(PhysicsObject2D* obj) = 0;
	virtual void UpdateObj(PhysicsObject2D* obj) = 0;

	virtual void Query(PhysicsObject2D* obj, Vector<PhysicsObject2D*>& result) = 0;
	virtual void Query(const Box& box, Vector<PhysicsObject2D*>& result) = 0;
	virtual void RaycastQuery(Raycast& ray, PhysicsObject2D* result) = 0;
	virtual void RaycastQueryAll(Raycast& ray, Vector<PhysicsObject2D*>& result) = 0;
};