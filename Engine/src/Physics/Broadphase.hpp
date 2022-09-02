#pragma once

template<typename> struct Vector;
struct PhysicsObject2D;
struct Box;

class Broadphase
{
public:
	~Broadphase();

	virtual void InsertObj(PhysicsObject2D* obj) = 0;
	virtual void RemoveObj(PhysicsObject2D* obj) = 0;
	virtual void UpdateObj(PhysicsObject2D* obj) = 0;

	virtual void Query(PhysicsObject2D* obj, Vector<PhysicsObject2D*>& result) = 0;
	virtual void Query(const Box& box, Vector<PhysicsObject2D*>& result) = 0;
	virtual void Raycast(PhysicsObject2D* result) = 0;
	virtual void RaycastAll(Vector<PhysicsObject2D*>& result) = 0;
};