#include "BAH.hpp"

#include "Physics.hpp"
#include <Containers/Stack.hpp>

const U32 BAH::maxNodeSize = 3;

void BAH::Node::ComputeBoundary()
{
	if (objects.Size())
	{
		PhysicsObject2D* first = objects[0];
		xBounds = first->collider->xBounds + first->transform->Position().x;
		yBounds = first->collider->yBounds + first->transform->Position().y;

		auto end = objects.end();

		for (auto it = objects.begin() + 1; it != end; ++it)
		{
			PhysicsObject2D* po = *it;

			Vector2 boundsX = po->collider->xBounds + po->transform->Position().x;
			Vector2 boundsY = po->collider->yBounds + po->transform->Position().y;

			xBounds.x = Math::Min(xBounds.x, boundsX.x);
			xBounds.y = Math::Max(xBounds.y, boundsX.y);
			yBounds.x = Math::Min(yBounds.x, boundsY.x);
			yBounds.y = Math::Max(yBounds.y, boundsY.y);
		}
	}
}

void BAH::Node::Split()
{
	if (objects.Size() > maxNodeSize)
	{
		List<PhysicsObject2D*> split;
		objects.Split(objects.Size() / 2, split);
		right = new Node(Move(split));
		left = new Node(Move(objects));
	}
}

void BAH::Node::Query(const Vector2& boundsX, const Vector2& boundsY, List<PhysicsObject2D*>& results)
{
	if (Contains(boundsX, boundsY))
	{
		results.AddRange(objects);

		if (left) { left->Query(boundsX, boundsY, results); }
		if (right) { right->Query(boundsX, boundsY, results); }
	}
}

void BAH::Build(List<PhysicsObject2D*>&& bodies)
{
	Destroy();

	//TODO: Sort bodies by xPos

	root = new Node(Move(bodies));
}

void BAH::Destroy()
{
	if (root)
	{
		Stack<Node*> s;
		s.Push(root);

		while (!s.Empty())
		{
			Node* node = s.Pop();

			if (node->left) { s.Push(node->left); }
			if (node->right) { s.Push(node->right); }

			node->Destroy();
			delete node;
		}

		root = nullptr;
	}
}

void BAH::Query(const Vector2& boundsX, const Vector2& boundsY, List<PhysicsObject2D*>& results)
{
	root->Query(boundsX, boundsY, results);
}

void BAH::Query(struct PhysicsObject2D* object, List<struct PhysicsObject2D*>& results)
{
	Query(object->collider->xBounds + object->transform->Position().x, object->collider->yBounds + object->transform->Position().y, results);
}