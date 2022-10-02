#include "GridBroadphase.hpp"

#include "Physics/Physics.hpp"

GridBroadphase::GridBroadphase(U16 width, U16 height) : width{ width }, height{ height }, grid{ nullptr }
{
	grid = (Cell**)Memory::LinearAllocate(sizeof(Cell*) * width);

	for (U64 i = 0; i < width; ++i)
	{
		grid[i] = (Cell*)Memory::LinearAllocate(sizeof(Cell) * height);
	}
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

	for (U64 x = min.x; x <= max.x; ++x)
	{
		for (U64 y = min.y; y <= max.y; ++y)
		{
			grid[x][y].objs.PushBack(obj);
		}
	}
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

	for (U64 x = min.x; x <= max.x; ++x)
	{
		for (U64 y = min.y; y <= max.y; ++y)
		{
			grid[x][y].objs.Remove(obj);
		}
	}

	pos += obj->Move();
	min = (Vector2Int)(pos - ext);
	max = (Vector2Int)(pos + ext);

	for (U64 x = min.x; x <= max.x; ++x)
	{
		for (U64 y = min.y; y <= max.y; ++y)
		{
			grid[x][y].objs.PushBack(obj);
		}
	}
}

void GridBroadphase::ChangeTile(U16 x, U16 y, U8 id)
{
	grid[x][y].block = id;
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

	//if (mapCheckHigh.y >= 0 && mapCheckLow.y < height && !Math::Zero(dir.x))
	//{
	//	bool foundX = false;
	//	F32 distanceX = 0.0f;
	//	F32 lengthX = move.x * Math::Sign(move.x) + unitStepSize.x;
	//	I32 checkX = mapCheckLow.x;
	//	while (!foundX && length1DX < lengthX && checkX >= 0) //TODO: check in a wider area
	//	{
	//		checkX += step.x;
	//		distanceX = length1DX;
	//		length1DX += axisStepSize.x;

	//		foundX = checkX < width && ((mapCheckLow.y >= 0 && grid[checkX][mapCheckLow.y].block) || (mapCheckHigh.y < height&& grid[checkX][mapCheckHigh.y].block));
	//	}

	//	if (foundX)
	//	{
	//		Contact2D contact{};
	//		contact.normal = Vector2::RIGHT * step.x;
	//		contact.a = obj;
	//		contact.distance = distanceX - unitStepSize.x;
	//		contact.relativeVelocity = obj->Move();
	//		contact.restitution = obj->Restitution();//TODO: get tile restitution;

	//		contacts.PushBack(contact);
	//	}
	//}

	//if (mapCheckHigh.x >= 0 && mapCheckLow.x < width && !Math::Zero(dir.y))
	//{
	//	bool foundY = false;
	//	F32 distanceY = 0.0f;
	//	F32 lengthY = move.y * Math::Sign(move.y) + unitStepSize.y;
	//	I32 checkY = mapCheckLow.y;
	//	while (!foundY && length1DY < lengthY && checkY >= 0)
	//	{
	//		checkY += step.y;
	//		distanceY = length1DY;
	//		length1DY += axisStepSize.y;

	//		foundY = checkY < height && ((mapCheckLow.x >= 0 && grid[mapCheckLow.x][checkY].block) || (mapCheckHigh.x < height&& grid[mapCheckHigh.x][checkY].block));
	//	}

	//	if (foundY)
	//	{
	//		Contact2D contact{};
	//		contact.normal = Vector2::UP * step.y;
	//		contact.a = obj;
	//		contact.distance = distanceY - unitStepSize.y;
	//		contact.relativeVelocity = obj->Move();
	//		contact.restitution = obj->Restitution();//TODO: get tile restitution;

	//		contacts.PushBack(contact);
	//	}
	//}

	Vector2 extents = obj->Collider()->box.Extents();

	bool collidedX = Math::NaN(length1D.x);
	bool collidedY = Math::NaN(length1D.y);

	/*if (dir.x < 0.0f)
	{
		debugBreak();
	}*/

	Vector2Int min = (Vector2Int)(start - extents);
	Vector2Int max = (Vector2Int)(start + extents);
	I32& x = step.x > 0 ? max.x : min.x;
	I32& y = step.y > 0 ? max.y : min.y;

	F32 distance = 0.0f;
	while ((length1D.x < length + extents.x && !collidedX) || (length1D.y < length + extents.y && !collidedY))
	{
		if ((collidedY || length1D.x < length1D.y) && !collidedX)
		{
			mapCheck.x += step.x;
			distance = length1D.x;
			length1D.x += unitStepSize.x;
			min.x += step.x;
			max.x += step.x;

			for (U64 y = min.y; y <= max.y; ++y)
			{
				if (x >= 0 && x < x < width && y >= 0 && y < height && grid[x][y].block)
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
		}
		else if (!collidedY)
		{
			mapCheck.y += step.y;
			distance = length1D.y;
			length1D.y += unitStepSize.y;
			min.y += step.y;
			max.y += step.y;

			for (U64 x = min.x; x <= max.x; ++x)
			{
				if (x >= 0 && x < x < width && y >= 0 && y < height && grid[x][y].block)
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
