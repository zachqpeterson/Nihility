#pragma once

#include "Physics/Broadphase.hpp"
#include "Physics/Physics.hpp"
#include "Containers/BoolTable.hpp"

#include <Containers/Vector.hpp>
#include <Containers/List.hpp>

struct Tile;

class GridBroadphase : public Broadphase
{
public:
	GridBroadphase(Tile** grid, U16 width, U16 height);
	~GridBroadphase();
	void Destroy();
	void* operator new(U64 size);
	void operator delete(void* ptr);

	void InsertObj(PhysicsObject2D* obj) final;
	void RemoveObj(PhysicsObject2D* obj) final;
	void UpdateObj(PhysicsObject2D* obj) final;

	void Update(List<List<Contact2D>>& contacts) final;

	bool Query(PhysicsObject2D* obj, List<Contact2D>& contacts) final;
	bool Query(const Box& box, Vector<PhysicsObject2D*>& result) final;
	
	bool RaycastQuery(PhysicsObject2D* obj, PhysicsObject2D* result) final;
	bool RaycastQueryAll(Raycast& ray, Vector<PhysicsObject2D*>& results) final;

private:
	List<PhysicsObject2D*> objs;

	U64 width;
	U64 height;
	Tile** grid;

	BoolTable collisions;
};