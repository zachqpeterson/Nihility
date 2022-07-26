#include "BAH.hpp"

#include "Physics.hpp"

void BAH::Node::ComputeBoundary()
{
	if (objects.Size())
	{
		PhysicsObject2D* first = objects[0];
		xBounds = first->collider->XBounds() + first->position.x;
		yBounds = first->collider->YBounds() + first->position.y;

		auto end = objects.end();

		for (auto it = objects.begin() + 1; it != end; ++it)
		{
			PhysicsObject2D* po = *it;

			Vector2 boundsX = po->collider->XBounds() + po->position.x;
			Vector2 boundsY = po->collider->YBounds() + po->position.y;

			xBounds.x = Math::Min(xBounds.x, boundsX.x);
			xBounds.y = Math::Max(xBounds.y, boundsX.y);
			yBounds.x = Math::Min(yBounds.x, boundsY.x);
			yBounds.y = Math::Max(yBounds.y, boundsY.y);
		}
	}
}

void BAH::Node::Split()
{
	U64 half = (U64)(objects.Size() * 0.5f);
	if (half > 0)
	{
		List<PhysicsObject2D*>&& list = objects.Split(half);

		right = new Node(Move(list));
		left = new Node(Move(objects));
	}
}

void BAH::Node::Query(const Vector2& boundsX, const Vector2& boundsY, List<PhysicsObject2D*>& results)
{
	if (Contains(boundsX, boundsY))
	{
		results.AddRange(objects);

		if (left) { Query(boundsX, boundsY, results); }
		if (right) { Query(boundsX, boundsY, results); }
	}
}

void BAH::Build(List<PhysicsObject2D*>&& bodies)
{
	//TODO: Sort bodies by xPos

	root = new Node(Move(bodies));
}

void BAH::Query(const Vector2& boundsX, const Vector2& boundsY, List<PhysicsObject2D*>& results)
{
	root->Query(boundsX, boundsY, results);
}

void BAH::Query(struct PhysicsObject2D* object, List<struct PhysicsObject2D*>& results)
{
	Query(object->collider->XBounds(), object->collider->YBounds(), results);
}