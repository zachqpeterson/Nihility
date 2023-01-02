#include "GridBroadphase.hpp"

#include "Tile.hpp"

#include <Physics/Physics.hpp>
#include <Core/Time.hpp>

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
	//Vector2 move = obj->Move();
	//Vector2 start = obj->Transform()->Position();
	//Vector2 size = obj->Collider()->box.Size();
	//Vector2 extents = obj->Collider()->box.Extents();
	//Vector2 target;
	//Vector2 step;
	//Vector2Int tile;
	//
	//F32 accumulatedTime = 0.0f;
	//F32 allottedTime = (F32)Time::DeltaTime();
	//
	//if (move.x < 0.0f)
	//{
	//	start.x -= extents.x;
	//	target.x = Math::Round(start.x - 0.0001f) - 0.5f; //TODO: The tile size could be something other than 1, so use (tileSize * 0.5f)
	//	tile.x = (I32)(target.x - 0.5f);
	//	step.x = -1.0f;
	//}
	//else
	//{
	//	start.x += extents.x;
	//	target.x = Math::Round(start.x - 0.0001f) + 0.5f;
	//	tile.x = (I32)(target.x + 0.5f);
	//	step.x = 1.0f;
	//}
	//
	//if (move.y < 0.0f)
	//{
	//	start.y -= extents.y;
	//	target.y = Math::Round(start.y - 0.0001f) - 0.5f;
	//	tile.y = (I32)(target.y - 0.5f);
	//	step.y = -1.0f;
	//}
	//else
	//{
	//	start.y += extents.y;
	//	target.y = Math::Round(start.y - 0.0001f) + 0.5f;
	//	tile.y = (I32)(target.y + 0.5f);
	//	step.y = 1.0f;
	//}
	//
	//Vector2 position = start;
	//bool collidedX = move.x == 0.0f;
	//bool collidedY = move.y == 0.0f;
	//
	//while (true)
	//{
	//	Vector2 t = (target - position) / move;
	//
	//	if ((t.x < t.y || collidedY) && !collidedX)
	//	{
	//		accumulatedTime += t.x;
	//		if (accumulatedTime > allottedTime) { return contacts.Size(); }
	//
	//		position.x = target.x;
	//
	//		if (grid[tile.x][tile.y - (I32)step.y].blockID) //TODO: Check if in bounds
	//		{
	//			move.x = 0.0f; //TODO: Take restitution into account
	//			collidedX = true;
	//
	//			Contact2D c{};
	//			c.a = obj;
	//			c.distance = position.x - start.x;
	//			c.normal = Vector2::RIGHT * step.x;
	//			c.relativeVelocity = obj->Move();
	//			c.restitution = obj->Restitution(); //TODO: get tile restitution;
	//
	//			contacts.PushBack(c);
	//		}
	//
	//		target.x += step.x;
	//		tile.x += step.x;
	//	}
	//	else if (!collidedY)
	//	{
	//		accumulatedTime += t.y;
	//		if (accumulatedTime > allottedTime) { return contacts.Size(); }
	//
	//		position.y = target.y;
	//
	//		if (grid[tile.x - (I32)step.x][tile.y].blockID)
	//		{
	//			move.y = 0.0f; //TODO: Take restitution into account
	//			collidedY = true;
	//
	//			Contact2D c{};
	//			c.a = obj;
	//			c.distance = position.y - start.y;
	//			c.normal = Vector2::UP * step.y;
	//			c.relativeVelocity = obj->Move();
	//			c.restitution = obj->Restitution(); //TODO: get tile restitution;
	//
	//			contacts.PushBack(c);
	//		}
	//
	//		target.y += step.y;
	//		tile.y += step.y;
	//	}
	//	else { break; }
	//}

	Vector2 start = obj->Transform()->Position();
	Vector2 move = obj->Move();
	F32 length = move.Magnitude();
	Vector2 dir = move / length;

	Vector2 size = obj->Collider()->box.Size();
	Vector2 extents = obj->Collider()->box.Extents();

	Vector2 unitStepSize = { 1.0f / dir.x * Math::Sign(dir.x), 1.0f / dir.y * Math::Sign(dir.y) };
	Vector2 excess = Vector2::ONE / obj->Collider()->box.Size();

	Vector2Int mapCheck = (Vector2Int)start;
	Vector2 length1D;

	Vector2Int step;

	Vector2 startDist;

	if (dir.x < 0.0f)
	{
		F32 side = start.x - extents.x;
		side = (U32)(side + 1.0f) - side;
		startDist.x = 0.5f - side + (side > 0.5f);
		step.x = -1;
		length1D.x = (start.x + 1.0f - mapCheck.x) * unitStepSize.x;
	}
	else
	{
		F32 side = start.x + extents.x;
		side -= (U32)side;
		startDist.x = 0.5f - side + (side > 0.5f);
		step.x = 1;
		length1D.x = (mapCheck.x + 1.0f - start.x) * unitStepSize.x;
	}

	if (dir.y < 0.0f)
	{
		F32 side = start.y - extents.y;
		side = (U32)(side + 1.0f) - side;
		startDist.y = 0.5f - side + (side > 0.5f);
		step.y = -1;
		length1D.y = (start.y + 1.0f - mapCheck.y) * unitStepSize.y;
	}
	else
	{
		F32 side = start.y + extents.y;
		side -= (U32)side;
		startDist.y = 0.5f - side + (side > 0.5f);
		step.y = 1;
		length1D.y = (mapCheck.y + 1.0f - start.y) * unitStepSize.y;
	}

	bool collidedX = Math::NaN(length1D.x) || startDist.x > Math::Abs(move.x);
	bool collidedY = Math::NaN(length1D.y) || startDist.y > Math::Abs(move.y);

	U32 minX = (U32)(start.x - extents.x + 0.49999999999);
	U32 maxX = (U32)(start.x + extents.x + 0.49999999999);
	U32 minY = (U32)(start.y - extents.y + 0.49999999999);
	U32 maxY = (U32)(start.y + extents.y + 0.49999999999);

	U32& x = step.x > 0 ? maxX : minX;
	U32& y = step.y > 0 ? maxY : minY;

	F32 distanceX = startDist.x;
	F32 distanceY = startDist.y;

	//TODO: You can pass through walls on the left

	while ((length1D.x < length + extents.x && !collidedX) || (length1D.y < length + extents.y && !collidedY))
	{
		for (U32 y = minY; y <= maxY && !collidedX; ++y)
		{
			if ((U32)(x + step.x) < width && y < height && grid[x + step.x][y].blockID)
			{
				collidedX = true;

				Contact2D c{};
				c.a = obj;
				c.distance = distanceX;
				c.normal = Vector2::RIGHT * (F32)step.x;
				c.relativeVelocity = obj->Move();
				c.restitution = obj->Restitution(); //TODO: get tile restitution;

				contacts.PushBack(c);
			}
		}

		for (U32 x = minX; x <= maxX && !collidedY; ++x)
		{
			if (x < width && (U32)(y + step.y) < height && grid[x][y + step.y].blockID)
			{
				collidedY = true;

				Contact2D c{};
				c.a = obj;
				c.distance = distanceY;
				c.normal = Vector2::UP * (F32)step.y;
				c.relativeVelocity = obj->Move();
				c.restitution = obj->Restitution(); //TODO: get tile restitution;

				contacts.PushBack(c);
			}
		}

		if ((collidedY || length1D.x < length1D.y) && !collidedX)
		{
			mapCheck.x += step.x;
			distanceX += 1;
			length1D.x += 1;
			minX += step.x;
			maxX += step.x;
		}
		else if (!collidedY)
		{
			mapCheck.y += step.y;
			distanceY += 1;
			length1D.y += 1;
			minY += step.y;
			maxY += step.y;
		}
	}

	return contacts.Size();
}

bool GridBroadphase::Query(const Box& box, Vector<PhysicsObject2D*>& result)
{
	bool found = false;

	for (PhysicsObject2D* po : objs)
	{
		if ((po->Collider()->box + po->Transform()->Position()).Contains(box))
		{
			found = true;
			result.Push(po);
		}
	}

	return found;
}

bool GridBroadphase::RaycastQuery(PhysicsObject2D* obj, PhysicsObject2D* result)
{
	return false;
}

bool GridBroadphase::RaycastQueryAll(Raycast& ray, Vector<PhysicsObject2D*>& result)
{
	return false;
}
