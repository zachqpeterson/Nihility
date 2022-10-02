#pragma once

#include "Defines.hpp"

template<typename> struct Vector;
template<typename> struct List;
struct PhysicsObject2D;
struct Box;
struct Raycast;
struct Contact2D;

class Broadphase
{
public:
	virtual ~Broadphase() {}

	virtual void InsertObj(PhysicsObject2D* obj) = 0;
	virtual void RemoveObj(PhysicsObject2D* obj) = 0;
	virtual void UpdateObj(PhysicsObject2D* obj) = 0;

	virtual void Update(List<List<Contact2D>>& contacts) = 0;

	virtual bool Query(PhysicsObject2D* obj, List<Contact2D>& contacts) = 0;
	virtual bool Query(const Box& box, Vector<PhysicsObject2D*>& result) = 0;
	virtual bool RaycastQuery(PhysicsObject2D* obj, PhysicsObject2D* result) = 0;
	virtual bool RaycastQueryAll(Raycast& ray, Vector<PhysicsObject2D*>& results) = 0;
};