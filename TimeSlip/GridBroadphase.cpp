#include "GridBroadphase.hpp"

#include "Tile.hpp"

#include <Physics/Physics.hpp>

GridBroadphase::GridBroadphase(Tile** grid, U16 width, U16 height) : width{ width }, height{ height }, grid{ grid }
{

}

GridBroadphase::~GridBroadphase()
{
	Destroy();
}

void GridBroadphase::Destroy()
{

}

void* GridBroadphase::operator new(U64 size) { return Memory::Allocate(sizeof(GridBroadphase), MEMORY_TAG_DATA_STRUCT); }
void GridBroadphase::operator delete(void* ptr) { Memory::Free(ptr, sizeof(GridBroadphase), MEMORY_TAG_DATA_STRUCT); }

void GridBroadphase::InsertObj(PhysicsObject2D* obj)
{
	objs.PushBack(obj);
	collisions.Expand();

	Vector2 pos = obj->Transform()->Position();
	Vector2 ext = obj->Collider()->box.Extents();

	Vector2Int min = (Vector2Int)(pos - ext);
	Vector2Int max = (Vector2Int)(pos + ext);

	//TODO: Insert
}

void GridBroadphase::RemoveObj(PhysicsObject2D* obj)
{
	objs.Remove(obj);
}

void GridBroadphase::UpdateObj(PhysicsObject2D* obj)
{
	Vector2 pos = obj->Transform()->Position();
	Vector2 ext = obj->Collider()->box.Extents();

	Vector2Int min = (Vector2Int)(pos - ext);
	Vector2Int max = (Vector2Int)(pos + ext);

	//TODO: Remove

	pos += obj->Move();
	min = (Vector2Int)(pos - ext);
	max = (Vector2Int)(pos + ext);

	//TODO: Reinsert
}

void GridBroadphase::Update(List<List<Contact2D>>& contacts)
{
	for (PhysicsObject2D* obj : objs)
	{
		List<Contact2D> cs;
		if (Query(obj, cs))
		{
			contacts.PushBack(cs);
		}
	}
}

bool GridBroadphase::Query(PhysicsObject2D* obj, List<Contact2D>& contacts)
{
	Vector2 start = obj->Transform()->Position();
	Vector2 move = obj->Move();
	F32 length = move.Magnitude();
	Vector2 dir = move / length;

	Vector2 unitStepSize = { 1.0f / dir.x * Math::Sign(dir.x), 1.0f / dir.y * Math::Sign(dir.y) };
	Vector2 excess = Vector2::ONE / obj->Collider()->box.Size();

	Vector2Int mapCheck = (Vector2Int)start;
	Vector2 length1D;

	Vector2Int step;

	if (dir.x < 0.0f)
	{
		step.x = -1;
		length1D.x = (start.x + 1.0f - mapCheck.x) * unitStepSize.x;
	}
	else
	{
		step.x = 1;
		length1D.x = (mapCheck.x + 1.0f - start.x) * unitStepSize.x;
	}

	if (dir.y < 0.0f)
	{
		step.y = -1;
		length1D.y = (start.y + 1.0f - mapCheck.y) * unitStepSize.y;
	}
	else
	{
		step.y = 1;
		length1D.y = (mapCheck.y + 1.0f - start.y) * unitStepSize.y;
	}

	Vector2 extents = obj->Collider()->box.Extents();

	bool collidedX = Math::NaN(length1D.x);
	bool collidedY = Math::NaN(length1D.y);

	if (dir.x > 0.0f)
	{
		debugBreak();
	}

	U32 minX = (U32)(start.x - extents.x);
	U32 maxX = (U32)(start.x + extents.x);
	U32 minY = (U32)(start.y - extents.y);
	U32 maxY = (U32)(start.y + extents.y);

	U32& x = step.x > 0 ? maxX : minX;
	U32& y = step.y > 0 ? maxY : minY;

	F32 distance = 0.0f;
	while ((length1D.x < length + extents.x && !collidedX) || (length1D.y < length + extents.y && !collidedY))
	{
		if ((collidedY || length1D.x < length1D.y) && !collidedX)
		{
			mapCheck.x += step.x;
			distance = length1D.x;
			length1D.x += unitStepSize.x;
			minX += step.x;
			maxX += step.x;
		}
		else if (!collidedY)
		{
			mapCheck.y += step.y;
			distance = length1D.y;
			length1D.y += unitStepSize.y;
			minY += step.y;
			maxY += step.y;
		}

		for (U32 y = minY; y <= maxY && !collidedX; ++y)
		{
			if (x < width && y < height && grid[x][y].blockID)
			{
				collidedX = true;

				Contact2D c{};
				c.a = obj;
				c.distance = distance - unitStepSize.x * excess.x;
				c.normal = Vector2::RIGHT * step.x;
				c.relativeVelocity = obj->Move();
				c.restitution = obj->Restitution(); //TODO: get tile restitution;

				contacts.PushBack(c);
			}
		}

		for (U32 x = minX; x <= maxX && !collidedY; ++x)
		{
			if (x < width && y < height && grid[x][y].blockID)
			{
				collidedY = true;

				Contact2D c{};
				c.a = obj;
				c.distance = distance - unitStepSize.y * excess.y;
				c.normal = Vector2::UP * step.y;
				c.relativeVelocity = obj->Move();
				c.restitution = obj->Restitution(); //TODO: get tile restitution;

				contacts.PushBack(c);
			}
		}
	}

	return contacts.Size();
}

bool GridBroadphase::Query(const Box& box, Vector<PhysicsObject2D*>& result)
{
	return false;
}

bool GridBroadphase::RaycastQuery(PhysicsObject2D* obj, PhysicsObject2D* result)
{
	return false;
}

bool GridBroadphase::RaycastQueryAll(Raycast& ray, Vector<PhysicsObject2D*>& result)
{
	return false;
}
